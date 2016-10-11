#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"

class SimpleRenderer : public Renderer {
    SimpleRenderer(GLFWwindow* window);

public:
    static SimpleRenderer *getRenderer(GLFWwindow* window);

    virtual void onMouseWheel(GLFWwindow *window, double xoffset, double yoffset);
    virtual void onWindowSizeChanged(GLFWwindow* window, int width, int height);
    virtual void onMouseButton(GLFWwindow *window, int button, int action, int mods);
    virtual void onMousePos(GLFWwindow *window, double x, double y);
    virtual void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
    virtual void render();

    void initVertices();
    void initIndices();
    void initTexture(bool reset = false);
private:
    float mouseSpeed = 0.02;
    float scale = 1;
    glm::vec2 translate = glm::vec2(0,0);

    GLFWwindow* window; // (In the accompanying source code, this variable is global)
    GLuint programID;
    GLint locationMatrixID;
    GLuint uiVertexBuffer; // Here are stored heightmap data (vertices)
    GLuint uiIndexBuffer; // And here indices for rendering heightmap
    GLuint uiVertexArray; // One VAO for heightmap
    GLint scaleParamID;
    GLint maxIterationsID;
    GLint imageSpaceWidthHeightID;
    GLint imageSpaceTranslateID;
    GLuint textureID;

    const float imageSpaceWidth = 4; // complex plain -2 to 2
    const float imageSpaceHeight = 4; // --/--

    double lastMousePosX = -1;
    double lastMousePosY = -1;

    bool isMousePressed = false;

    // window size (inner)
    int width = 0;
    int height = 0;

    int maxIterationsNumber = 100;
    enum {
        TRIANGLES_NUMBER_X = 1000,
        TRIANGLES_NUMBER_Y = 1000,
        VERTEX_NUMBER = (TRIANGLES_NUMBER_Y + 1)*(TRIANGLES_NUMBER_X + 1)
    };
    glm::vec3 vertex [VERTEX_NUMBER];
    int index[2 * VERTEX_NUMBER + TRIANGLES_NUMBER_Y];
};
