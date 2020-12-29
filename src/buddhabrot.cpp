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

#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #define GLUT_SILENCE_DEPRECATION
    #include <OpenGL/gl.h>
    #include <GLUT/glut.h>
#else
    #include <GL/gl.h>
    #include <GL/glut.h>
#endif

#include <png.h>

#include <getopt.h>
#include <iostream>
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

static unsigned int ITERATIONS = 500;
static unsigned int WINDOW_WIDTH = 501;
static unsigned int CELLS_PER_ROW = WINDOW_WIDTH;
static unsigned char NUM_THREADS = std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 4;
static unsigned char COLOUR_R = 0;
static unsigned char COLOUR_G = 0;
static unsigned char COLOUR_B = 255;
static bool anti = false;
static bool save = false;
static bool alpha = false;
static bool useGpu = false;
static std::string FILE_NAME;

static std::vector<std::vector<Cell*>> g_cellsClass = {};
static Real *g_cellsGPU;
static unsigned int g_maxCount = 0;

int main(int argc, char *argv[]) {
    // parse arguments
    int option;
    if (argc < 2)
        showUsage(argv[0]);
    
    // put ':' at the starting of the string so compiler can distinguish between '?' and ':'
    while((option = getopt(argc, argv, ":ao4s:w:p:i:r:g:b:t:")) != -1) {
        switch(option) {
            case 'a' :
                anti = true;
                break;
                
            case 's' :
                save = true;
                FILE_NAME = std::string(optarg);
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
                
            case 'r' :
                COLOUR_R = (unsigned char)std::stoi(optarg);
                break;
                
            case 'g' :
                COLOUR_G = (unsigned char)std::stoi(optarg);
                break;
                
            case 'b' :
                COLOUR_B = (unsigned char)std::stoi(optarg);
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
    std::cout << std::endl << "Generating " + std::string(anti ? "anti-" : "") + "buddhabrot with arguments :" << std::endl;
    printf("\tpixels size\t\t %d x %d\n", CELLS_PER_ROW, CELLS_PER_ROW);
    printf("\twindow size\t\t %d x %d\n", WINDOW_WIDTH, WINDOW_WIDTH);
    printf("\titerations\t\t %d\n", ITERATIONS);
    printf("\tgenerate with GPU\t %s\n", useGpu ? "true" : "false");
    printf("\tthreads\t\t\t %s\n", useGpu ? "N/A" : std::to_string(NUM_THREADS).c_str());
    printf("\tcolour\t\t\t (%d, %d, %d)\n", COLOUR_R, COLOUR_G, COLOUR_B);
    printf("\tsave to\t\t\t %s\n", save ? std::string(FILE_NAME + ".png").c_str() : "N/A");
    printf("\tpng with alpha\t\t %s\n", save ? alpha ? "true" : "false" : "N/A");
    
    std::cout << std::endl;
    
    // calculate buddhabrot
    long double cellImageWidth = (GL_CANVAS_MAX_X - GL_CANVAS_MIN_X) / double(CELLS_PER_ROW);
	long double cellRealWidth = REAL_DIFF / CELLS_PER_ROW;

    if (useGpu) {
        calculateCells(&g_cellsGPU, &g_maxCount, &ITERATIONS, &CELLS_PER_ROW, &MIN, &cellRealWidth, &anti);
    } else {
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

        std::cout << "Memory successfully yoinked" << std::endl;
        
        std::vector<std::thread*> threads;
        
        for (std::vector<Cell*> h : g_cellsClass) {
            for (unsigned int i = 0; i < h.size(); ++i) {
                Cell *cell = h[i];
                if (i >= (NUM_THREADS - 1)) { // - 1 due to the "main" thread as well
                    threads[0]->join();
                    delete threads[0];
                    threads.erase(threads.begin());
                }
                threads.push_back(new std::thread(Cell::escape, &cell->complex, &g_cellsClass, &MIN, ITERATIONS, CELLS_PER_ROW, &g_maxCount, anti));
            }
        }

        for (unsigned int i = 0; i < threads.size(); ++i) {
            threads[i]->join();
            delete threads[i];
            threads.erase(threads.begin() + i);
        }
    }
    
    std::cout << "Calculated fractal" << std::endl;
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
        FILE *fp = fopen(std::string(FILE_NAME + ".png").c_str(), "wb");
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        png_infop info_ptr = png_create_info_struct(png_ptr);
        png_init_io(png_ptr, fp);

        // write header
        png_set_IHDR(png_ptr, info_ptr, WINDOW_WIDTH, WINDOW_WIDTH,
            8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);

        // write bytes
        png_bytep *row_pointers;
        row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * WINDOW_WIDTH);
        for (unsigned int i = 0; i < WINDOW_WIDTH; ++i) {
            row_pointers[i] = (png_byte*) malloc( sizeof(png_byte) * WINDOW_WIDTH * 4);

            png_bytep rpptr = row_pointers[i];
            for (unsigned int j = 0; j < WINDOW_WIDTH; ++j) {
                long double percentageOfMax;
                if (useGpu)
                    percentageOfMax = log(g_cellsGPU[i * CELLS_PER_ROW * 3 + j * 3 + 2]) / log(g_maxCount);
                else
                    percentageOfMax = log(g_cellsClass[i][j]->counter) / log(g_maxCount);
                
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
                        *rpptr = COLOUR_R;
                        rpptr++;
                        *rpptr = COLOUR_G;
                        rpptr++;
                        *rpptr = COLOUR_B;
                        rpptr++;

                        // Alpha
                        *rpptr = (unsigned char)(percentageOfMax * 255.0f);
                        rpptr++;
                        
                    } else {
                        // RGB
                        *rpptr = (unsigned char)(COLOUR_R * percentageOfMax);
                        rpptr++;
                        *rpptr = (unsigned char)(COLOUR_G * percentageOfMax);
                        rpptr++;
                        *rpptr = (unsigned char)(COLOUR_B * percentageOfMax);
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
        
        std::cout << "Saved fractal to " << FILE_NAME << ".png" << std::endl;
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
        << "\t-h \t\t\t Show this help message\n"
        << "\t-a \t\t\t Generate an anti-buddhabrot \t\t\t\t\t  defaults to false\n"
        << "\t-o \t\t\t Calculate the buddhabrot using OpenCL, i.e using GPU \t\t  defaults to false\n"
        << "\t-4 \t\t\t Generate png with alpha based-brightness; viewer dependant \t  defaults to false\n"
        << "\t-s FILE_NAME\t\t Saves buddhabrot as a png to the specified (FILE_NAME + '.png')  defaults to 'buddhabrot'\n"
        << "\t-w WINDOW_WIDTH \t Specify the width of the window \t\t\t\t  defaults to 501\n"
        << "\t-p CELLS_PER_ROW \t Specify the number of 'boxes' per row \t\t\t\t  defaults to WINDOW_WIDTH\n"
        << "\t-i ITERATIONS \t\t Specify the number of iterations to be performed on each point   defaults to 500\n"
        << "\t-r COLOUR_R \t\t Specify the red component of the render's colour \t\t  defaults to 0\n"
        << "\t-g COLOUR_G \t\t Specify the green component of the render's colour \t\t  defaults to 0\n"
        << "\t-b COLOUR_B \t\t Specify the blue component of the render's colour \t\t  defaults to 255\n"
        << "\t-t NUM_THREADS \t\t Specify the number of threads to be used to compute the fractals defaults to the number of CPU cores\n"
        << std::endl;
}
