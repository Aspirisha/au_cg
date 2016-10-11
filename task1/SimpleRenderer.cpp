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
    maxIterationsID = glGetUniformLocation(programID, "MAX_ITERS");

    glfwGetFramebufferSize(window, &width, &height);
    initVertices();
    initIndices();
    initTexture();

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

    glUseProgram(programID);
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

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) width / (float)height, 0.1f, 5.0f);


    glm::mat4 MVPmatrix = projection * view; // Remember : inverted !

    glUniformMatrix4fv(locationMatrixID, 1, GL_FALSE, &MVPmatrix[0][0]);
    glUniform1f(scaleParamID, scale);

    glUniform2f(imageSpaceWidthHeightID, imageSpaceWidth, imageSpaceHeight);
    glUniform2f(imageSpaceTranslateID, translate.x, translate.y);
    glUniform1i(maxIterationsID, maxIterationsNumber);

    // Draw the triangle !
    glDrawElements(GL_TRIANGLE_STRIP, (TRIANGLES_NUMBER_X+1)*TRIANGLES_NUMBER_Y*2+TRIANGLES_NUMBER_Y-1, GL_UNSIGNED_INT, 0);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void SimpleRenderer::onMouseWheel(GLFWwindow *window, double, double yoffset) {
    scale *= pow(1.1, -yoffset);
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

    translate.x += dx * scale * imageSpaceWidth / width;
    translate.y += dy * scale * imageSpaceHeight / height;

    lastMousePosX = x;
    lastMousePosY = y;
}

void SimpleRenderer::onWindowSizeChanged(GLFWwindow *window, int width,
                                         int height) {
    glViewport(0, 0, width, height);
    this->width = width;
    this->height = height;
}

void SimpleRenderer::initTexture(bool reset) {
    const int COLORS_NUM = maxIterationsNumber;
    const int COMPONENTS = 3;
    GLfloat colors[COMPONENTS * (COLORS_NUM + 1)];
    float dc = 1.0f / (COLORS_NUM + 1);
    for (int i = 0, offset = 0; i < COLORS_NUM; i++) {
        colors[offset++] = dc * i;
        colors[offset++] = 1 - dc * i;
        colors[offset++] = 0;
    }
    int idx = COMPONENTS * COLORS_NUM;
    colors[idx] = colors[idx+1] = colors[idx+2] = 0;


    GLuint colorBuffer;
    glGenBuffers   ( 1, &colorBuffer );
    glBindBuffer   ( GL_TEXTURE_BUFFER, colorBuffer );
    glBufferData   ( GL_TEXTURE_BUFFER, sizeof(colors), 0, GL_STATIC_DRAW );  // Alloc
    glBufferSubData( GL_TEXTURE_BUFFER, 0, sizeof(colors), colors );              // Fill

    if (reset) {
        glDeleteTextures(1, &textureID);
    }
    glGenTextures(1, &textureID);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_BUFFER, textureID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, colorBuffer);
    glUniform1i( glGetUniformLocation( programID, "TEX_COLORS" ), 0 );
}

void SimpleRenderer::onKeyEvent(GLFWwindow *window, int key, int scancode,
                                int action, int mods) {
    if (action == GLFW_RELEASE)
        return;

    switch (key) {
        case GLFW_KEY_EQUAL:
            maxIterationsNumber += 1;
            break;
        case GLFW_KEY_MINUS:
            if (maxIterationsNumber > 1)
                maxIterationsNumber -= 1;
            break;
        default:
            return;
    }
    initTexture(true);
    cout << "Using " << maxIterationsNumber << " iterations to test set belonging\n";
}






