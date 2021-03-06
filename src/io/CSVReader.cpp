//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "CSVReader.hpp"

#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string>

Real *CSVReader::read() {
    std::ifstream file;
    file.open(fname);
    
    std::string line, tmp;

    std::getline(file, line, '\n'); // remove headings
    
    unsigned int lineCount = 0;
    
    while (std::getline(file, line, '\n'))
        ++lineCount;
    
    cellsPerRow = sqrt(lineCount);
    
    Real *result = new Real[cellsPerRow * cellsPerRow * 3];
    
    file.clear();
    file.seekg(0);
    
    unsigned int count = 0;
    
    std::getline(file, line, '\n'); // remove headings
    
    while (count < cellsPerRow * cellsPerRow && std::getline(file, line, '\n')) {
        std::stringstream lineStream(line);
        
        unsigned int x = count / cellsPerRow;
        unsigned int y = count % cellsPerRow;
        
        // real
        std::getline(lineStream, tmp, ',');
        result[x * cellsPerRow * 3 + y * 3 + 0] = std::stold(tmp);
        
        // imag
        std::getline(lineStream, tmp, ',');
        result[x * cellsPerRow * 3 + y * 3 + 1] = std::stold(tmp);
        
        // counter
        std::getline(lineStream, tmp, ',');
        result[x * cellsPerRow * 3 + y * 3 + 2] = std::stold(tmp);
        
        ++count;
    }

    file.close();
    
    return result;
}

int CSVReader::write(std::vector<std::vector<Cell*>> *fileData) {
    std::ofstream stream;
    
    stream.open(fname);
    
    if (!stream)
        return 1;
    
    std::string resultString = "pixel_real,pixel_imag,counter\n";
    
    for (unsigned int i = 0; i < fileData->size(); ++i) {
        for (unsigned int j = 0; j < fileData->at(i).size(); ++j) {
            resultString.append(
                                std::to_string(fileData->at(i)[j]->complex.real) + ',' +
                                std::to_string(fileData->at(i)[j]->complex.imag) + ',' +
                                std::to_string((unsigned int)fileData->at(i)[j]->counter) + '\n');
        }
        
        if (i == cellsPerRow / 2) {
            // write and reset to make sure memory doesnt get higher than initial memory steal
            stream << resultString;
            
            resultString.clear();
        }
    }
    
    stream << resultString;
    
    stream.close();
    
    std::cout << "Saved fractal data to " << fname << std::endl;
    
    return 0;
}

int CSVReader::write(Real *fileData) {
    std::ofstream stream;
    
    stream.open(fname);
    
    if (!stream)
        return 1;
    
    std::string resultString = "pixel_real,pixel_imag,counter\n";
    
    for (unsigned int i = 0; i < cellsPerRow; ++i) {
        for (unsigned int j = 0; j < cellsPerRow; ++j)
            resultString.append(
                                std::to_string(fileData[i * cellsPerRow * 3 + j * 3 + 0]) + ',' +
                                std::to_string(fileData[i * cellsPerRow * 3 + j * 3 + 1]) + ',' +
                                std::to_string((unsigned int)fileData[i * cellsPerRow * 3 + j * 3 + 2]) + '\n');
        
        if (i == cellsPerRow / 2) {
            // write and reset to make sure memory doesnt get higher than initial memory steal
            stream << resultString;
            
            resultString.clear();
            
            resultString.shrink_to_fit();
        }
    }
    
    stream << resultString;
    
    stream.close();
    
    std::cout << "Saved fractal data to " << fname << std::endl;
    
    return 0;
}


