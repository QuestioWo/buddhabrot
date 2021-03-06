//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "Cell.hpp"

#if USE_OPENGL
    #ifdef __APPLE__
        #define GL_SILENCE_DEPRECATION
        #define GLUT_SILENCE_DEPRECATION
        #include <OpenGL/gl.h>
        #include <GLUT/glut.h>
    #else
        #include <GL/gl.h>
        #include <GL/glut.h>
    #endif
#endif

#include <iostream>
#include <math.h>
#include <vector>

Cell::Cell(long double x, long double y, long double width, long double imagewidth, const std::pair<long double, long double> *realMin, const std::pair<long double, long double> *imageMin) : x(x), y(y), width(width), imagewidth(imagewidth) {
	imagex = abs(((realMin->first - x) / width ) * imagewidth) + imageMin->first;
    imagey = abs(((realMin->second - y) / width ) * imagewidth) + imageMin->second;

	counter = 0;

	complex = ComplexNumber(x, y);
}

void Cell::escape(ComplexNumber *c, std::vector<std::vector<Cell*>> *cells, const std::pair<long double, long double> *realMin, unsigned int iterations, unsigned int cellsPerRow, unsigned int *maxCount, bool anti) {
	ComplexNumber z = ComplexNumber();
	std::vector<Cell*> visited;
	visited.clear();

	for (unsigned int i = 0; i <= iterations; ++i) {
        z = z*z + *c;
		int visitedx = floor((z.real - realMin->first) / cells->at(0)[0]->width);
		int visitedy = floor((z.imag - realMin->second) / cells->at(0)[0]->width);

        if (visitedx <= (cellsPerRow - 1) && visitedy <= (cellsPerRow - 1) && visitedx >= 0 && visitedy >= 0 && i != 0)
			visited.push_back(cells->at(visitedx)[visitedy]);
        else if (i != 0)
			break;
	}

	if (anti ? z.abs() < 2.0 : z.abs() > 2.0) { // regular buddhabrot means that the point escapes, thus > 2.0 for !anti
		for (Cell *escapedbox : visited) {
			escapedbox->counter++;
            if (*maxCount < escapedbox->counter)
                *maxCount = escapedbox->counter;
		}
	}
}

#if USE_OPENGL
void Cell::render(unsigned int maxCount, unsigned int colourR, unsigned int colourG, unsigned int colourB) {
    long double percentageOfMax = log(counter) / log(maxCount);
    long double brightness = percentageOfMax > 0.25 ? percentageOfMax : 0.0;
    glColor4f(colourR / 255.0f , colourG / 255.0f, colourB / 255.0f, brightness);
    glRectd(imagey, imagex, imagey + imagewidth, imagex + imagewidth); // x and y flipped to render it vertically
}
#endif
