//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "../Cell.hpp"

#include <string>

#ifndef CSVReader_hpp
#define CSVReader_hpp

#if DOUBLE_SUPPORT_AVAILABLE
    // double
    typedef double Real;
#else
    // float
    typedef float Real;
#endif

class CSVReader {
private:
    std::string fname;
    unsigned int cellsPerRow;
    
public:
    CSVReader(std::string fname, unsigned int cellsPerRow) : fname(fname), cellsPerRow(cellsPerRow) {};

    Real *read();
    int write(std::vector<std::vector<Cell*>> *fileData);
    int write(Real *fileData);
};

#endif // CSVReader_hpp
