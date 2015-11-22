#version 420 core

// Output
layout (location = 0) out vec4 color;
// note: default layout(location = 0) is piped to the back buffer
//       so we can just do out vec4 color;

// Input from vertex shader
in VS_OUT
{
    vec4 color;
} fs_in;

void main(void)
{
    // Write final color to the framebuffer
    color = fs_in.color;
}
