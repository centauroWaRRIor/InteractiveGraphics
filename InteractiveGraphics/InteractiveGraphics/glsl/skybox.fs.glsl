#version 410 core

uniform samplerCube tex_cubemap;

in VS_OUT
{
    vec3    tc;
} fs_in;

layout (location = 0) out vec4 color;

void main(void)
{
	// GLSL is smart enough to calculate the vector from the center
	// of the cubemap that maps to the provided texture coordinate 
	// in order to do the texture lookup here.
    color = texture(tex_cubemap, fs_in.tc);
}
