#version 410

in vec3 p_eye;
in vec2 tex_coord;

uniform sampler2D normal_map;
/* note that we should force the location number here by adding layout location
keywords */
layout (location = 0) out vec3 def_p;
layout (location = 1) out vec3 def_n;

void main () {
	def_p = p_eye;
	def_n = texture(normal_map, tex_coord).rgb;
}
