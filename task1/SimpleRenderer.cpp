#include <GL/glew.h>
#include <iostream>
#include <sqltypes.h>
#include "SimpleRenderer.h"

using namespace std;

UINT uiVertexBuffer; // Here are stored heightmap data (vertices)
UINT uiIndexBuffer; // And here indices for rendering heightmap

UINT uiVertexArray; // One VAO for heightmap

SimpleRenderer::SimpleRenderer(GLFWwindow* window, GLuint programID,
        GLint locationMatrixID) : window(window), programID(programID),
locationMatrixID(locationMatrixID) {
    glfwGetFramebufferSize(window, &width, &height);
    initVertices();
    initIndices();

    glGenVertexArrays(1, &uiVertexArray); // Create one VAO
    glGenBuffers(1, &uiVertexBuffer); // One VBO for data
    glGenBuffers(1, &uiIndexBuffer); // And finally one VBO for indices

    glBindVertexArray(uiVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, uiVertexBuffer); // make uiVertex an active buffer (this buffer we are talking now)

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*VERTEX_NUMBER, vertex, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiIndexBuffer);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(VERTEX_NUMBER);
}

void SimpleRenderer::initVertices() {
    float dx = 2.0f / TRIANGLES_NUMBER_X;
    float dy = 2.0f / TRIANGLES_NUMBER_Y;

    for (int i = 0, row = 0, col = 0; i < VERTEX_NUMBER; i++, col++) {
        if (col == TRIANGLES_NUMBER_Y + 1) {
            col = 0;
            row++;
        }

        vertex[i].x = -1 + dx * col;
        vertex[i].y = 1 - dy * row;
        vertex[i].z = 0;
    }
}

void SimpleRenderer::initIndices() {
    int *ptr = index;
    cout << "============================\n";
    for (int i = 0; i < TRIANGLES_NUMBER_Y; i++) {
        for (int j = 0; j < TRIANGLES_NUMBER_X + 1; j++) {
            *ptr++ = i * (TRIANGLES_NUMBER_X + 1) + j;
            cout << *(ptr-1) << " ";
            *ptr++ = (i + 1) * (TRIANGLES_NUMBER_X + 1) + j;
            cout << *(ptr-1) << " ";
        }

        *ptr++ = VERTEX_NUMBER;
        cout << *(ptr-1) << " \n";
    }

}

SimpleRenderer *SimpleRenderer::getRenderer(GLFWwindow* window, GLuint programID,
                                            GLuint locationMatrixID) {
    renderers().push_back(unique_ptr<Renderer>(
            (Renderer*)new SimpleRenderer(window, programID, locationMatrixID)));
    return dynamic_cast<SimpleRenderer*>(renderers().back().get());
}

void SimpleRenderer::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 myScaleMatrix = glm::scale(glm::mat4(1.0f),
                                           glm::vec3(scale, scale, scale));
    glm::mat4 myTranslationMatrix = glm::translate(glm::mat4(1.0f),
                                         translate);
    glm::mat4 model = myTranslationMatrix * myScaleMatrix;

    glm::mat4 view = glm::lookAt(
    glm::vec3(0,0,-2), // the position of your camera, in world space
    glm::vec3(0,0,0),   // where you want to look at, in world space
    glm::vec3(0,1,0)        // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
    );

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) width / (float)height, 0.1f, 100.0f);


    glm::mat4 MVPmatrix = projection * view * model; // Remember : inverted !

    glUniformMatrix4fv(locationMatrixID, 1, GL_FALSE, &MVPmatrix[0][0]);
    glUseProgram(programID);
    // Draw the triangle !
    glDrawElements(GL_TRIANGLE_STRIP, (TRIANGLES_NUMBER_X+1)*TRIANGLES_NUMBER_Y*2+TRIANGLES_NUMBER_Y-1, GL_UNSIGNED_INT, 0);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void SimpleRenderer::onMouseWheel(GLFWwindow *window, double, double yoffset) {
    scale += yoffset * mouseSpeed;
}

void SimpleRenderer::onWindowSizeChanged(GLFWwindow *window, int width,
                                         int height) {
    glViewport(0, 0, width, height);
    this->width = width;
    this->height = height;
}


