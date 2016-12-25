cmake_minimum_required(VERSION 3.4)

project(task3)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

INCLUDE(FindPkgConfig)

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLM REQUIRED)


pkg_search_module(GLFW REQUIRED glfw3)

include_directories( ${OPENGL_INCLUDE_DIRS} ${GlmIncludeDir}
        ${GLFW_INCLUDE_DIRS} )
add_definitions(-Wall -Wextra -pedantic -O3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES main.cpp ShaderLoader.cpp obj_parser.cpp gbuffer.cpp)

add_executable(task3 ${SOURCE_FILES})
include_directories(${GLM_INCLUDE_DIRS})

target_link_libraries(task3
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${GLEW_LIBRARY})