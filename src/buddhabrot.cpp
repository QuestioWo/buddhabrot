//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#include "Cell.hpp"
#include "KernelHelper.hpp"
#include "io/PNGReader.hpp"
#include "io/CSVReader.hpp"

#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #define GLUT_SILENCE_DEPRECATION
    #include <OpenGL/gl.h>
    #include <GLUT/glut.h>
#else
    #include <GL/gl.h>
    #include <GL/glut.h>
#endif

#include <getopt.h>
#include <iostream>
#include <math.h>
#include <sstream>
#include <thread>
#include <vector>

static const long double GL_CANVAS_MIN_X = -1.0;
static const long double GL_CANVAS_MAX_X = 1.0;
static const long double GL_CANVAS_MIN_Y = -1.0;
//static const long double GL_CANVAS_MAX_Y = 1.0;
static const long double REAL_DIFF = 3.5;
static const std::pair<long double, long double> MIN = { -2.5, -1.75 };
//static const std::pair<long double, long double> MAX = { MIN.first + REAL_DIFF, MIN.second + REAL_DIFF };
static const std::pair<long double, long double> IMAGE_MIN = { GL_CANVAS_MIN_X, GL_CANVAS_MIN_Y };

static void displayCallback();
static void showUsage(std::string name);
static void executeRowsEscapes(unsigned int threadId, unsigned int threadsTotal);

static unsigned int ITERATIONS = 500;
static unsigned int WINDOW_WIDTH = 501;
static unsigned int CELLS_PER_ROW = WINDOW_WIDTH;
static unsigned char NUM_THREADS = std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 4;
static unsigned char COLOUR_R = 0;
static unsigned char COLOUR_G = 0;
static unsigned char COLOUR_B = 255;
static bool anti = false;
static bool save = false;
static bool load = false;
static bool alpha = false;
static bool useGpu = false;
static std::string SAVE_FILE_NAME;
static std::string LOAD_FILE_NAME;

static std::vector<std::vector<Cell*>> g_cellsClass = {};
static Real *g_cellsGPU;
static unsigned int g_maxCount = 0;

int main(int argc, char *argv[]) {
    // parse arguments
    int option;
    if (argc < 2)
        showUsage(argv[0]);
    
    // put ':' at the starting of the string so compiler can distinguish between '?' and ':'
    std::string tmp;
    std::stringstream lineStream;

    while((option = getopt(argc, argv, ":ao4s:l:w:p:i:c:t:")) != -1) {
        switch(option) {
            case 'a' :
                anti = true;
                break;
                
            case 's' :
                save = true;
                SAVE_FILE_NAME = std::string(optarg);
                break;
                
            case 'l' :
                load = true;
                LOAD_FILE_NAME = std::string(optarg);
                useGpu = true;
                break;
            
            case '4' :
                alpha = true;
                break;
            
            case 'o' :
                useGpu = true;
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
                
            case 'c' :
                lineStream = std::stringstream(std::string(optarg));
                
                std::getline(lineStream, tmp, ',');
                COLOUR_R = (unsigned char)std::stoi(tmp);
                
                std::getline(lineStream, tmp, ',');
                COLOUR_G = (unsigned char)std::stoi(tmp);
                
                std::getline(lineStream, tmp, ',');
                COLOUR_B = (unsigned char)std::stoi(tmp);
                break;
                
            case 't' :
                NUM_THREADS = (unsigned char)std::stoi(optarg);
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
    std::string saveLoc = save ? SAVE_FILE_NAME.c_str() : "N/A";
    std::cout << std::endl << std::string(load ? "Loading " : "Generating ") + std::string(anti ? "anti-" : "") + "buddhabrot with arguments :" << std::endl;
    printf("\tpixels size\t\t %d x %d\n", CELLS_PER_ROW, CELLS_PER_ROW);
    printf("\twindow size\t\t %d x %d\n", WINDOW_WIDTH, WINDOW_WIDTH);
    printf("\titerations\t\t %d\n", ITERATIONS);
    printf("\tgenerate with GPU\t %s\n", useGpu ? "true" : "false");
    printf("\tthreads\t\t\t %s\n", useGpu ? "N/A" : std::to_string(NUM_THREADS).c_str());
    printf("\tcolour\t\t\t (%d, %d, %d)\n", COLOUR_R, COLOUR_G, COLOUR_B);
    printf("\tsave to\t\t\t %s.png and %s.csv\n", saveLoc.c_str(), saveLoc.c_str());
    printf("\tpng with alpha\t\t %s\n", save ? alpha ? "true" : "false" : "N/A");
    
    std::cout << std::endl;
    
    // calculate buddhabrot
    long double cellImageWidth = (GL_CANVAS_MAX_X - GL_CANVAS_MIN_X) / double(CELLS_PER_ROW);
	long double cellRealWidth = REAL_DIFF / CELLS_PER_ROW;
    
    if (load) {
        CSVReader csv((char*)LOAD_FILE_NAME.c_str(), CELLS_PER_ROW);
        g_cellsGPU = csv.read();
        
        for (unsigned int i = 0; i < CELLS_PER_ROW; ++i) {
            for (unsigned int j = 0; j < CELLS_PER_ROW; ++j)
                g_maxCount = std::max(g_maxCount, (unsigned int)g_cellsGPU[i * CELLS_PER_ROW * 3 + j * 3 + 2]);
        }
        
        std::cout << "Loaded fractal" << std::endl;
    } else {
        if (useGpu)
            calculateCells(&g_cellsGPU, &g_maxCount, &ITERATIONS, &CELLS_PER_ROW, &MIN, &cellRealWidth, &anti);
        else {
            std::vector<Cell*> holding;

            for (unsigned int i = 0; i < CELLS_PER_ROW; ++i) {
                holding.clear();
                for (unsigned int j = 0; j < CELLS_PER_ROW; ++j) {
                    long double realx = MIN.first + cellRealWidth * (CELLS_PER_ROW - 1 - i); // inverting iteration through cells on the x-axis so that the buddhabrot renders "sitting-down" -- more picturesque
                    long double realy = MIN.second + cellRealWidth * j;
                    holding.push_back(new Cell(realx, realy, cellRealWidth, cellImageWidth, &MIN, &IMAGE_MIN));
                }

                g_cellsClass.push_back(holding);
            }
            
            g_cellsClass.shrink_to_fit();

            std::cout << "Memory successfully yoinked" << std::endl;
            
            std::vector<std::thread*> threads;
            
            for (unsigned int i = 0; i < (NUM_THREADS - 1); ++i)
                threads.push_back(new std::thread(executeRowsEscapes, i, NUM_THREADS));
            
            executeRowsEscapes(NUM_THREADS, NUM_THREADS);

            for (unsigned int i = 0; i < threads.size(); ++i) {
                threads[i]->join();
                delete threads[i];
                threads.erase(threads.begin() + i);
            }
        }
        
        std::cout << "Calculated fractal" << std::endl;
    }
    
    std::cout << "Max count := " << g_maxCount << std::endl;
    
    // display buddhabrot
    if (!save) {
        glutInit(&argc, argv);
        glutInitWindowSize(WINDOW_WIDTH, WINDOW_WIDTH);
        glutInitDisplayMode(GLUT_RGBA);
        glutCreateWindow("Buddhabrot");
        
        glutDisplayFunc(displayCallback);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        glutMainLoop();
    } else {
        PNGReader picture((char*)(SAVE_FILE_NAME + ".png").c_str(), WINDOW_WIDTH, CELLS_PER_ROW, COLOUR_R, COLOUR_G, COLOUR_B, g_maxCount, alpha);
        CSVReader csv((char*)(SAVE_FILE_NAME + ".csv").c_str(), CELLS_PER_ROW);
        
        if (useGpu) {
            picture.write(g_cellsGPU);
            csv.write(g_cellsGPU);
        } else {
            picture.write(&g_cellsClass);
            csv.write(&g_cellsClass);
        }
    }

	return 0;
}

static void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (useGpu) {
        long double cellRealWidth = REAL_DIFF / CELLS_PER_ROW;
        long double cellImageWidth = (GL_CANVAS_MAX_X - GL_CANVAS_MIN_X) / double(CELLS_PER_ROW);
        
        for (unsigned int i = 0; i < CELLS_PER_ROW; ++i) {
            for (unsigned int j = 0; j < CELLS_PER_ROW; ++j) {
                long double imagex = abs(((MIN.first - g_cellsGPU[i * CELLS_PER_ROW * 3 + j * 3 + 0]) / cellRealWidth ) * cellImageWidth) + IMAGE_MIN.first;
                long double imagey = abs(((MIN.second - g_cellsGPU[i * CELLS_PER_ROW * 3 + j * 3 + 1]) / cellRealWidth ) * cellImageWidth) + IMAGE_MIN.second;
                
                long double percentageOfMax = log(g_cellsGPU[i * CELLS_PER_ROW * 3 + j * 3 + 2]) / log(g_maxCount);
                long double brightness = percentageOfMax > 0.25 ? percentageOfMax : 0.0;
                glColor4f(COLOUR_R / 255.0f , COLOUR_G / 255.0f, COLOUR_B / 255.0f, brightness);
                glRectd(imagey, imagex, imagey + cellImageWidth, imagex + cellImageWidth); // x and y flipped to render it vertically
            }
        }
    } else {
        for (std::vector<Cell*> h : g_cellsClass) {
            for (Cell* cell : h)
                cell->render(g_maxCount, COLOUR_R, COLOUR_G, COLOUR_B);
        }
    }
    
    glFlush();
}

static void showUsage(std::string name) {
    std::cerr << "Usage: " << name << " <option(s)>\n"
        << "Options:\n"
        << "\t-h\n\t\t Show this help message\n\n"
        << "\t-a\n\t\t Generate an anti-buddhabrot\n\t\t defaults to false\n\n"
        << "\t-o\n\t\t Calculate the buddhabrot using OpenCL, i.e using GPU\n\t\t defaults to false\n\n"
        << "\t-4\n\t\t Generate png with alpha based-brightness; viewer dependant\n\t\t defaults to false\n\n"
        << "\t-s FILE_NAME\n\t\t Saves buddhabrot as a png and csv to the specified (SAVE_FILE_NAME + '.png'/'.csv')\n\t\t defaults to not save\n\n"
        << "\t-l FILE_NAME\n\t\t Loads buddhabrot from specified plaintext file. If correct -p not known, lines in sqrt(FILE_NAME - 1)\n\t\t defaults to not load\n\n"
        << "\t-w WINDOW_WIDTH\n\t\t Specify the width of the window\n\t\t defaults to 501\n\n"
        << "\t-p CELLS_PER_ROW\n\t\t Specify the number of 'boxes' per row\n\t\t defaults to WINDOW_WIDTH\n\n"
        << "\t-i ITERATIONS\n\t\t Specify the number of iterations to be performed on each point\n\t\t defaults to 500\n\n"
        << "\t-c COLOUR_R,COLOUR_G,COLOUR_B\n\t\t Specify the render's colour\n\t\t defaults to 0,0,255\n\n"
        << "\t-t NUM_THREADS\n\t\t Specify the number of threads to be used to compute the fractals\n\t\t defaults to the number of findable threads or 4\n"
        << std::endl;
}

static void executeRowsEscapes(unsigned int threadId, unsigned int threadsTotal) {
    unsigned int groups = CELLS_PER_ROW / threadsTotal;
    for (unsigned int i = 0; i < groups; ++i) {
        unsigned int currentColumn = i * threadsTotal + threadId;
        
        if (CELLS_PER_ROW > currentColumn) {
            std::vector<Cell*> h = g_cellsClass[currentColumn];
            for (Cell *cell : h)
                Cell::escape(&cell->complex, &g_cellsClass, &MIN, ITERATIONS, CELLS_PER_ROW, &g_maxCount, anti);
        }
    }
    
    if (CELLS_PER_ROW > groups * threadsTotal + threadId) {
        std::vector<Cell*> h = g_cellsClass[groups * threadsTotal + threadId];
        for (Cell *cell : h)
            Cell::escape(&cell->complex, &g_cellsClass, &MIN, ITERATIONS, CELLS_PER_ROW, &g_maxCount, anti);
    }
}
