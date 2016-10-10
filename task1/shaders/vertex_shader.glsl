#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec2 LEFT_BOTTOM_CORNER;
uniform float SCALE;

void main() {
   // Output position of the vertex, in clip space : MVP * position
   gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
}