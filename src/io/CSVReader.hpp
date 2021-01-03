//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "../Cell.hpp"

#ifndef CSVReader_hpp
#define CSVReader_hpp

#if defined(cl_khr_fp64)
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
    #define DOUBLE_SUPPORT_AVAILABLE
#elif defined(cl_amd_fp64)
    #pragma OPENCL EXTENSION cl_amd_fp64 : enable
    #define DOUBLE_SUPPORT_AVAILABLE
#endif

#if defined(DOUBLE_SUPPORT_AVAILABLE)
    // double
    typedef double Real;
    #define PRECISION (64)

#else
    // float
    typedef float Real;
    #define PRECISION (32)

#endif

class CSVReader {
private:
    char *fname;
    unsigned int cellsPerRow;
    
public:
    CSVReader(char *fname, unsigned int cellsPerRow) : fname(fname), cellsPerRow(cellsPerRow) {};

    Real *read();
    int write(std::vector<std::vector<Cell*>> *fileData);
    int write(Real *fileData);
};

#endif // CSVReader_hpp
