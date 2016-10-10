cmake_minimum_required(VERSION 3.4)
project(task1)
INCLUDE(FindPkgConfig)

find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
include(ExternalProject)
ExternalProject_Add(
        glm
        PREFIX ${CMAKE_BINARY_DIR}/glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
)
ExternalProject_Get_Property(glm source_dir)
set(GlmIncludeDir ${source_dir}/glm)


pkg_search_module(GLFW REQUIRED glfw3)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${GlmIncludeDir}
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
        ${GLUT_LIBRARY}
        ${GLEW_LIBRARY})

add_custom_command(TARGET task1 PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_SOURCE_DIR}/build/shaders)

add_custom_command(TARGET task1 PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders)

add_custom_command(TARGET task1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/task1 ${CMAKE_SOURCE_DIR}/build/task1)
