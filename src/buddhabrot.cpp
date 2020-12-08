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

static const long double GL_CANVAS_MIN_X = -1.0;
static const long double GL_CANVAS_MAX_X = 1.0;
static const long double GL_CANVAS_MIN_Y = -1.0;
//static const long double GL_CANVAS_MAX_Y = 1.0;

void displayCallback();
static void showUsage(std::string name);

static unsigned int ITERATIONS = 500;
static unsigned int WINDOW_WIDTH = 501;
static unsigned int CELLS_PER_ROW = WINDOW_WIDTH;
static unsigned int NUM_THREADS = std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 4;
static unsigned int COLOUR_R = 0;
static unsigned int COLOUR_G = 0;
static unsigned int COLOUR_B = 255;
static bool anti = false;

static std::vector<std::vector<Cell*>> g_cells = {};
static unsigned int g_maxCount = 0;

int main(int argc, char *argv[]) {
    // parse arguments
    int option;
    if (argc < 2)
        showUsage(argv[0]);
    
    // put ':' at the starting of the string so compiler can distinguish between '?' and ':'
    while((option = getopt(argc, argv, ":aw:p:i:r:g:b:t:")) != -1) {
        switch(option) {
            case 'a' :
                anti = true;
                break;
                
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
    std::cout << std::endl << "Generating buddhabrot with arguments :" << std::endl;
    printf("\tpixels size\t %d x %d\n", CELLS_PER_ROW, CELLS_PER_ROW);
    printf("\twindow size\t %d x %d\n", WINDOW_WIDTH, WINDOW_WIDTH);
    printf("\titerations\t %d\n", ITERATIONS);
    printf("\tthreads\t\t %d\n", NUM_THREADS);
    printf("\tcolour\t\t (%d, %d, %d)\n", COLOUR_R, COLOUR_G, COLOUR_B);
    std::cout << std::endl;
    
    // calculate buddhabrot
    long double realDiff = 3.5;

    std::pair<long double, long double> min = { -2.5, -1.75 };
	std::pair<long double, long double> max = { min.first + realDiff, min.second + realDiff };

    std::pair<long double, long double> imageMin = { GL_CANVAS_MIN_X, GL_CANVAS_MIN_Y };
    long double cellImageWidth = (GL_CANVAS_MAX_X - GL_CANVAS_MIN_X) / double(CELLS_PER_ROW);
	long double cellRealWidth = realDiff / CELLS_PER_ROW;

	std::vector<Cell*> holding;

	for (unsigned int i = 0; i < CELLS_PER_ROW; ++i) {
		holding.clear();
		for (unsigned int j = 0; j < CELLS_PER_ROW; ++j) {
			long double realx = min.first + cellRealWidth * (CELLS_PER_ROW - 1 - i); // inverting iteration through cells on the x-axis so that the buddhabrot renders "sitting-down" -- more picturesque
			long double realy = min.second + cellRealWidth * j;
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
            threads.push_back(new std::thread(Cell::escape, &cell->complex, &g_cells, &min, ITERATIONS, CELLS_PER_ROW, &g_maxCount, anti));
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
    std::cerr << "Usage: " << name << " <option(s)>\n"
        << "Options:\n"
        << "\t-h \t\t\t Show this help message\n"
        << "\t-a \t\t\t Generate an anti-buddhabrot \t\t\t\t\t  defaults to false\n"
        << "\t-w WINDOW_WIDTH \t Specify the width of the window \t\t\t\t  defaults to 501\n"
        << "\t-p CELLS_PER_ROW \t Specify the number of 'boxes' per row \t\t\t\t  defaults to WINDOW_WIDTH\n"
        << "\t-i ITERATIONS \t\t Specify the number of iterations to be performed on each point   defaults to 500\n"
        << "\t-r COLOUR_R \t\t Specify the red component of the render's colour \t\t  defaults to 0\n"
        << "\t-g COLOUR_G \t\t Specify the green component of the render's colour \t\t  defaults to 0\n"
        << "\t-b COLOUR_B \t\t Specify the blue component of the render's colour \t\t  defaults to 255\n"
        << "\t-t NUM_THREADS \t\t Specify the number of threads to be used to compute the fractals defaults to the number of CPU cores\n"
        << std::endl;
}
