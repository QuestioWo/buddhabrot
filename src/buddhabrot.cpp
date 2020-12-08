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
#define GLUT_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <getopt.h>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <vector>

#define GL_CANVAS_MIN_X double(-1.0)
#define GL_CANVAS_MAX_X double(1.0)
#define GL_CANVAS_MIN_Y double(-1.0)
#define GL_CANVAS_MAX_Y double(1.0)

void displayCallback();
static void showUsage(std::string name);

static unsigned int ITERATIONS = 500;
static unsigned int WINDOW_WIDTH = 501;
static unsigned int CELLS_PER_ROW = WINDOW_WIDTH;
static unsigned int NUM_THREADS = std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 4;
static unsigned int COLOUR_R = 0;
static unsigned int COLOUR_G = 0;
static unsigned int COLOUR_B = 255;

static std::vector<std::vector<Cell*>> g_cells = {};
static unsigned int g_maxCount = 0;

int main(int argc, char *argv[]) {
    // parse arguments
    int option;
    if (argc < 2)
        showUsage(argv[0]);
    
    // put ':' at the starting of the string so compiler can distinguish between '?' and ':'
    while((option = getopt(argc, argv, ":w:p:i:r:g:b:t:")) != -1) {
        switch(option) {
            case 'w' :
                WINDOW_WIDTH = std::stoi(optarg);
                CELLS_PER_ROW = WINDOW_WIDTH;
                break;
            case 'p' :
                CELLS_PER_ROW = std::stoi(optarg);
                break;
            case 'i' :
                ITERATIONS = std::stoi(optarg);
                break;
            case 'r' :
                COLOUR_R = std::stoi(optarg);
                break;
            case 'g' :
                COLOUR_G = std::stoi(optarg);
                break;
            case 'b' :
                COLOUR_B = std::stoi(optarg);
                break;
            case 't' :
                NUM_THREADS = std::stoi(optarg);
                break;
            case ':' :
                std::cout << ("Option needs a value") << std::endl;
                break;
            case '?' :
                if (optopt == 'h') {
                    showUsage(argv[0]);
                    return -1;
                }
                printf("Unknown option: %c\n", optopt);
                showUsage(argv[0]);
                break;
        }
    }
    
    // print what buddhabrot will be generated
    std::cout << "Generating buddhabrot with arguments :" << std::endl;
    printf("\tpixels size : %d x %d\n", CELLS_PER_ROW, CELLS_PER_ROW);
    printf("\twindow size : %d x %d\n", WINDOW_WIDTH, WINDOW_WIDTH);
    printf("\titerations : %d\n", ITERATIONS);
    printf("\tthreads : %d\n", NUM_THREADS);
    printf("\tcolour : %d x %d x %d\n", COLOUR_R, COLOUR_G, COLOUR_B);
    std::cout << std::endl;
    
    // calculate buddhabrot
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
    
    std::vector<std::thread*> threads;
    
	for (std::vector<Cell*> h : g_cells) {
		for (Cell *cell : h) {
            if (threads.size() >= (NUM_THREADS - 1)) { // - 1 due to the "main" thread as well
                threads[0]->join();
                delete threads[0];
                threads.erase(threads.begin());
            }
            threads.push_back(new std::thread(Cell::escape, &cell->complex, &g_cells, &min, ITERATIONS, CELLS_PER_ROW, &g_maxCount));
		}
	}

    for (unsigned int i = 0; i < threads.size(); ++i)
        threads[i]->join();
    
    std::cout << "Calculated fractal" << std::endl;
    
    // display buddhabrot
	glutInit(&argc, argv);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_WIDTH);
    glutInitDisplayMode(GLUT_RGBA);
    glutCreateWindow("Buddhabrot");
    
    glutDisplayFunc(displayCallback);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glutMainLoop();

	return 0;
}

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    for (std::vector<Cell*> h : g_cells) {
        for (Cell* cell : h) {
            cell->render(g_maxCount, COLOUR_R, COLOUR_G, COLOUR_B);
        }
    }
    
    glFlush();
}

static void showUsage(std::string name) {
    std::cerr << "Usage: " << name << " <option(s)> SOURCES"
        << "Options:\n"
        << "\t-h,\t\t\tShow this help message\n"
        << "\t-w WINDOW_WIDTH\t\tSpecify the width of the window\t\t\t\t\t defaults to 501\n"
        << "\t-p CELLS_PER_ROW\tSpecify the number of 'boxes' per row\t\t\t\t defaults to WINDOW_WIDTH\n"
        << "\t-i ITERATIONS\t\tSpecify the number of iterations to be performed on each point\t defaults to 500\n"
        << "\t-r COLOUR_R\t\tSpecify the red component of the render's colour\t\t defaults to 0\n"
        << "\t-g COLOUR_G\t\tSpecify the green component of the render's colour\t\t defaults to 0\n"
        << "\t-b COLOUR_B\t\tSpecify the blue component of the render's colour\t\t defaults to 255\n"
        << "\t-t NUM_THREADS\t\tSpecify the number of threads to be used to compute the fractals defaults to the number of CPU cores\n"
        << std::endl << std::endl;
}
