#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"

class SimpleRenderer : public Renderer {
    SimpleRenderer(GLFWwindow* window, GLuint programID, GLint locationMatrixID);

public:
    static SimpleRenderer *getRenderer(GLFWwindow* window, GLuint programID,
        GLuint locationMatrixID);

    virtual void onMouseWheel(GLFWwindow *window, double xoffset, double yoffset);
    virtual void onWindowSizeChanged(GLFWwindow* window, int width, int height);
    virtual void render();

    void initVertices();
    void initIndices();

    void setVertexBuffer(GLuint vb) {
        vertexbuffer = vb;
    }
private:
    float mouseSpeed = 0.1;
    float scale = 1;
    glm::vec3 translate = glm::vec3(0,0,0);

    GLFWwindow* window; // (In the accompanying source code, this variable is global)
    GLuint programID;
    GLint locationMatrixID;
    GLuint vertexbuffer;
    int width = 0;
    int height = 0;

    enum {
        TRIANGLES_NUMBER_X = 3,
        TRIANGLES_NUMBER_Y = 3,
        VERTEX_NUMBER = (TRIANGLES_NUMBER_Y + 1)*(TRIANGLES_NUMBER_X + 1)
    };
    glm::vec3 vertex [VERTEX_NUMBER];
    int index[2 * VERTEX_NUMBER + TRIANGLES_NUMBER_Y];
};
