//===========================================================================//
///
/// Copyright Jim Carty © 2020
///
/// This file is subject to the terms and conditions defined in file
/// 'LICENSE.txt', which is part of this source code package.
///
//===========================================================================//

#ifdef _WIN32
#define NOMINMAX
#endif

#include "Cell.hpp"
#include "KernelHelper.hpp"
#include "io/PNGReader.hpp"
#include "io/CSVReader.hpp"

#if USE_OPENGL
    #ifdef __APPLE__
        #include <OpenGL/gl.h>
        #include <GLUT/glut.h>
    #else
        #include <GL/gl.h>
        #include <GL/glut.h>
    #endif
#endif

#include <algorithm>
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

#if USE_OPENGL
    static void displayCallback();
#endif
static void showUsage(std::string name);
static void executeRowsEscapes(unsigned int threadId, unsigned int threadsTotal);

static unsigned int iterations = 500;
static unsigned int iterationsMax = 0;
static unsigned int windowWidth = 501;
static unsigned int cellsPerRow = windowWidth;
static unsigned char numThreads = std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 4;
static unsigned char colourR = 0;
static unsigned char colourG = 0;
static unsigned char colourB = 255;
static bool anti = false;
static bool load = false;
static bool alpha = false;
static bool useGpu = false;

static std::string loadFileName;
#if USE_OPENGL
    static bool save = false;
    static std::string saveFileName;
#else
    static bool save = true;
    static std::string saveFileName = "buddhabrot";
#endif

static std::vector<std::vector<Cell*>> g_cellsClass = {};
static Real *g_cellsGPU;
static unsigned int g_maxCount = 0;

int main(int argc, char *argv[]) {
    // parse arguments
    if (argc < 2)
        showUsage(argv[0]);
    
    // put ':' at the starting of the string so compiler can distinguish between '?' and ':'
    std::string tmp;
    std::stringstream lineStream;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-a")) {
            anti = true;
            break;
        } else if (!strcmp(argv[i], "-s")) {
            save = true;
            saveFileName = std::string(argv[++i]);
            break;
        } else if (!strcmp(argv[i], "-l")) {
            load = true;
            loadFileName = std::string(argv[++i]);
            useGpu = true;
            break;
        } else if (!strcmp(argv[i], "-4")) {
            alpha = true;
            break;
        } else if (!strcmp(argv[i], "-o")) {
            useGpu = true;
            break;
        } else if (!strcmp(argv[i], "-w")) {
            windowWidth = std::stoi(argv[++i]);
            cellsPerRow = windowWidth;
            break;
        } else if (!strcmp(argv[i], "-p")) {
            cellsPerRow = std::stoi(argv[++i]);
            break;
        } else if (!strcmp(argv[i], "-i")) {
            iterations = std::stoi(argv[++i]);
            break;
        } else if (!strcmp(argv[i], "-m")) {
            iterationsMax = std::stoi(argv[++i]);
            break;
        } else if (!strcmp(argv[i], "-c")) {
            lineStream = std::stringstream(std::string(argv[++i]));
            
            std::getline(lineStream, tmp, ',');
            colourR = (unsigned char)std::stoi(tmp);
            
            std::getline(lineStream, tmp, ',');
            colourG = (unsigned char)std::stoi(tmp);
            
            std::getline(lineStream, tmp, ',');
            colourB = (unsigned char)std::stoi(tmp);
            break;
        } else if (!strcmp(argv[i], "-t")) {
            numThreads = (unsigned char)std::stoi(argv[++i]);
            break;
        } else if (!strcmp(argv[i], "-h")) {
            showUsage(argv[0]);
            return -1;
        } else {
            printf("Unknown option: %c\n", argv[i]);
            showUsage(argv[0]);
            break;
        }
    }
    
    // print what buddhabrot will be generated
    std::string saveLoc = save ? saveFileName.c_str() : "N/A";
    std::cout << std::endl << std::string(load ? "Loading " : "Generating ") + std::string(anti ? "anti-" : "") + "buddhabrot with arguments :" << std::endl;
    printf("\tpixels size\t\t %d x %d\n", cellsPerRow, cellsPerRow);
    printf("\twindow size\t\t %d x %d\n", windowWidth, windowWidth);
    printf("\titerations\t\t %d\n", iterations);
    printf("\tgenerate with GPU\t %s\n", useGpu ? "true" : "false");
    printf("\tthreads\t\t\t %s\n", useGpu ? "N/A" : std::to_string(numThreads).c_str());
    printf("\tcolour\t\t\t (%d, %d, %d)\n", colourR, colourG, colourB);
    printf("\tsave to\t\t\t %s.png and %s.csv\n", saveLoc.c_str(), saveLoc.c_str());
    printf("\tpng with alpha\t\t %s\n", save ? alpha ? "true" : "false" : "N/A");
    
    std::cout << std::endl;
    
    // calculate buddhabrot
    long double cellImageWidth = (GL_CANVAS_MAX_X - GL_CANVAS_MIN_X) / double(cellsPerRow);
	long double cellRealWidth = REAL_DIFF / cellsPerRow;
    
    if (load) {
        CSVReader csv((char*)loadFileName.c_str(), cellsPerRow);
        g_cellsGPU = csv.read();
        
        for (unsigned int i = 0; i < cellsPerRow; ++i) {
            for (unsigned int j = 0; j < cellsPerRow; ++j)
                g_maxCount = std::max(g_maxCount, (unsigned int)g_cellsGPU[i * cellsPerRow * 3 + j * 3 + 2]);
        }
        
        std::cout << "Loaded fractal" << std::endl;
    } else {
        if (useGpu)
            calculateCells(&g_cellsGPU, &g_maxCount, &iterations, &cellsPerRow, &MIN, &cellRealWidth, &anti, &iterationsMax);
        else {
            std::vector<Cell*> holding;

            for (unsigned int i = 0; i < cellsPerRow; ++i) {
                holding.clear();
                for (unsigned int j = 0; j < cellsPerRow; ++j) {
                    long double realx = MIN.first + cellRealWidth * (cellsPerRow - 1 - i); // inverting iteration through cells on the x-axis so that the buddhabrot renders "sitting-down" -- more picturesque
                    long double realy = MIN.second + cellRealWidth * j;
                    holding.push_back(new Cell(realx, realy, cellRealWidth, cellImageWidth, &MIN, &IMAGE_MIN));
                }

                g_cellsClass.push_back(holding);
            }
            
            g_cellsClass.shrink_to_fit();

            std::cout << "Memory successfully yoinked" << std::endl;
            
            std::vector<std::thread*> threads;
            
            for (unsigned int i = 0; i < (numThreads - 1); ++i)
                threads.push_back(new std::thread(executeRowsEscapes, i, numThreads));
            
            executeRowsEscapes(numThreads, numThreads);

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
#if USE_OPENGL
    if (!save) {
        glutInit(&argc, argv);
        glutInitWindowSize(windowWidth, windowWidth);
        glutInitDisplayMode(GLUT_RGBA);
        glutCreateWindow("Buddhabrot");
        
        glutDisplayFunc(displayCallback);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        glutMainLoop();
    } else {
#endif
        PNGReader picture((char*)(saveFileName + ".png").c_str(), windowWidth, cellsPerRow, colourR, colourG, colourB, g_maxCount, alpha);
        CSVReader csv((char*)(saveFileName + ".csv").c_str(), cellsPerRow);
        
        if (useGpu) {
            picture.write(g_cellsGPU);
            csv.write(g_cellsGPU);
        } else {
            picture.write(&g_cellsClass);
            csv.write(&g_cellsClass);
        }
#if USE_OPENGL
    }
#endif

	return 0;
}

#if USE_OPENGL
static void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (useGpu) {
        long double cellRealWidth = REAL_DIFF / cellsPerRow;
        long double cellImageWidth = (GL_CANVAS_MAX_X - GL_CANVAS_MIN_X) / double(cellsPerRow);
        
        for (unsigned int i = 0; i < cellsPerRow; ++i) {
            for (unsigned int j = 0; j < cellsPerRow; ++j) {
                long double imagex = abs(((MIN.first - g_cellsGPU[i * cellsPerRow * 3 + j * 3 + 0]) / cellRealWidth ) * cellImageWidth) + IMAGE_MIN.first;
                long double imagey = abs(((MIN.second - g_cellsGPU[i * cellsPerRow * 3 + j * 3 + 1]) / cellRealWidth ) * cellImageWidth) + IMAGE_MIN.second;
                
                long double percentageOfMax = log(g_cellsGPU[i * cellsPerRow * 3 + j * 3 + 2]) / log(g_maxCount);
                long double brightness = percentageOfMax > 0.25 ? percentageOfMax : 0.0;
                glColor4f(colourR / 255.0f , colourG / 255.0f, colourB / 255.0f, brightness);
                glRectd(imagey, imagex, imagey + cellImageWidth, imagex + cellImageWidth); // x and y flipped to render it vertically
            }
        }
    } else {
        for (std::vector<Cell*> h : g_cellsClass) {
            for (Cell* cell : h)
                cell->render(g_maxCount, colourR, colourG, colourB);
        }
    }
    
    glFlush();
}
#endif

static void showUsage(std::string name) {
    std::cerr << "Usage: " << name << " <option(s)>\n"
        << "Options:\n"
        << "\t-h\n\t\t Show this help message\n\n"
        << "\t-a\n\t\t Generate an anti-buddhabrot\n\t\t defaults to false\n\n"
        << "\t-o\n\t\t Calculate the buddhabrot using OpenCL, i.e using GPU\n\t\t defaults to false\n\n"
        << "\t-4\n\t\t Generate png with alpha based-brightness; viewer dependant\n\t\t defaults to false\n\n"
#if USE_OPENGL
        << "\t-s FILE_NAME\n\t\t Saves buddhabrot as a png and csv to the specified\n\t\t (FILE_NAME + '.png'/'.csv')\n\t\t defaults to not save\n\n"
#else
        << "\t-s FILE_NAME\n\t\t Saves buddhabrot as a png and csv to the specified\n\t\t (FILE_NAME + '.png'/'.csv')\n\t\t defaults to 'buddhabrot'\n\n"
#endif
        << "\t-l FILE_NAME\n\t\t Loads buddhabrot from specified plaintext file. If correct -p\n\t\t not known, sqrt(lines in FILE_NAME - 1)\n\t\t defaults to not load\n\n"
        << "\t-w WINDOW_WIDTH\n\t\t Specify the width of the window\n\t\t defaults to 501\n\n"
        << "\t-p CELLS_PER_ROW\n\t\t Specify the number of 'boxes' per row\n\t\t defaults to WINDOW_WIDTH\n\n"
        << "\t-i ITERATIONS\n\t\t Specify the number of iterations to be performed on each point\n\t\t defaults to 500\n\n"
        << "\t-m ITERATIONS_MAX\n\t\t Specify the number of iterations per group to be ran by the\n\t\t GPU. Important as prevents hanging GPU for larger ITERATIONS\n\t\t values. CELLS_PER_ROW is the largest influence of how large the\n\t\t ITERATION_MAX value can be. e.g 2001 can run at around 50000\n\t\t per group and 4501 at around 10000\n\t\t defaults to use estimation for maximum\n\n"
        << "\t-c COLOUR_R,COLOUR_G,COLOUR_B\n\t\t Specify the render's colour\n\t\t defaults to 0,0,255\n\n"
        << "\t-t NUM_THREADS\n\t\t Specify the number of threads to be used to compute the fractal\n\t\t defaults to the number of findable threads or 4\n"
        << std::endl;
}

static void executeRowsEscapes(unsigned int threadId, unsigned int threadsTotal) {
    unsigned int groups = cellsPerRow / threadsTotal;
    for (unsigned int i = 0; i < groups; ++i) {
        unsigned int currentColumn = i * threadsTotal + threadId;
        
        if (cellsPerRow > currentColumn) {
            std::vector<Cell*> h = g_cellsClass[currentColumn];
            for (Cell *cell : h)
                Cell::escape(&cell->complex, &g_cellsClass, &MIN, iterations, cellsPerRow, &g_maxCount, anti);
        }
    }
    
    if (cellsPerRow > groups * threadsTotal + threadId) {
        std::vector<Cell*> h = g_cellsClass[groups * threadsTotal + threadId];
        for (Cell *cell : h)
            Cell::escape(&cell->complex, &g_cellsClass, &MIN, iterations, cellsPerRow, &g_maxCount, anti);
    }
}
