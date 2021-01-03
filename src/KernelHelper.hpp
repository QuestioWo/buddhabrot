//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #define GLUT_SILENCE_DEPRECATION

    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif

#include <tuple>

#ifndef KernelHelper_hpp
#define KernelHelper_hpp

#if DOUBLE_SUPPORT_AVAILABLE
    // double
    typedef double Real;
    #define PRECISION (64)
#else
    // float
    typedef float Real;
    #define PRECISION (32)
#endif

static const char *KERNEL_FILENAME = "src/EscapeKernel.cl";
static const char *KERNEL_FUNCTION_NAME = "escape";
static const unsigned int ITERATIONS_GPU_MAX = 7500;

int loadTextFromFile(const char *filename, char **fileString, size_t *stringLength);

void calculateCells(Real **g_cellsGPU, unsigned int *g_maxCount, unsigned int *iterations, unsigned int *cellsPerRow, const std::pair<long double, long double> *min, long double *cellRealWidth, bool *anti);
void runKernel(cl_context *context, cl_command_queue *commands, cl_kernel *kernel, cl_device_id *deviceId, Real **interimResults, const unsigned int *iterations);

#endif // KernelHelper_hpp
