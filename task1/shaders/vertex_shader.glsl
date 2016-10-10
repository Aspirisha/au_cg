#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec2 IMAGE_SPACE_WIDTH_HEIGHT;
uniform vec2 IMAGE_SPACE_TRANSLATE;
uniform float SCALE;
varying vec3 vColor;
void main() {
   // Output position of the vertex, in clip space : MVP * position
   gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
   vColor = vertexPosition_modelspace;
   vec2 pos = (vec2(vertexPosition_modelspace.x, vertexPosition_modelspace.y) 
    * IMAGE_SPACE_WIDTH_HEIGHT / 2.0f) * SCALE  + IMAGE_SPACE_TRANSLATE;

   int MAX_ITERS = 200;
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

   if (i == MAX_ITERS) {
      vColor = vec3(0, 0, 0);
   } else {
      vColor = vec3(float(MAX_ITERS - i) / float(MAX_ITERS), 0, 0);  
   }

}