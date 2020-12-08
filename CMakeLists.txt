cmake_minimum_required( VERSION 3.0.2 )

project( buddhabrot )

add_compile_options( -std=c++11 )

# Find OpenGL
find_package(OpenGL REQUIRED)
message(STATUS "OPENGL_FOUND:= ${OPENGL_FOUND}")
find_package(GLUT REQUIRED)
message(STATUS "GLUT_FOUND:= ${GLUT_FOUND}")

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.hpp)

# Build Application
include_directories( src )

add_executable( ${PROJECT_NAME}
	${SOURCE_FILES}
	${HEADER_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})