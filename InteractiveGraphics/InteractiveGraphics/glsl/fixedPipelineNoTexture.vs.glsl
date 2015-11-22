#version 420 core

// uniforms
// note: no need to use layout (location = 0) syntax
//       because I'm finding its position manually by
//       using glGetUniformLocation
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

// Per-vertex inputs
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out VS_OUT
{
    vec4 color;
} vs_out;

void main(void)
{
    // Calculate view-space coordinate
	vec4 posHomog = vec4(position, 1.0);
    vec4 P = mv_matrix * posHomog;
	
	vs_out.color = vec4(color, 1.0);

    // Calculate the clip-space position of each vertex
    gl_Position = proj_matrix * P;
}
