//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "KernelHelper.hpp"

#include <iostream>
#include <math.h>

static Real **g_cellsGPU;
static unsigned int *g_maxCount;
static std::pair<Real, Real> MIN_GPU;
static Real cellRealWidth;
static unsigned long int count;
static unsigned long int inputCount;
static unsigned int cellsPerRow;

void calculateCells(Real **cellsGPU, unsigned int *maxCount, unsigned int *iterations, unsigned int *cellsPerRowPassed, const std::pair<long double, long double> *min, long double *cellRealWidthPassed, bool *anti, unsigned int *iterationsMax) {
    printf("Using %d-bit (%s) floating point precision\n", PRECISION, PRECISION == 64 ? "double" : "float");
    
    g_cellsGPU = cellsGPU;
    g_maxCount = maxCount;
    cellsPerRow = *cellsPerRowPassed;
    
    count = cellsPerRow * cellsPerRow;
    inputCount = count * 2;
    
    if (*iterationsMax == 0)
        *iterationsMax = 3.28E11 * pow(cellsPerRow, -2.06);
    
    if (*anti)
        *iterationsMax = ceil(*iterationsMax / 8);
    
    std::cout << "Iterations per group := " << *iterationsMax << std::endl;
    
    // 1000-500 maximum for 6001
    // ~5500 maximum for 2001
    // >7500 maximum for 501

    cl_platform_id platformId;
    cl_device_id deviceId;
    cl_context context;
    cl_command_queue commands;
    cl_kernel kernelCheck;
    cl_kernel kernelCount;
    cl_program program;
    
    cl_int err;

    (*g_cellsGPU) = new Real[count * 3];
    Real *originalCells = new Real[inputCount];
    Real *interimResultsCheck = new Real[inputCount]();

    cellRealWidth = (Real)*cellRealWidthPassed;
    MIN_GPU = { (Real)min->first, (Real)min->second };

    for (unsigned int i = 0; i < cellsPerRow; ++i) {
        for (unsigned int j = 0; j < cellsPerRow; ++j) {
            Real realx = MIN_GPU.first + cellRealWidth * (cellsPerRow - 1 - i); // inverting iteration through cells on the x-axis so that the buddhabrot renders "sitting-down" -- more picturesque
            Real realy = MIN_GPU.second + cellRealWidth * j;
            
            (*g_cellsGPU)[i * cellsPerRow * 3 + j * 3 + 0] = realx;
            (*g_cellsGPU)[i * cellsPerRow * 3 + j * 3 + 1] = realy;
            (*g_cellsGPU)[i * cellsPerRow * 3 + j * 3 + 2] = 0;
            
            originalCells[i * cellsPerRow * 2 + j * 2 + 0] = realx;
            originalCells[i * cellsPerRow * 2 + j * 2 + 1] = realy;
        }
    }
    
    cl_uint numPlatforms;
    err = clGetPlatformIDs(1, &platformId, &numPlatforms);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to find an OpenCL platform. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    err = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_GPU, 1, &deviceId, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "GPU device not found. Check install or use without -o option" << std::endl;
        exit(1);
    }

    context = clCreateContext(0, 1, &deviceId, NULL, NULL, &err);
    if (!context) {
        std::cout << "Couldn't create a compute context using GPU. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    commands = clCreateCommandQueue(context, deviceId, 0, &err);
    if (!commands) {
        std::cout << "Failed to create a command queue. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    size_t length = 0;
    char *source;
    err = loadTextFromFile(KERNEL_FILENAME, &source, &length);
    if (err != 0) {
        std::cout << "Failed to load kernel source. Check where program is executed from, must be able to see src/, which contains EscapeKernel.cl" << std::endl;
        exit(1);
    }

    program = clCreateProgramWithSource(context, 1, (const char **) &source, NULL, &err);
    if (!program || err != CL_SUCCESS) {
        std::cout << "Failed to create program from source. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }
    free(source);

    // Build the program executable
    char compileArgs[128];
    sprintf(compileArgs, "-D CELLS_PER_ROW=%d -D ANTI=%d -D ITERATIONS_MAX=%d", cellsPerRow, (cl_uint)*anti, *iterationsMax);

    err = clBuildProgram(program, 1, &deviceId, compileArgs, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t len;
        char buffer[8192];

        std::cout << "Failed to build program executable. Check OpenCL install or use without -o option" << std::endl;
        clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, sizeof(char) * 8192, buffer, &len);
        std::cout << buffer << std::endl;
        exit(1);
    }

    kernelCheck = clCreateKernel(program, KERNEL_CHECK_FUNCTION_NAME, &err);
    if (!kernelCheck || err != CL_SUCCESS) {
        std::cout << "Failed to create check kernel. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }
    
    kernelCount = clCreateKernel(program, KERNEL_COUNT_FUNCTION_NAME, &err);
    if (!kernelCount || err != CL_SUCCESS) {
        std::cout << "Failed to create count kernel. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }
    
    cl_uint *pointCorrectlyEscapes = new cl_uint[count];
    
    std::cout << "Memory successfully yoinked" << std::endl;
    
    const unsigned int iterationGroups = *iterations / *iterationsMax;
    const unsigned int iterationFinal = *iterations - *iterationsMax * iterationGroups;
    bool extraGroup = *iterations > (*iterationsMax * iterationGroups);
    
    // check which coords have to be ran to find correct buddhabrot
    for (unsigned int i = 0; i < iterationGroups; ++i) {
        runCheckKernel(&context, &commands, &kernelCheck, &deviceId, &originalCells, &interimResultsCheck, &pointCorrectlyEscapes, iterationsMax);
        
        std::cout << '\t' << i << '/' << (extraGroup ? iterationGroups : (iterationGroups - 1)) << std::endl;
    }
    
    if (extraGroup) {
        runCheckKernel(&context, &commands, &kernelCheck, &deviceId, &originalCells, &interimResultsCheck, &pointCorrectlyEscapes, &iterationFinal);
        
        std::cout << '\t' << iterationGroups << '/' << iterationGroups << std::endl;
    }
    
    clReleaseKernel(kernelCheck);
    delete[] interimResultsCheck;
    delete[] originalCells;
    
    unsigned int pointsThatCorrectlyEscape = 0;
    
    cl_uint *positionsOfCorrect = new cl_uint[inputCount];
    
    for (unsigned int i = 0; i < cellsPerRow; ++i) {
        for (unsigned int j = 0; j < cellsPerRow; ++j) {
            if (pointCorrectlyEscapes[i * cellsPerRow + j]) {
                pointsThatCorrectlyEscape += 1;
                
                positionsOfCorrect[pointsThatCorrectlyEscape * 2 + 0] = i;
                positionsOfCorrect[pointsThatCorrectlyEscape * 2 + 1] = j;
            }
        }
    }
    
    delete[] pointCorrectlyEscapes;
    
    std::cout << "Correctly escaping points found := " << pointsThatCorrectlyEscape << '/' << count << std::endl;
    
    // make new interim results with only those that escape
    Real *pointsThatEscape = new Real[pointsThatCorrectlyEscape * 2];
    Real *interimResultsCount = new Real[pointsThatCorrectlyEscape * 2]();
    
    for (unsigned int i = 0; i < pointsThatCorrectlyEscape; ++i) {
        Real realx = MIN_GPU.first + cellRealWidth * (cellsPerRow - 1 - positionsOfCorrect[i * 2 + 0]); // inverting iteration through cells on the x-axis so that the buddhabrot renders "sitting-down" -- more picturesque
        Real realy = MIN_GPU.second + cellRealWidth * positionsOfCorrect[i * 2 + 1];
        
        pointsThatEscape[i * 2 + 0] = realx;
        pointsThatEscape[i * 2 + 1] = realy;
    }
    
    // use correctly escaping points to find all correctly visited points
    for (unsigned int i = 0; i < iterationGroups; ++i) {
        runCountKernel(&context, &commands, &kernelCount, &deviceId, &pointsThatEscape, &interimResultsCount, pointsThatCorrectlyEscape, iterationsMax);
        
        std::cout << '\t' << i << '/' << (extraGroup ? iterationGroups : (iterationGroups - 1)) << std::endl;
    }
    
    if (extraGroup) {
        runCountKernel(&context, &commands, &kernelCount, &deviceId, &pointsThatEscape, &interimResultsCount, pointsThatCorrectlyEscape, &iterationFinal);
        
        std::cout << '\t' << iterationGroups << '/' << iterationGroups << std::endl;
    }
    
    clReleaseProgram(program);
    clReleaseKernel(kernelCount);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);
    clReleaseDevice(deviceId);
    
    delete[] pointsThatEscape;
    delete[] interimResultsCount;
}

int loadTextFromFile(const char *filename, char **fileString, size_t *stringLength) {
    size_t length = 0;
    
    std::string p(filename);
    
    FILE *file = fopen(p.c_str(), "rb");
    if(file == NULL) {
        std::cout << "Error: Couldn't read the program file" << std::endl;
        return 1;
    }
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    rewind(file); // reset the file pointer so that 'fread' reads from the front
    *fileString = (char*)malloc(length + 1);
    (*fileString)[length] = '\0';
    fread(*fileString, sizeof(char), length, file);
    fclose(file);
    
    *stringLength = length;
    
    return 0;
}

void runCheckKernel(cl_context *context, cl_command_queue *commands, cl_kernel *kernel, cl_device_id *deviceId, Real **originalPoints, Real **interimResults, cl_uint **pointCorrectlyEscapes, const unsigned int *iterations) {
    size_t global;
    size_t local;
    
    // Create the input and output arrays in device memory for our calculation
    cl_mem inputOriginal = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(Real) * inputCount, NULL, NULL),
        inputCurrent = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(Real) * inputCount, NULL, NULL);
    
    cl_mem interimResultsGPU = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(Real) * inputCount, NULL, NULL);
    cl_mem output = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * count, NULL, NULL);
    if (!inputOriginal || !inputCurrent || !interimResultsGPU || !output) {
        std::cout << "Failed to allocate device memory. Check OpenCL install or use lower resolution or iteration values" << std::endl;
        exit(1);
    }

    cl_int err = clEnqueueWriteBuffer(*commands, inputCurrent, CL_TRUE, 0, sizeof(Real) * inputCount, *interimResults, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(*commands, inputOriginal, CL_TRUE, 0, sizeof(Real) * inputCount, *originalPoints, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to write to source array. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }
    
    // Set the arguments to our compute kernel
    err = 0;
    err = clSetKernelArg(*kernel, 0, sizeof(cl_mem), &inputOriginal);
    err |= clSetKernelArg(*kernel, 1, sizeof(cl_mem), &inputCurrent);
    err |= clSetKernelArg(*kernel, 2, sizeof(Real), &(MIN_GPU.first));
    err |= clSetKernelArg(*kernel, 3, sizeof(Real), &(MIN_GPU.second));
    err |= clSetKernelArg(*kernel, 4, sizeof(Real), &cellRealWidth);
    err |= clSetKernelArg(*kernel, 5, sizeof(cl_uint), iterations);
    err |= clSetKernelArg(*kernel, 6, sizeof(cl_mem), &interimResultsGPU);
    err |= clSetKernelArg(*kernel, 7, sizeof(cl_mem), &output);
    
    if (err != CL_SUCCESS) {
        std::cout << "Failed to set kernel arguments: " << err << std::endl;
        exit(1);
    }

    err = clGetKernelWorkGroupInfo(*kernel, *deviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to retrieve kernel work group info: " << err << std::endl;
        exit(1);
    }

    // Execute the kernel
    if (count % local == 0)
        global = count;
    else
        global = (floor(count / local) + 1) * local;

    err = clEnqueueNDRangeKernel(*commands, *kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err) {
        std::cout << "Failed to execute kernel. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    clFinish(*commands);

    err = clEnqueueReadBuffer(*commands, interimResultsGPU, CL_TRUE, 0, sizeof(Real) * inputCount, *interimResults, 0, NULL, NULL);
    err |= clEnqueueReadBuffer(*commands, output, CL_TRUE, 0, sizeof(cl_uint) * count, *pointCorrectlyEscapes, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Error: Failed to read output array: " << err << std::endl;
        exit(1);
    }
    
    clReleaseMemObject(inputOriginal);
    clReleaseMemObject(inputCurrent);
    clReleaseMemObject(interimResultsGPU);
    clReleaseMemObject(output);
}


void runCountKernel(cl_context *context, cl_command_queue *commands, cl_kernel *kernel, cl_device_id *deviceId, Real **originalPoints, Real **interimResults, unsigned int pointsThatCorrectlyEscape, const unsigned int *iterations) {
    
    cl_uint *counts = new cl_uint[count]();
    
    size_t global;
    size_t local;
    
    // Create the input and output arrays in device memory for our calculation
    cl_mem inputOriginal = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(Real) * pointsThatCorrectlyEscape * 2, NULL, NULL),
        inputCurrent = clCreateBuffer(*context, CL_MEM_READ_ONLY, sizeof(Real) * pointsThatCorrectlyEscape * 2, NULL, NULL);
    
    cl_mem interimResultsGPU = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(Real) * pointsThatCorrectlyEscape * 2, NULL, NULL);
    cl_mem output = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, sizeof(cl_uint) * count, NULL, NULL);
    if (!inputOriginal || !inputCurrent || !interimResultsGPU || !output ) {
        std::cout << "Failed to allocate device memory. Check OpenCL install or use lower resolution or iteration values" << std::endl;
        exit(1);
    }

    cl_int err = clEnqueueWriteBuffer(*commands, inputCurrent, CL_TRUE, 0, sizeof(Real) * pointsThatCorrectlyEscape * 2, *interimResults, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(*commands, inputOriginal, CL_TRUE, 0, sizeof(Real) * pointsThatCorrectlyEscape * 2, *originalPoints, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to write to source array. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }
    
    // Set the arguments to our compute kernel
    err = 0;
    err = clSetKernelArg(*kernel, 0, sizeof(cl_mem), &inputOriginal);
    err |= clSetKernelArg(*kernel, 1, sizeof(cl_mem), &inputCurrent);
    err |= clSetKernelArg(*kernel, 2, sizeof(cl_uint), &pointsThatCorrectlyEscape);
    err |= clSetKernelArg(*kernel, 3, sizeof(Real), &(MIN_GPU.first));
    err |= clSetKernelArg(*kernel, 4, sizeof(Real), &(MIN_GPU.second));
    err |= clSetKernelArg(*kernel, 5, sizeof(Real), &cellRealWidth);
    err |= clSetKernelArg(*kernel, 6, sizeof(cl_uint), iterations);
    err |= clSetKernelArg(*kernel, 7, sizeof(cl_mem), &interimResultsGPU);
    err |= clSetKernelArg(*kernel, 8, sizeof(cl_mem), &output);

    if (err != CL_SUCCESS) {
        std::cout << "Failed to set kernel arguments: " << err << std::endl;
        exit(1);
    }

    err = clGetKernelWorkGroupInfo(*kernel, *deviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to retrieve kernel work group info: " << err << std::endl;
        exit(1);
    }

    // Execute the kernel
    if (count % local == 0)
        global = count;
    else
        global = (floor(count / local) + 1) * local;

    err = clEnqueueNDRangeKernel(*commands, *kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err) {
        std::cout << "Failed to execute kernel. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    clFinish(*commands);

    err = clEnqueueReadBuffer(*commands, interimResultsGPU, CL_TRUE, 0, sizeof(Real) * pointsThatCorrectlyEscape * 2, *interimResults, 0, NULL, NULL);
    err |= clEnqueueReadBuffer(*commands, output, CL_TRUE, 0, sizeof(cl_uint) * count, counts, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Error: Failed to read output array: " << err << std::endl;
        exit(1);
    }
    
    for (unsigned int i = 0; i < cellsPerRow; ++i) {
        for (unsigned int j = 0; j < cellsPerRow; ++j) {
            (*g_cellsGPU)[i * cellsPerRow * 3 + j * 3 + 2] += (Real)counts[i * cellsPerRow + j];
            
            if ((*g_cellsGPU)[i * cellsPerRow * 3 + j * 3 + 2] > *g_maxCount)
                *g_maxCount = (*g_cellsGPU)[i * cellsPerRow * 3 + j * 3 + 2];
        }
    }
    
    clReleaseMemObject(inputOriginal);
    clReleaseMemObject(inputCurrent);
    clReleaseMemObject(interimResultsGPU);
    clReleaseMemObject(output);
    
    delete[] counts;
}
