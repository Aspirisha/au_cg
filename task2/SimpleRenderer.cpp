#include <GL/glew.h>
#include <iostream>
#include <unordered_map>
#include "glm/ext.hpp"
#include "SimpleRenderer.h"
#include "ShaderLoader.h"
#include "obj_parser.h"
#include "stb_image/stb_image.h"

using namespace std;

namespace
{
GLuint load_obj_into_array(const char *fileName, int &pointCount) {
    std::shared_ptr<GLfloat> vp; // array of vertex points
    std::shared_ptr<GLfloat> vn; // array of vertex normals
    std::shared_ptr<GLfloat> vt; // array of texture coordinates
    assert (load_obj_file(fileName, vp, vt, vn, pointCount));

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint points_vbo, normals_vbo;
    if (NULL != vp) {
        glGenBuffers(1, &points_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
        glBufferData(
                GL_ARRAY_BUFFER, 3 * pointCount * sizeof(GLfloat), vp.get(),
                GL_STATIC_DRAW
        );
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
    }
    if (NULL != vn) {
        glGenBuffers(1, &normals_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
        glBufferData(
                GL_ARRAY_BUFFER, 3 * pointCount * sizeof(GLfloat), vn.get(),
                GL_STATIC_DRAW
        );
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
    }
    return vao;
}


/* use stb_image to load an image file into memory, and then into one side of
a cube-map texture. */
bool load_cube_map_side(GLuint texture, GLenum side_target,
                        const string &file_name) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int x, y, n;
    int force_channels = 4;
    unsigned char *image_data = stbi_load(file_name.c_str(), &x, &y, &n,
                                          force_channels);
    if (!image_data) {
        fprintf(stderr, "ERROR: could not load %s\n", file_name);
        return false;
    }
    // non-power-of-2 dimensions check
    if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
        fprintf(
                stderr, "WARNING: image %s is not power-of-2 dimensions\n",
                file_name
        );
    }

    // copy image data into 'target' side of cube map
    glTexImage2D(
            side_target,
            0,
            GL_RGBA,
            x,
            y,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image_data
    );
    free(image_data);
    return true;
}

/* load all 6 sides of the cube-map from images, then apply formatting to the
final texture */
void create_cube_map(GLuint *tex_cube) {
    const pair<string, GLenum> cube_sides[] = {
            {"cube_textures/back.bmp", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
            {"cube_textures/front.bmp", GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
            {"cube_textures/top.bmp", GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
            {"cube_textures/bottom.bmp", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
            {"cube_textures/left.bmp", GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
            {"cube_textures/right.bmp", GL_TEXTURE_CUBE_MAP_POSITIVE_X}
    };
    // generate a cube-map texture to hold all the sides
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, tex_cube);

    // load each image and copy into a side of the cube-map texture
    for (const auto &cs: cube_sides) {
        load_cube_map_side(*tex_cube, cs.second, cs.first);
    }
    // format cube map texture
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void _update_fps_counter (GLFWwindow* window) {
    static double previous_seconds = glfwGetTime ();
    static int frame_count;
    double current_seconds = glfwGetTime ();
    double elapsed_seconds = current_seconds - previous_seconds;
    if (elapsed_seconds > 0.25) {
        previous_seconds = current_seconds;
        double fps = (double)frame_count / elapsed_seconds;
        char tmp[128];
        sprintf (tmp, "opengl @ fps: %.2f", fps);
        glfwSetWindowTitle (window, tmp);
        frame_count = 0;
    }
    frame_count++;
}

}

SimpleRenderer::SimpleRenderer(GLFWwindow* window) : window(window) {
    const char* MONKEY_VERT_FILE = "shaders/reflect_vs.glsl";
    const char* MONKEY_FRAG_FILE = "shaders/reflect_fs.glsl";
    const char* CUBE_VERT_FILE = "shaders/cube_vs.glsl";
    const char* CUBE_FRAG_FILE = "shaders/cube_fs.glsl";
    const char* MODEL_FILE = "models/bunny_with_normals.obj";
    const char* CUBE_FILE = "models/cube.obj";


    cubeProgramID = LoadShaders(CUBE_VERT_FILE, CUBE_FRAG_FILE);
    cube_V = glGetUniformLocation(cubeProgramID, "V");
    cube_P = glGetUniformLocation(cubeProgramID, "P");

    modelProgramID = LoadShaders(MONKEY_VERT_FILE, MONKEY_FRAG_FILE);
    model_M = glGetUniformLocation(modelProgramID, "M");
    model_V = glGetUniformLocation(modelProgramID, "V");
    model_P = glGetUniformLocation(modelProgramID, "P");

    create_cube_map(&cubeTexture);

    // Load objs
    modelVertexArray = load_obj_into_array(MODEL_FILE, modelPointCount);
    cubeVertexArray = load_obj_into_array(CUBE_FILE, cubePointCount);

    cube_mat = glm::scale(glm::mat4{}, glm::vec3(cube_scale));

    model_mat = glm::scale(glm::mat4{}, glm::vec3(model_scale));
    glfwGetFramebufferSize(window, &width, &height);
    proj_mat = glm::perspective(glm::radians(60.0f), (float) width / (float)height, 0.1f, 100.0f);

    T = glm::translate(glm::mat4(), cam_pos);
    update_view();

    glUseProgram (modelProgramID);
    glUniformMatrix4fv (model_P, 1, GL_FALSE, &proj_mat[0][0]);
    glUseProgram (cubeProgramID);
    glUniformMatrix4fv (cube_P, 1, GL_FALSE, &proj_mat[0][0]);

    glEnable (GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable (GL_CULL_FACE); // cull face
    glCullFace (GL_BACK); // cull back face
    glFrontFace (GL_CCW); // set counter-clock-wise vertex order to mean the front
    glClearColor (0.2, 0.2, 0.2, 1.0); // grey background
    glViewport (0, 0, width, height);
}


SimpleRenderer *SimpleRenderer::getRenderer(GLFWwindow* window) {
    renderers().push_back(unique_ptr<Renderer>(
            (Renderer*)new SimpleRenderer(window)));
    return dynamic_cast<SimpleRenderer*>(renderers().back().get());
}

void SimpleRenderer::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _update_fps_counter (window);

    // wipe the drawing surface clear
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render a sky-box using the cube-map texture
    glDepthMask (GL_FALSE);
    glUseProgram (cubeProgramID);
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_CUBE_MAP, cubeTexture);
    glBindVertexArray (cubeVertexArray);
    glDrawArrays (GL_TRIANGLES, 0, cubePointCount);
    glDepthMask (GL_TRUE);

    glUseProgram (modelProgramID);
    glBindVertexArray (modelVertexArray);
    glUniformMatrix4fv (model_M, 1, GL_FALSE, &model_mat[0][0]);
    glDrawArrays (GL_TRIANGLES, 0, modelPointCount);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void SimpleRenderer::onMouseButton(GLFWwindow *window, int button, int action, int mods) { }

void SimpleRenderer::onMousePos(GLFWwindow *window, double x, double y) {

    float dx = (float) (x - lastMousePosX);
    float dy = (float) (y - lastMousePosY);

    auto on_rotate = [this]() {
        cam_lookAt = glm::vec3(R * glm::vec4 (0.0, 0.0, -1.0, 0.0));
        cam_right = glm::vec3(R * glm::vec4 (1.0, 0.0, 0.0, 0.0));
        cam_up = glm::vec3(R * glm::vec4 (0.0, 1.0, 0.0, 0.0));
    };

    R = glm::rotate(glm::mat4{}, -dx * glm::radians(cam_heading_speed), glm::vec3 (0.0, 1.0, 0.0)) * R;
    on_rotate();
    R = glm::rotate(glm::mat4{}, dy * glm::radians(cam_heading_speed), cam_right) * R;
    on_rotate();

    update_view();
    lastMousePosY = y;
    lastMousePosX = x;
}

void SimpleRenderer::onWindowSizeChanged(GLFWwindow *window, int width,
                                         int height) {
    glViewport(0, 0, width, height);
    this->width = width;
    this->height = height;
}

void SimpleRenderer::onKeyEvent(GLFWwindow *window, int key, int scancode,
                                int action, int mods) {
    static glm::vec3 fwd_init (0.0f, 0.0f, -1.0f);
    static glm::vec3 rgt_init (1.0f, 0.0f, 0.0f);
    static glm::vec3 up_init (0.0f, 1.0f, 0.0f);
    static unordered_map<int, int> direction_sign = {
            {GLFW_KEY_LEFT, 1}, {GLFW_KEY_RIGHT, -1},
            {GLFW_KEY_UP, 1}, {GLFW_KEY_DOWN, -1},
            {GLFW_KEY_W, -1}, {GLFW_KEY_S, 1},
            {GLFW_KEY_D, 1}, {GLFW_KEY_A, -1},
    };


    if (action == GLFW_RELEASE)
        return;
    glm::vec3 move(0,0,0);

    switch (key) {
        case GLFW_KEY_A:
        case GLFW_KEY_D:
            move.x += direction_sign.at(key) * cam_speed;
            break;
        case GLFW_KEY_S:
        case GLFW_KEY_W:
            move.z += direction_sign.at(key) * cam_speed;
            break;
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_LEFT: {
            model_mat = glm::rotate(glm::mat4{}, direction_sign.at(key) * glm::radians(model_rotation_speed), up_init) * model_mat;
            break;
        }
        case GLFW_KEY_DOWN:
        case GLFW_KEY_UP: {
            model_mat = glm::rotate(glm::mat4{}, direction_sign.at(key) * glm::radians(model_rotation_speed), rgt_init) * model_mat;
            break;
        }
        default:
            return;
    }
    cam_pos = cam_pos + cam_lookAt * -move.z;
    cam_pos = cam_pos + cam_up * move.y;
    cam_pos = cam_pos + cam_right * move.x;
    T = translate (glm::mat4(), cam_pos);

    update_view();
}

void SimpleRenderer::update_view() {
    view_mat = inverse (R) * inverse (T);
    glUseProgram (modelProgramID);
    glUniformMatrix4fv (model_V, 1, GL_FALSE, &view_mat[0][0]);

    // cube-map view matrix has rotation, but not translation
    glUseProgram (cubeProgramID);
    auto RReversed = inverse(R) *inverse (T)  * cube_mat;
    glUniformMatrix4fv (cube_V, 1, GL_FALSE, &RReversed[0][0]);
}

SimpleRenderer::~SimpleRenderer() {
    glDeleteTextures(1, &cubeTexture);
}



