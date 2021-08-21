//===========================================================================//
///
/// Copyright Jim Carty © 2020-2021
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "../Cell.hpp"

#include <string>

#ifndef PNGWriter_hpp
#define PNGWriter_hpp

#if DOUBLE_SUPPORT_AVAILABLE
    // double
typedef double Real;
#else
    // float
typedef float Real;
#endif

class PNGWriter {
private:
    std::string fname;

    unsigned int windowWidth,
        cellsPerRow,
        colourR,
        colourG,
        colourB,
        maxCount;

    bool alpha;

public:
    PNGWriter(std::string fname, unsigned int windowWidth, unsigned int cellsPerRow, unsigned int colourR, unsigned int colourG, unsigned int colourB, unsigned int maxCount, bool alpha) : fname(fname), windowWidth(windowWidth), cellsPerRow(cellsPerRow), colourR(colourR), colourG(colourG), colourB(colourB), maxCount(maxCount), alpha(alpha) {};

    void write(std::vector<std::vector<Cell*>>* fileData);
    void write(Real* fileData);
};

#endif // PNGWriter_hpp
