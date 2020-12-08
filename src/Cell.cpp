//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "Cell.hpp"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <iostream>
#include <math.h>
#include <vector>

Cell::Cell(double x, double y, double width, double imagewidth, std::pair<double, double> *realMin, std::pair<double, double> *imageMin) : x(x), y(y), width(width), imagewidth(imagewidth) {
	imagex = abs(((realMin->first - x) / width ) * imagewidth) + imageMin->first;
    imagey = abs(((realMin->second - y) / width ) * imagewidth) + imageMin->second;

	counter = 0;

	complex = ComplexNumber(x, y);
}

void Cell::escape(ComplexNumber *c, std::vector<std::vector<Cell*>> *cells, std::pair<double, double> *realMin, unsigned int iterations, unsigned int cellsPerRow, unsigned int *maxCount) {
	ComplexNumber z = ComplexNumber();
	std::vector<Cell*> visited;
	visited.clear();

	for (unsigned int i = 0; i < iterations; ++i) {
        z = z*z + *c;
		int visitedx = floor((z.real - realMin->first) / cells->at(0)[0]->width);
		int visitedy = floor((z.imag - realMin->second) / cells->at(0)[0]->width);

        if (visitedx < (cellsPerRow - 1) && visitedy < (cellsPerRow - 1) && visitedx >= 0 && visitedy >= 0 && i != 0)
			visited.push_back(cells->at(visitedx)[visitedy]);
        else if (i != 0)
			break;
	}

	if (z.abs() > 2.0) {
		for (Cell *escapedbox : visited) {
			escapedbox->counter++;
            *maxCount = std::max(escapedbox->counter, *maxCount);
		}
	}
}

void Cell::render(unsigned int maxCount, unsigned int colourR, unsigned int colourG, unsigned int colourB) {
    double percentageOfMax = log(counter) / log(maxCount);
    double brightness = percentageOfMax > 0.25 ? percentageOfMax : 0.0;
    glColor4f(colourR / 255.0f , colourG / 255.0f, colourB / 255.0f, brightness);
    glRectd(imagey, imagex, imagey + imagewidth, imagex + imagewidth); // x and y flipped to render it vertically
}
