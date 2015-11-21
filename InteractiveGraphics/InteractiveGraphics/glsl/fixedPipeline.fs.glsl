#version 420 core

// Output
layout (location = 0) out vec4 color;
//default layout(location = 0) is piped to the back buffer
// so we can just do out vec4 color;

// Input from vertex shader
in VS_OUT
{
    //vec3 normal;
    //vec3 tangent;
    //vec3 bitangent;

    //vec3 ws_pos;
    vec4 color;
    //vec2 texcoord;
} fs_in;

void main(void)
{
    // Write final color to the framebuffer
    color = fs_in.color;
}
