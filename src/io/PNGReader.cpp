//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "PNGReader.hpp"

#include <png.h>

#include <fstream>
#include <iostream>
#include <math.h>

int PNGReader::write(std::vector<std::vector<Cell*>> *fileData) {
    FILE *fp = fopen(fname, "wb");
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);

    // write header
    png_set_IHDR(png_ptr, info_ptr, windowWidth, windowWidth,
        8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    // write bytes
    png_bytep *row_pointers;
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * windowWidth);
    for (unsigned int i = 0; i < windowWidth; ++i) {
        row_pointers[i] = (png_byte*) malloc( sizeof(png_byte) * windowWidth * 4);

        png_bytep rpptr = row_pointers[i];
        for (unsigned int j = 0; j < windowWidth; ++j) {
            long double percentageOfMax = log(fileData->at(i)[j]->counter) / log(maxCount);
            
            if (percentageOfMax <= 0.25) {
                // RGB
                *rpptr = 0;
                rpptr++;
                *rpptr = 0;
                rpptr++;
                *rpptr = 0;
                rpptr++;
                
                // Alpha
                *rpptr = 255;
                rpptr++;
            } else {
                // rendering using brightness to determin alpha value but due to viewing videos,
                // may appear weirdly washed out so for more consistent results, change rgb
                // values and a set alpha by default
                if (alpha) {
                    // RGB
                    *rpptr = colourR;
                    rpptr++;
                    *rpptr = colourG;
                    rpptr++;
                    *rpptr = colourB;
                    rpptr++;

                    // Alpha
                    *rpptr = (unsigned char)(percentageOfMax * 255.0f);
                    rpptr++;
                    
                } else {
                    // RGB
                    *rpptr = (unsigned char)(colourR * percentageOfMax);
                    rpptr++;
                    *rpptr = (unsigned char)(colourG * percentageOfMax);
                    rpptr++;
                    *rpptr = (unsigned char)(colourB * percentageOfMax);
                    rpptr++;
                    
                    // Alpha
                    *rpptr = 255;
                    rpptr++;
                }
            }
        }
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    fclose(fp);
    
    std::cout << "Saved fractal to " << fname << std::endl;
    
    return 0;
}

int PNGReader::write(Real *fileData) {
    FILE *fp = fopen(fname, "wb");
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);

    // write header
    png_set_IHDR(png_ptr, info_ptr, windowWidth, windowWidth,
        8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    // write bytes
    png_bytep *row_pointers;
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * windowWidth);
    for (unsigned int i = 0; i < windowWidth; ++i) {
        row_pointers[i] = (png_byte*) malloc( sizeof(png_byte) * windowWidth * 4);

        png_bytep rpptr = row_pointers[i];
        for (unsigned int j = 0; j < windowWidth; ++j) {
            long double percentageOfMax = log(fileData[i * cellsPerRow * 3 + j * 3 + 2]) / log(maxCount);
            
            if (percentageOfMax <= 0.25) {
                // RGB
                *rpptr = 0;
                rpptr++;
                *rpptr = 0;
                rpptr++;
                *rpptr = 0;
                rpptr++;
                
                // Alpha
                *rpptr = 255;
                rpptr++;
            } else {
                // rendering using brightness to determin alpha value but due to viewing videos,
                // may appear weirdly washed out so for more consistent results, change rgb
                // values and a set alpha by default
                if (alpha) {
                    // RGB
                    *rpptr = colourR;
                    rpptr++;
                    *rpptr = colourG;
                    rpptr++;
                    *rpptr = colourB;
                    rpptr++;

                    // Alpha
                    *rpptr = (unsigned char)(percentageOfMax * 255.0f);
                    rpptr++;
                    
                } else {
                    // RGB
                    *rpptr = (unsigned char)(colourR * percentageOfMax);
                    rpptr++;
                    *rpptr = (unsigned char)(colourG * percentageOfMax);
                    rpptr++;
                    *rpptr = (unsigned char)(colourB * percentageOfMax);
                    rpptr++;
                    
                    // Alpha
                    *rpptr = 255;
                    rpptr++;
                }
            }
        }
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    fclose(fp);
    
    std::cout << "Saved fractal to " << fname << std::endl;
    
    return 0;
}
