#version 420 core

// uniforms
// note: using this syntax I avoid uploading the uniform manually
//       binding 0 corresponds to GL_TEXTURE0 unit 
//       binding 1 corresponds to GL_TEXTURE1 unit so on and so forth
//layout (binding = 0) uniform sampler2D tex_color;

uniform vec3 billboard[4];
uniform vec3 billboardColor[4];
uniform vec3 eyePosition;

// Output
layout (location = 0) out vec4 color;
// note: default layout(location = 0) is piped to the back buffer
//       so we can just do out vec4 color;

// Input from vertex shader
in VS_OUT
{
    vec4 color;
    vec2 texcoord;
	vec3 modelSpaceXYZ;
	vec3 normalDirection;
} fs_in;

void main(void)
{
    //vec3 viewDirection = fs_in.modelSpaceXYZ - eyePosition;
	//vec3 reflectDir = reflect(viewDirection, normalize(normalDirection)));

    // Write final color to the framebuffer
	color = vec4(normalize(normalDirection), 1.0);
    //color = fs_in.color;
	//color = texture(tex_color, fs_in.texcoord);
}
