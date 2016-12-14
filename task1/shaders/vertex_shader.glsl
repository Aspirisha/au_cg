#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;


precision highp float; 

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec2 IMAGE_SPACE_WIDTH_HEIGHT;
uniform mat3 TRANSFORM;
uniform int MAX_ITERS;
uniform samplerBuffer TEX_COLORS;
varying vec3 vColor;


void main() {
   // Output position of the vertex, in clip space : MVP * position
   gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
   vColor = vertexPosition_modelspace;

   vec3 pos = TRANSFORM * vertexPosition_modelspace * vec3(IMAGE_SPACE_WIDTH_HEIGHT, 1) / 2;
   float zx = 0;
   float zy = 0;
   int i = 0;

   for(i = 0; i < MAX_ITERS; i++) {
        float newzx = pos.x + zx * zx - zy * zy;
        zy = pos.y + 2 * zy * zx;
        zx = newzx;

        if (zx * zx + zy * zy > 4) {
            break;
        }
   }

    vColor = texelFetch(TEX_COLORS, i).rgb;//vec3(float(MAX_ITERS - i) / float(MAX_ITERS), 0, 0);  

}