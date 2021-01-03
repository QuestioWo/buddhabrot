//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "../Cell.hpp"

#ifndef PNGReader_hpp
#define PNGReader_hpp

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

class PNGReader {
private :
    char *fname;
    
    unsigned int windowWidth,
        cellsPerRow,
        colourR,
        colourG,
        colourB,
        maxCount;
    
    bool alpha;
    
public:
    PNGReader(char *fname, unsigned int windowWidth, unsigned int cellsPerRow, unsigned int colourR, unsigned int colourG, unsigned int colourB, unsigned int maxCount, bool alpha) : fname(fname), windowWidth(windowWidth), cellsPerRow(cellsPerRow), colourR(colourR), colourG(colourG), colourB(colourB), maxCount(maxCount), alpha(alpha) {};
    
    int write(std::vector<std::vector<Cell*>> *fileData);
    int write(Real *fileData);
};

#endif // PNGReader_hpp
