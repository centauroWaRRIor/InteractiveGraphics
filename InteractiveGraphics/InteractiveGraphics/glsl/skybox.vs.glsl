#version 410 core

out VS_OUT
{
	vec3 tc; // texture coordinates
} vs_out;

uniform mat4 view_matrix;

void main(void)
{
	// skybox vertices are hardcoded to coincide with the 
	// clip space box that is arrived after the homogeneous 
	// space transformations. We are always looking at one
	// of these faces. Also, the skybox vertices coincide 
	// with the texture coordinates defined for an environment
	// cubemap.
    vec3[4] vertices = vec3[4](vec3(-1.0, -1.0, 1.0),
                               vec3( 1.0, -1.0, 1.0),
                               vec3(-1.0,  1.0, 1.0),
                               vec3( 1.0,  1.0, 1.0));
							   
	// the texture coordinates are obtained by using the upper
	// 3x3 matrix of the transfomation matrix (rotation) on the
	// skybox vertices. This way the rotation of the skybox is 
	// simulated.
    vs_out.tc = mat3(view_matrix) * vertices[gl_VertexID];

    gl_Position = vec4(vertices[gl_VertexID], 1.0);
	
	// note: gl_VertexID tells GLSL the offset of the current
	// vertex being processed. This is needed since we are not
	// supplying vertex attributes and are instead providing them
	// hardcoded in here.
}
