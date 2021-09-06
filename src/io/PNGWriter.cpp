//===========================================================================//
///
/// Copyright Jim Carty © 2020-2021
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "PNGWriter.hpp"

#include <opencv2/opencv.hpp>

#include <fstream>
#include <iostream>
#include <math.h>

void PNGWriter::write(std::vector<std::vector<Cell*>>* fileData) {
    cv::Mat image(windowWidth, windowWidth, CV_8UC4);

    for (unsigned int i = 0; i < fileData->size(); ++i) {
        for (unsigned int j = 0; j < fileData->size(); ++j) {
            long double percentageOfMax = log(fileData->at(i)[j]->counter) / log(maxCount);

            cv::Vec4b pixel;

            if (percentageOfMax <= 0.25)
                pixel = { 0, 0, 0, 255 };
            else {
                // rendering using brightness to determin alpha value but due to viewing videos,
                // may appear weirdly washed out so for more consistent results, change rgb
                // values and a set alpha by default

                if (alpha)
                    pixel = { (unsigned char)colourR, (unsigned char)colourG, (unsigned char)colourB, (unsigned char)(percentageOfMax * 255.0f) };
                else
                    pixel = { (unsigned char)(colourR * percentageOfMax), (unsigned char)(colourG * percentageOfMax), (unsigned char)(colourB * percentageOfMax), 255 };
            }

            image.at<cv::Vec4b>(cv::Point(j, i)) = pixel;
        }
    }

    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

    cv::imwrite(fname, image);

    std::cout << "Saved fractal to " << fname << std::endl;
}

void PNGWriter::write(Real* fileData) {
    cv::Mat image(windowWidth, windowWidth, CV_8UC4);

    for (unsigned int i = 0; i < windowWidth; ++i) {
        for (unsigned int j = 0; j < windowWidth; ++j) {
            long double percentageOfMax = log(fileData[i * cellsPerRow * 3 + j * 3 + 2]) / log(maxCount);

            cv::Vec4b pixel;

            if (percentageOfMax <= 0.25)
                pixel = { 0, 0, 0, 255 };
            else {
                // rendering using brightness to determin alpha value but due to viewing videos,
                // may appear weirdly washed out so for more consistent results, change rgb
                // values and a set alpha by default

                if (alpha)
                    pixel = { (unsigned char)colourR, (unsigned char)colourG, (unsigned char)colourB, (unsigned char)(percentageOfMax * 255.0f) };
                else
                    pixel = { (unsigned char)(colourR * percentageOfMax), (unsigned char)(colourG * percentageOfMax), (unsigned char)(colourB * percentageOfMax), 255 };
            }

            image.at<cv::Vec4b>(cv::Point(j, i)) = pixel;
        }
    }

    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

    cv::imwrite(fname, image);

    std::cout << "Saved fractal to " << fname << std::endl;
}
