#include "obj_parser.h"
#include "ShaderLoader.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "PointLight.h"
#include "gbuffer.h"

using namespace glm;
using namespace std;
const char * FIRST_PASS_VS = "shaders/first_pass_vs.glsl";
const char * FIRST_PASS_FS = "shaders/first_pass_fs.glsl";
const char * SECOND_PASS_VS =  "shaders/second_pass_vs.glsl";
const char * SECOND_PASS_FS = "shaders/second_pass_fs.glsl";
const char * BULB_VS = "shaders/bulb_vs.glsl";
const char * BULB_FS = "shaders/bulb_fs.glsl";
const char * SPHERE_FILE = "models/sphere.obj";
const char * PLANE_FILE = "models/plane.obj";
const char * BUNNY_FILE = "models/bunny_with_normals.obj";
const size_t NUM_LIGHTS = 30;

int g_gl_width = 800;
int g_gl_height = 800;
GLFWwindow* g_window;


GLuint g_sphere_vao; /* 3d sphere representing light coverage area and corresponfing bulb*/
int sphere_point_count;
std::vector<PointLight> lights;

GLuint g_plane_vao;
int plane_point_count;
mat4 plane_model_mat;

GLuint g_bunny_vao;
int g_bunny_point_count;
mat4 g_bunny_M;

GLuint geometry_pass_program;
GLint gpass_P_loc;
GLint gpass_V_loc;
GLint gpass_M_loc;

GLuint light_pass_program;
GLint lpass_P_loc = -1;
GLint lpass_V_loc = -1;
GLint lpass_M_loc = -1;
GLint lpass_L_p_loc = -1;
GLint lpass_L_d_loc = -1;
GLint lpass_L_s_loc = -1;
GLint lpass_p_tex_loc = -1;
GLint lpass_n_tex_loc = -1;

GLuint bulbs_program;
GLint bulb_P_loc = -1;
GLint bulb_V_loc = -1;
GLint bulb_M_loc = -1;
GLint bulb_color_loc = -1;

GBuffer gb;

mat4 projection_mat;

namespace controller
{
double last_mouse_y = -1;
double last_mouse_x = -1;
mat4 view_mat;
mat4 view_rotation;
mat4 view_translation = glm::translate({}, vec3{0, 30, 30});
glm::vec3 cam_lookAt(0, 0, 0);
glm::vec3 cam_right(1, 0, 0);
glm::vec3 cam_up(0, 1, 0);


void update_view() {
	view_mat = inverse (view_rotation) * inverse (view_translation);
}


void onMousePos(GLFWwindow *window, double x, double y) {
	const float cam_heading_speed = 0.05f;

	float dx = (float) (x - last_mouse_x);
	float dy = (float) (y - last_mouse_y);

	auto on_rotate = []() {
		cam_lookAt = glm::vec3(view_rotation * glm::vec4 (0.0, 0.0, -1.0, 0.0));
		cam_right = glm::vec3(view_rotation * glm::vec4 (1.0, 0.0, 0.0, 0.0));
		cam_up = glm::vec3(view_rotation * glm::vec4 (0.0, 1.0, 0.0, 0.0));
	};

	view_rotation = glm::rotate(glm::mat4{}, -dx * glm::radians(cam_heading_speed), glm::vec3 (0.0, 1.0, 0.0)) * view_rotation;
	on_rotate();
	view_rotation = glm::rotate(glm::mat4{}, dy * glm::radians(cam_heading_speed), cam_right) * view_rotation;
	on_rotate();

	update_view();
	last_mouse_y = y;
	last_mouse_x = x;
}

void onKeyEvent(GLFWwindow *window, int key, int scancode,
				int action, int mods) {
	const float cam_speed = 0.5;
	static glm::vec3 fwd_init (0.0f, 0.0f, -1.0f);
	static glm::vec3 rgt_init (1.0f, 0.0f, 0.0f);
	static glm::vec3 up_init (0.0f, 1.0f, 0.0f);
	static unordered_map<int, int> direction_sign = {
			{GLFW_KEY_LEFT, 1}, {GLFW_KEY_RIGHT, -1},
			{GLFW_KEY_UP, 1}, {GLFW_KEY_DOWN, -1},
			{GLFW_KEY_W, -1}, {GLFW_KEY_S, 1},
			{GLFW_KEY_D, 1}, {GLFW_KEY_A, -1},
	};
	static glm::vec3 cam_pos(view_translation[3]);

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
			//model_mat = glm::rotate(glm::mat4{}, direction_sign.at(key) * glm::radians(model_rotation_speed), up_init) * model_mat;
			break;
		}
		case GLFW_KEY_DOWN:
		case GLFW_KEY_UP: {
			//model_mat = glm::rotate(glm::mat4{}, direction_sign.at(key) * glm::radians(model_rotation_speed), rgt_init) * model_mat;
			break;
		}
		default:
			return;
	}
	cam_pos = cam_pos + cam_lookAt * -move.z;
	cam_pos = cam_pos + cam_up * move.y;
	cam_pos = cam_pos + cam_right * move.x;
	view_translation = translate ({}, cam_pos);

	update_view();
}
}

GLuint load_obj_into_array(const char *fileName, int &pointCount) {
	GLfloat *vp = nullptr; // array of vertex points
	GLfloat *vn = nullptr; // array of vertex normals
	GLfloat *vt = nullptr; // array of texture coordinates
	assert (load_obj_file(fileName, vp, vt, vn, pointCount));

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint points_vbo, normals_vbo;
	if (NULL != vp) {
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(
				GL_ARRAY_BUFFER, 3 * pointCount * sizeof(GLfloat), vp,
				GL_STATIC_DRAW
		);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
	}
	if (NULL != vn) {
		glGenBuffers(1, &normals_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
		glBufferData(
				GL_ARRAY_BUFFER, 3 * pointCount * sizeof(GLfloat), vn,
				GL_STATIC_DRAW
		);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(1);
	}

	delete[] vp;
	delete[] vn;
	delete[] vt;
	return vao;
}

/* the first pass draws
* pixel positions
* pixel normals
* pixel depths
to 3 attached textures. */
void draw_first_pass () {
	gb.BindForWriting();
	glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable (GL_BLEND);
	glEnable (GL_DEPTH_TEST);
	glDepthMask (GL_TRUE);

	glUseProgram (geometry_pass_program);
	glBindVertexArray (g_plane_vao);

	/* virtual camera matrices */
	glUniformMatrix4fv (gpass_P_loc, 1, GL_FALSE, &projection_mat[0][0]);
	glUniformMatrix4fv (gpass_V_loc, 1, GL_FALSE, &controller::view_mat[0][0]);

	glUniformMatrix4fv (gpass_M_loc, 1, GL_FALSE, &plane_model_mat[0][0]);
	glDrawArrays (GL_TRIANGLES, 0, plane_point_count);

	glBindVertexArray (g_bunny_vao);
	glUniformMatrix4fv (gpass_M_loc, 1, GL_FALSE, &g_bunny_M[0][0]);
	glDrawArrays (GL_TRIANGLES, 0, g_bunny_point_count);
}

/* the second pass
* retrieves pixel positions, normals, and depths
*/
void draw_second_pass () {
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	/* clear to any colour */
	glClearColor (0.2, 0.2, 0.2, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT);

	glEnable (GL_BLEND); // --- could reject background frags!
	glBlendEquation (GL_FUNC_ADD);
	glBlendFunc (GL_ONE, GL_ONE); // addition each time
	glDisable (GL_DEPTH_TEST);
	glDepthMask (GL_FALSE);

	gb.BindForReading();

	glUseProgram (light_pass_program);
	glBindVertexArray (g_sphere_vao);

	/* virtual camera matrices */
	glUniformMatrix4fv (lpass_P_loc, 1, GL_FALSE, &projection_mat[0][0]);
	glUniformMatrix4fv (lpass_V_loc, 1, GL_FALSE, &controller::view_mat[0][0]);
	glFrontFace(GL_CW);
	for (auto &l : lights) {
		/* world position */
		glUniform3f (lpass_L_p_loc, l.pos.x, l.pos.y, l.pos.z);
		/* diffuse colour */
		glUniform3f (lpass_L_d_loc, l.color_diffuse.x, l.color_diffuse.y, l.color_diffuse.z);
		/* specular colour */
		glUniform3f (lpass_L_s_loc, l.color_specular.x, l.color_specular.y, l.color_specular.z);

		glUniformMatrix4fv (lpass_M_loc, 1, GL_FALSE, &l.light_M[0][0]);
		glDrawArrays (GL_TRIANGLES, 0, sphere_point_count);
	}

	glFrontFace(GL_CCW);
	glUseProgram(bulbs_program);
	glUniformMatrix4fv (bulb_P_loc, 1, GL_FALSE, &projection_mat[0][0]);
	glUniformMatrix4fv (bulb_V_loc, 1, GL_FALSE, &controller::view_mat[0][0]);
	for (auto &l : lights) {
		glUniformMatrix4fv (bulb_M_loc, 1, GL_FALSE, &l.bulbM[0][0]);
		glUniform3fv (bulb_color_loc, 1, &l.color_diffuse[0]);
		glDrawArrays (GL_TRIANGLES, 0, sphere_point_count);
	}
	glBindVertexArray (g_bunny_vao);
	glUniformMatrix4fv (bulb_M_loc, 1, GL_FALSE, &g_bunny_M[0][0]);
//	glDrawArrays (GL_TRIANGLES, 0, g_bunny_point_count);
}
GLFWwindow *initWindow() {
	if( !glfwInit() ) {
		cerr << "Failed to initialize GLFW\n";
		return nullptr;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

	GLFWwindow* window;
	const int width = 800;
	const int height = 800;

	window = glfwCreateWindow(width, height, "Bunny", NULL, NULL);
	if(window == NULL) {
		std::cerr << "Failed to open GLFW window. If you have an Intel GPU, they "
							 "are not 3.3 compatible. Try the 2.1 version of the tutorials.\n";
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window); // Initialize GLEW
	glfwWindowHint (GLFW_SAMPLES, 4);
	glewExperimental=true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW\n";
		return nullptr;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return window;
}

void update() {
	static double previousUpdateTime = glfwGetTime();
	const double update_freq = 1 / 60.0;

	if (glfwGetTime() - previousUpdateTime > update_freq) {
		for (auto& l: lights) {
			l.move();
		}
		previousUpdateTime = glfwGetTime();
	}
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



int main () {
	/* initialise GL context and window */
	g_window = initWindow();
	gb.Init(g_gl_width, g_gl_height);
	glfwSetCursorPosCallback(g_window, controller::onMousePos);
	glfwSetKeyCallback(g_window, controller::onKeyEvent);
	/* initialise framebuffer and G-buffer */

	/* object positions and matrices */
	plane_model_mat = translate ({}, vec3 (0.0f, -2.0f, 0.0f));
	plane_model_mat = scale (plane_model_mat, vec3 (200.0f, 1.0f, 200.0f));

	g_bunny_M = translate ({}, vec3 (0.0f, 1.0f, 0.0f));
	g_bunny_M = scale(g_bunny_M, vec3 (30.0f, 30.0f, 30.0f));


	/* load pre-pass shaders that write to the g-buffer */
	geometry_pass_program = LoadShaders (FIRST_PASS_VS, FIRST_PASS_FS);
	gpass_P_loc = glGetUniformLocation (geometry_pass_program, "P");
	gpass_V_loc = glGetUniformLocation (geometry_pass_program, "V");
	gpass_M_loc = glGetUniformLocation (geometry_pass_program, "M");
	/* load screen-space pass shaders that read from the g-buffer */
	light_pass_program = LoadShaders (SECOND_PASS_VS, SECOND_PASS_FS);
	lpass_P_loc = glGetUniformLocation (light_pass_program, "P");
	lpass_V_loc = glGetUniformLocation (light_pass_program, "V");
	lpass_M_loc = glGetUniformLocation (light_pass_program, "M");
	lpass_L_p_loc = glGetUniformLocation (light_pass_program, "lp");
	lpass_L_d_loc = glGetUniformLocation (light_pass_program, "ld");
	lpass_L_s_loc = glGetUniformLocation (light_pass_program, "ls");
	lpass_p_tex_loc = glGetUniformLocation (light_pass_program, "p_tex");
	lpass_n_tex_loc = glGetUniformLocation (light_pass_program, "n_tex");

	bulbs_program = LoadShaders (BULB_VS, BULB_FS);
	bulb_P_loc = glGetUniformLocation (bulbs_program, "P");
	bulb_V_loc = glGetUniformLocation (bulbs_program, "V");
	bulb_M_loc = glGetUniformLocation (bulbs_program, "M");
	bulb_color_loc = glGetUniformLocation (bulbs_program, "color");

	glUseProgram (light_pass_program);
	glUniform1i (lpass_p_tex_loc, 0);
	glUniform1i (lpass_n_tex_loc, 1);


	g_sphere_vao = load_obj_into_array(SPHERE_FILE, sphere_point_count);
	g_plane_vao = load_obj_into_array(PLANE_FILE, plane_point_count);
	g_bunny_vao = load_obj_into_array(BUNNY_FILE, g_bunny_point_count);

	/* light positions and matrices */
	const size_t amplitude = 40;
	for (size_t i = 0; i < NUM_LIGHTS; i++) {
		lights.push_back({vec3(rand() % amplitude - amplitude / 2.0f,
							   5.0f, rand() % amplitude - amplitude / 2.0f)});
	}

	float aspect = (float)g_gl_width / (float)g_gl_height;
	float near = 0.1f;
	float far = 1000.0f;
	float fovy = glm::radians(67.0f);
	projection_mat = perspective (fovy, aspect, near, far);
	controller::update_view();

	glViewport (0, 0, g_gl_width, g_gl_height);
	glEnable (GL_CULL_FACE); // cull face
	glCullFace (GL_BACK); // cull back face
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	while (!glfwWindowShouldClose (g_window)) {
		_update_fps_counter (g_window);
		draw_first_pass ();
		draw_second_pass ();
		
		glfwSwapBuffers (g_window);
		glfwPollEvents ();
		if (GLFW_PRESS == glfwGetKey (g_window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose (g_window, 1);
		}
		update();
	}
	
	/* close GL context and any other GLFW resources */
	glfwTerminate();
	return 0;
}
