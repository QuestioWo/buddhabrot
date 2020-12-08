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

#define WINDOW_WIDTH 801
#define GL_CANVAS_MIN_X double(-1.0)
#define GL_CANVAS_MAX_X double(1.0)
#define GL_CANVAS_MIN_Y double(-1.0)
#define GL_CANVAS_MAX_Y double(1.0)
#define CELLS_PER_ROW 801
#define ITERATIONS 2500

#include <iostream>
#include <stdio.h>
#include <vector>

void displayCallback();

static std::vector<std::vector<Cell*>> g_cells = {};
static unsigned int g_maxCount = 0;

int main(int argc, char **argv) {
    double realDiff = 3.5;

    std::pair<double, double> min = { -2.5, -1.75 };
	std::pair<double, double> max = { min.first + realDiff, min.second + realDiff };

    std::pair<double, double> imageMin = { GL_CANVAS_MIN_X, GL_CANVAS_MIN_Y };
    double cellImageWidth = (GL_CANVAS_MAX_X - GL_CANVAS_MIN_X) / double(CELLS_PER_ROW);
	double cellRealWidth = realDiff / CELLS_PER_ROW;

	std::vector<Cell*> holding;

	for (unsigned int i = 0; i < CELLS_PER_ROW; ++i) {
		holding.clear();
		for (unsigned int j = 0; j < CELLS_PER_ROW; ++j) {
			double realx = min.first + cellRealWidth * (CELLS_PER_ROW - 1 - i); // inverting iteration through cells on the x-axis so that the buddhabrot renders "sitting-down" -- more picturesque
			double realy = min.second + cellRealWidth * j;
            holding.push_back(new Cell(realx, realy, cellRealWidth, cellImageWidth, &min, &imageMin));
		}

		g_cells.push_back(holding);
	}

    std::cout << "Memory successfully yoinked" << std::endl;
    
	for (std::vector<Cell*> h : g_cells) {
		for (Cell *cell : h) {
            g_maxCount = Cell::escape(&cell->complex, &g_cells, &min, ITERATIONS, CELLS_PER_ROW, g_maxCount);
		}
	}

	glutInit(&argc, argv);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_WIDTH);
    glutInitDisplayMode(GLUT_RGBA);
    glutCreateWindow("Buddhabrot");
    
    glutDisplayFunc(displayCallback);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glutMainLoop();

	return 0;
}

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    for (std::vector<Cell*> h : g_cells) {
        for (Cell* cell : h) {
            cell->render(g_maxCount);
        }
    }
    
    glFlush();
}
