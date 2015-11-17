#version 120

// Output
//layout (location = 0) out vec4 color;

// Input from vertex shader
//in VS_OUT
//{
//    vec3 N;
//    vec3 L;
//    vec3 V;
//} fs_in;

void main(void)
{
    // Write final color to the framebuffer
    //color = vec4(1.0, 0.0, 0.0, 0.0);
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); 
}
