/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 7 Nov 2013                                                     |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries' separate legal notices                             |
|******************************************************************************|
| Anton's lazy Wavefront OBJ parser                                            |
| Anton Gerdelan 7 Nov 2013                                                    |
| Notes:                                                                       |
| I ignore MTL files                                                           |
| Mesh MUST be triangulated - quads not accepted                               |
| Mesh MUST contain vertex points, normals, and texture coordinates            |
| Faces MUST come after all other data in the .obj file                        |
\******************************************************************************/
#ifndef _OBJ_PARSER_H_
#define _OBJ_PARSER_H_

#include <memory>

bool load_obj_file (
	const char* file_name,
	std::shared_ptr<float>& points,
	std::shared_ptr<float>& tex_coords,
	std::shared_ptr<float>& normals,
	int& point_count
);

#endif
