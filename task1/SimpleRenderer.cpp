#include <GL/glew.h>
#include <iostream>
#include "SimpleRenderer.h"
#include "ShaderLoader.h"

using namespace std;

SimpleRenderer::SimpleRenderer(GLFWwindow* window) : window(window) {
    programID = LoadShaders("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    locationMatrixID = glGetUniformLocation(programID, "MVP");
    scaleParamID = glGetUniformLocation(programID, "SCALE");
    imageSpaceWidthHeightID = glGetUniformLocation(programID, "IMAGE_SPACE_WIDTH_HEIGHT");
    imageSpaceTranslateID = glGetUniformLocation(programID, "IMAGE_SPACE_TRANSLATE");

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
    for (int i = 0; i < TRIANGLES_NUMBER_Y; i++) {
        for (int j = 0; j < TRIANGLES_NUMBER_X + 1; j++) {
            *ptr++ = i * (TRIANGLES_NUMBER_X + 1) + j;
            *ptr++ = (i + 1) * (TRIANGLES_NUMBER_X + 1) + j;
        }

        *ptr++ = VERTEX_NUMBER;
    }

}

SimpleRenderer *SimpleRenderer::getRenderer(GLFWwindow* window) {
    renderers().push_back(unique_ptr<Renderer>(
            (Renderer*)new SimpleRenderer(window)));
    return dynamic_cast<SimpleRenderer*>(renderers().back().get());
}

void SimpleRenderer::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = glm::lookAt(
    glm::vec3(0,0,-2), // the position of your camera, in world space
    glm::vec3(0,0,0),   // where you want to look at, in world space
    glm::vec3(0,1,0)        // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
    );

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) width / (float)height, 0.1f, 100.0f);


    glm::mat4 MVPmatrix = projection * view; // Remember : inverted !

    glUniformMatrix4fv(locationMatrixID, 1, GL_FALSE, &MVPmatrix[0][0]);
    glUniform1f(scaleParamID, scale);

    glUniform2f(imageSpaceWidthHeightID, imageSpaceWidth, imageSpaceHeight);
    glUniform2f(imageSpaceTranslateID, translate.x, translate.y);

    glUseProgram(programID);
    // Draw the triangle !
    glDrawElements(GL_TRIANGLE_STRIP, (TRIANGLES_NUMBER_X+1)*TRIANGLES_NUMBER_Y*2+TRIANGLES_NUMBER_Y-1, GL_UNSIGNED_INT, 0);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void SimpleRenderer::onMouseWheel(GLFWwindow *window, double, double yoffset) {
    scale -= yoffset * mouseSpeed;
}

void SimpleRenderer::onMouseButton(GLFWwindow *window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    isMousePressed = action == GLFW_PRESS;
}

void SimpleRenderer::onMousePos(GLFWwindow *window, double x, double y) {
    if (!isMousePressed) {
        lastMousePosY = y;
        lastMousePosX = x;
        return;
    }

    double dx = x - lastMousePosX;
    double dy = y - lastMousePosY;

    translate.x -= dx * imageSpaceWidth / width;
    translate.y -= dy * imageSpaceHeight / height;

    lastMousePosX = x;
    lastMousePosY = y;
}

void SimpleRenderer::onWindowSizeChanged(GLFWwindow *window, int width,
                                         int height) {
    glViewport(0, 0, width, height);
    this->width = width;
    this->height = height;
}


