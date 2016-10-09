cmake_minimum_required(VERSION 3.4)
project(task1)

find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
add_definitions(-Wall -Wextra -pedantic -Werror -O3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES main.cpp)
add_executable(task1 ${SOURCE_FILES})
target_link_libraries(task1 ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
