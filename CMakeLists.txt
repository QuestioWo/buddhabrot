cmake_minimum_required( VERSION 3.0.2 )

project( buddhabrot )

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
    message(FATAL_ERROR "Require at least gcc-8.0")
endif()

add_compile_options( -std=c++11 )

# Linux specific requirements for threading
if (UNIX)
	set(CMAKE_THREAD_LIBS_INIT "-lpthread")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	set(CMAKE_HAVE_THREADS_LIBRARY 1)
	set(CMAKE_USE_WIN32_THREADS_INIT 0)
	set(CMAKE_USE_PTHREADS_INIT 1)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
endif(UNIX)

# Find OpenGL and GLUT
find_package(OpenGL REQUIRED)
message(STATUS "OPENGL_FOUND := TRUE") # Would not run if not found as required
find_package(GLUT REQUIRED)
message(STATUS "GLUT_FOUND := TRUE")
find_package(OpenCL 1.2 EXACT REQUIRED)
message(STATUS "OPENCL_FOUND := TRUE")

# Find libpng using PkgConfig
find_package(PNG REQUIRED)
message(STATUS "PNG_FOUND := TRUE")

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.hpp)

# Build Application
add_executable( ${PROJECT_NAME}
	${SOURCE_FILES}
	${HEADER_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${OpenCL_LIBRARIES} ${PNG_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${OpenCL_LIBRARIES} ${PNG_LIBRARIES})