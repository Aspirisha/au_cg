cmake_minimum_required(VERSION 3.4)
project(task1)
INCLUDE(FindPkgConfig)

find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)



pkg_search_module(GLFW REQUIRED glfw3)

include_directories( ${OPENGL_INCLUDE_DIRS} ${GlmIncludeDir}
        ${GLFW_INCLUDE_DIRS} )
add_definitions(-Wall -Wextra -pedantic -O3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(SOURCE_FILES main.cpp ShaderLoader.cpp ShaderLoader.h Renderer.h SimpleRenderer.h SimpleRenderer.cpp)

add_executable(task1 ${SOURCE_FILES})
include_directories(
        )

target_link_libraries(task1
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${GLEW_LIBRARY})