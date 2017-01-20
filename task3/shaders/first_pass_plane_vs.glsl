#version 410

layout (location = 0) in vec3 vp;
layout (location = 2) in vec2 vt;
uniform mat4 P, V, M;

out vec3 p_eye;
out vec2 tex_coord;

void main () {
  p_eye = (V * M * vec4 (vp, 1.0)).xyz;
  tex_coord = vt;
  gl_Position = P * vec4 (p_eye, 1.0);
}
