//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include <filesystem>
#include <iostream>

#include "KernelHelper.hpp"

void calculateCells(Real **g_cellsGPU, unsigned int *g_maxCount, unsigned int *iterations, unsigned int *cellsPerRow, const std::pair<long double, long double> *min, long double *cellRealWidth, bool *anti) {
    printf("Using %d-bit %s floating point precision\n", PRECISION, PRECISION == 64 ? "double" : "float");
    cl_int err;

    size_t global;
    size_t local;

    cl_context context;
    cl_command_queue commands;
    cl_kernel kernel;
    cl_program program;
    cl_device_id deviceId;

    cl_mem input;
    cl_mem output;

    unsigned int count = (*cellsPerRow) * (*cellsPerRow);
    unsigned int inputCount = count * 3;

    (*g_cellsGPU) = new Real[inputCount];
    unsigned int *counts = new unsigned int[count]();

    const Real cellRealWidthGPU = (Real)*cellRealWidth;
    const std::pair<Real, Real> MIN_GPU = { (Real)min->first, (Real)min->second };

    for (unsigned int i = 0; i < *cellsPerRow; ++i) {
        for (unsigned int j = 0; j < *cellsPerRow; ++j) {
            Real realx = MIN_GPU.first + cellRealWidthGPU * (*cellsPerRow - 1 - i); // inverting iteration through cells on the x-axis so that the buddhabrot renders "sitting-down" -- more picturesque
            Real realy = MIN_GPU.second + cellRealWidthGPU * j;
            (*g_cellsGPU)[i * *cellsPerRow * 3 + j * 3 + 0] = realx;
            (*g_cellsGPU)[i * *cellsPerRow * 3 + j * 3 + 1] = realy;
            (*g_cellsGPU)[i * *cellsPerRow * 3 + j * 3 + 2] = 0;
        }
    }

    err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &deviceId, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to create a device group as no GPU found. Check install or use without -o option" << std::endl;
        exit(1);
    }

    context = clCreateContext(0, 1, &deviceId, NULL, NULL, &err);
    if (!context) {
        std::cout << "Failed to create a compute context. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    commands = clCreateCommandQueue(context, deviceId, 0, &err);
    if (!commands) {
        std::cout << "Failed to create a command commands. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    size_t length = 0;
    char *source;
    err = loadTextFromFile(KERNEL_FILENAME, &source, &length);
    if (err != 0) {
        std::cout << "Failed to load kernel source. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    program = clCreateProgramWithSource(context, 1, (const char **) &source, NULL, &err);
    if (!program || err != CL_SUCCESS) {
        std::cout << "Failed to create compute program. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }
    free(source);

    // Build the program executable
    char compileArgs[128];
    sprintf(compileArgs, "-DITERATIONS=%d -DCELLS_PER_ROW=%d -DANTI=%d", *iterations, *cellsPerRow, (unsigned int)*anti);

    err = clBuildProgram(program, 1, &deviceId, compileArgs, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t len;
        char buffer[2048];

        std::cout << "Failed to build program executable. Check OpenCL install or use without -o option" << std::endl;
        clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 2048 * 8, buffer, &len);
        std::cout << buffer << std::endl;
        exit(1);
    }

    kernel = clCreateKernel(program, KERNEL_FUNCTION_NAME, &err);
    if (!kernel || err != CL_SUCCESS) {
        std::cout << "Failed to create compute kernel. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    // Create the input and output arrays in device memory for our calculation
    input = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Real) * inputCount, NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(unsigned int) * count, NULL, NULL);
    if (!input || !output) {
        std::cout << "Failed to allocate device memory. Check OpenCL install or use lower resolution or iteration values" << std::endl;
        exit(1);
    }

    err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(Real) * inputCount, *g_cellsGPU, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to write to source array. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    // Set the arguments to our compute kernel
    err = 0;
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(Real), &MIN_GPU.first);
    err |= clSetKernelArg(kernel, 2, sizeof(Real), &MIN_GPU.second);
    err |= clSetKernelArg(kernel, 3, sizeof(Real), &cellRealWidthGPU);
    err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &output);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to set kernel arguments: " << err << std::endl;
        exit(1);
    }

    std::cout << "Memory successfully yoinked" << std::endl;

    err = clGetKernelWorkGroupInfo(kernel, deviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Failed to retrieve kernel work group info: " << err << std::endl;
        exit(1);
    }

    // Execute the kernel
    if (count % local == 0)
        global = count;
    else
        global = (floor(count / local) + 1) * local;

    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err) {
        std::cout << "Failed to execute kernel. Check OpenCL install or use without -o option" << std::endl;
        exit(1);
    }

    clFinish(commands);

    err = clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(unsigned int) * count, counts, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "Error: Failed to read output array: " << err << std::endl;
        exit(1);
    }

    for (unsigned int i = 0; i < *cellsPerRow; ++i) {
        for (unsigned int j = 0; j < *cellsPerRow; ++j) {
            (*g_cellsGPU)[i * *cellsPerRow * 3 + j * 3 + 2] = counts[i * *cellsPerRow + j];
            
            if ((*g_cellsGPU)[i * *cellsPerRow * 3 + j * 3 + 2] > *g_maxCount)
                *g_maxCount = (*g_cellsGPU)[i * *cellsPerRow * 3 + j * 3 + 2];
        }
    }

    clReleaseMemObject(input);
    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    delete[] counts;
}

int loadTextFromFile(const char *filename, char **fileString, size_t *stringLength) {
    size_t length = 0;
    
    std::filesystem::path p = std::filesystem::current_path();
    
    p += filename;
    
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
