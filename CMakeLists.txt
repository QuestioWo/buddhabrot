##===========================================================================##
###
### Copyright Jim Carty © 2020-2021
###
### This file is subject to the terms and conditions defined in file
### 'LICENSE.txt', which is part of this source code package.
###
##===========================================================================##

cmake_minimum_required( VERSION 3.0.2 )

project( buddhabrot )

add_compile_options( -std=c++11 )

# Linux specific requirements for threading
if (UNIX)
	set(CMAKE_THREAD_LIBS_INIT "-lpthread")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	set(CMAKE_HAVE_THREADS_LIBRARY 1)
	set(CMAKE_USE_WIN32_THREADS_INIT 0)
	set(CMAKE_USE_PTHREADS_INIT 1)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
endif(UNIX)

if (APPLE)
	option(USE_OPENGL "Build to be able to use the OpenGL and GLUT libraries" ON)
else ()
	set(USE_OPENGL OFF)
endif (APPLE)
option(OPENCL_DOUBLE_PRECISION "Uses double precision for GPU processing of fractal. Use clinfo to check if GPU has either cl_khr_fp64 or cl_amd_fp64 extensions before enabling" OFF)

message( "Compiling with options:" )
message( "\tUSE_OPENGL := " ${USE_OPENGL} )
message( "\tOPENCL_DOUBLE_PRECISION := " ${OPENCL_DOUBLE_PRECISION} )

# Find OpenGL and GLUT
if (USE_OPENGL)
	find_package(OpenGL REQUIRED)
	message(STATUS "OPENGL_FOUND := TRUE") # Would not run if not found as required
	find_package(GLUT REQUIRED)
	message(STATUS "GLUT_FOUND := TRUE")

	add_compile_options( -DUSE_OPENGL=true )
else ()
	add_compile_options( -DUSE_OPENGL=false )
endif (USE_OPENGL)

# Find OpenCL
find_package(OpenCL 1.2 REQUIRED)
message(STATUS "OpenCL_FOUND := TRUE")

if (OPENCL_DOUBLE_PRECISION)
	add_compile_options( -DDOUBLE_SUPPORT_AVAILABLE=true )
else ()
	add_compile_options( -DDOUBLE_SUPPORT_AVAILABLE=false )
endif (OPENCL_DOUBLE_PRECISION)

# Find OpenCV for image writing
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV_FOUND := TRUE")

# Find GoogleTest
find_package(GTest REQUIRED)
message(STATUS "GTest Found := TRUE")

# Add source files
file(GLOB_RECURSE SOURCE_FILES
	${CMAKE_SOURCE_DIR}/src/*.cpp)

set(TEST_SOURCE_FILES ${SOURCE_FILES})
list(REMOVE_ITEM TEST_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/buddhabrot.cpp)

file(GLOB_RECURSE TEST_TEST_FILES
	${CMAKE_SOURCE_DIR}/tests/*.cpp)

# Add header files
file(GLOB_RECURSE HEADER_FILES
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.hpp)

# Build Application
add_executable( ${PROJECT_NAME}
	${SOURCE_FILES}
	${HEADER_FILES}
)

include_directories(${PROJECT_NAME}
PUBLIC
	${OPENGL_INCLUDE_DIRS}
	${GLUT_INCLUDE_DIRS}
	${OpenCL_INCLUDE_DIRS}
	${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME}
	${OPENGL_LIBRARIES}
	${GLUT_LIBRARIES}
	${OpenCL_LIBRARIES}
	${OpenCV_LIBS}
)

# Build Test Application
set( PROJECT_NAME_TEST ${PROJECT_NAME}Test )
add_executable( ${PROJECT_NAME_TEST}
	${TEST_SOURCE_FILES}
	${TEST_TEST_FILES}
	${HEADER_FILES}
)

include_directories(${PROJECT_NAME_TEST}
PUBLIC
	${OPENGL_INCLUDE_DIRS}
	${GLUT_INCLUDE_DIRS}
	${OpenCL_INCLUDE_DIRS}
	${OpenCV_INCLUDE_DIRS}
	${GTest_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME_TEST}
	${OPENGL_LIBRARIES}
	${GLUT_LIBRARIES}
	${OpenCL_LIBRARIES}
	${OpenCV_LIBS}
	${GTEST_BOTH_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
)