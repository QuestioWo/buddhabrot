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

#define SCREEN_WIDTH 701

#include <iostream>
#include <stdio.h>
#include <vector>

void displayCallback();

int main(int argc, char **argv) {
	unsigned int iterations = 500;

	double realDiff = 3.5;

    std::pair<double, double> min = { -2.5, -1.75 };
	std::pair<double, double> max = { min.first + realDiff, min.second
			+ realDiff };

	unsigned int cellsPerRow = 35;

	double cellImageWidth = SCREEN_WIDTH / cellsPerRow;
	double cellRealWidth = realDiff / cellsPerRow;

	std::vector<std::vector<Cell*>> cells;
	std::vector<Cell*> holding;

	for (unsigned int i = 0; i < cellsPerRow; ++i) {
		holding.clear();
		for (unsigned int j = 0; j < cellsPerRow; ++j) {
			double realx = min.first + cellRealWidth * i;
			double realy = min.second + cellRealWidth * j;
			holding.push_back(
					new Cell(realx, realy, cellRealWidth, cellImageWidth, min,
							max));
		}

		cells.push_back(holding);
	}

	unsigned int maxCount = 0;

	std::vector<std::vector<Cell *>> countedCells(cells);

	for (std::vector<Cell*> h : cells) {
		for (Cell* cell : h) {
			Cell::escape(cell, &countedCells, min, iterations, cellsPerRow);
		}
	}

	for (std::vector<Cell*> h : cells) {
		for (Cell* cell : h) {
			std::cout << cell->counter << ", ";
			maxCount = std::max(cell->counter, maxCount);
		}
		std::cout << std::endl;
	}

	std::cout << maxCount << std::endl;

	glutInit(&argc, argv);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_WIDTH);
    glutInitDisplayMode(GLUT_RGBA);
    glutCreateWindow("Buddhabrot");
    
    glutDisplayFunc(displayCallback);
    
    glutMainLoop();

	return 0;
}

void displayCallback() {
    glutSwapBuffers();
}
