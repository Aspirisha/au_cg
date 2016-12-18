#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"

class SimpleRenderer : public Renderer {
    SimpleRenderer(GLFWwindow* window);
public:
    static SimpleRenderer *getRenderer(GLFWwindow* window);

    void onMouseWheel(GLFWwindow *window, double xoffset, double yoffset) {};
    void onWindowSizeChanged(GLFWwindow* window, int width, int height);
    void onMouseButton(GLFWwindow *window, int button, int action, int mods);
    void onMousePos(GLFWwindow *window, double x, double y);
    void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);
    void render();
    ~SimpleRenderer();
private:
    void update_view();

    float cam_speed = 0.05f;
    float cam_heading_speed = 0.05f; // degrees
    float model_rotation_speed = 1.0f;
    float model_scale = 5;
    float cube_scale = 30;
    // this are camera matrices evaluated when something changes
    // in fact we store them just not to make full recalculation each time anything changes
    glm::mat4 T;
    glm::mat4 R;

    glm::mat4 view_mat;
    glm::mat4 proj_mat;
    glm::mat4 model_mat;
    glm::vec3 cam_pos = {0.0f, 0.0f, 5.0f};
    glm::vec3 cam_lookAt = {0, 0, -1};
    glm::vec3 cam_up = {0, 1, 0};
    glm::vec3 cam_right = {1, 0, 0}; // even though we can calculate it as cross product, what for
    glm::mat4 cube_mat;

    GLFWwindow* window;
    GLuint cubeProgramID;
    GLuint modelProgramID;
    GLint model_V;
    GLint model_M;
    GLint model_P;
    GLuint cubeTexture;

    int cubePointCount;
    int modelPointCount;

    GLint cube_P;
    GLint cube_V;

    GLuint modelVertexArray;
    GLuint cubeVertexArray;

    double lastMousePosX = -1;
    double lastMousePosY = -1;

    // window size (inner)
    int width = 0;
    int height = 0;
};
