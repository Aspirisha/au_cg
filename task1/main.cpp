#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <memory>

#include "Renderer.h"
#include "SimpleRenderer.h"

using namespace std;

GLFWwindow *initWindow() {
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return nullptr;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

    // Open a window and create its OpenGL context
    GLFWwindow* window; // (In the accompanying source code, this variable is global)
    const int width = 1024;
    const int height = 768;

    window = glfwCreateWindow(width, height, "Fractal", NULL, NULL);
    if( window == NULL ){
        cerr << "Failed to open GLFW window. If you have an Intel GPU, they "
                "are not 3.3 compatible. Try the 2.1 version of the tutorials.\n";
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW

    glewExperimental=true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        cerr << "Failed to initialize GLEW\n";
        return nullptr;
    }

    return window;
}

int main(int argc, char** argv)
{
    GLFWwindow *window = initWindow();

    glfwSetScrollCallback(window, Renderer::dispatchMouseWheel);
    glfwSetFramebufferSizeCallback(window, Renderer::framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, Renderer::dispatchMouseButton);
    glfwSetCursorPosCallback(window, Renderer::dispatchMousePos);
    glfwSetKeyCallback(window, Renderer::dispatchKeyEvent);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    SimpleRenderer *renderer = SimpleRenderer::getRenderer(window);

    do {
        renderer->render();
    } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    Renderer::closeRenderer(renderer);
    glfwDestroyWindow(window);
    glfwTerminate();
}