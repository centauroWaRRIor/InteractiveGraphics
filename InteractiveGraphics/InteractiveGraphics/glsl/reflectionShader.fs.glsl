#version 420 core

// uniforms
// note: using this syntax I avoid uploading the uniform manually
//       binding 0 corresponds to GL_TEXTURE0 unit 
//       binding 1 corresponds to GL_TEXTURE1 unit so on and so forth
//layout (binding = 0) uniform sampler2D tex_color;

uniform vec3 eyePosition;
uniform vec3 billboard[4];
uniform vec3 billboardColor[4];

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
    vec3 viewDirection = fs_in.modelSpaceXYZ - eyePosition;
	viewDirection = normalize(viewDirection);
	
	vec3 billboardNormal = cross(billboard[1] - billboard[0], billboard[3] - billboard[0]);
	
	// from "Mathematics for 3D Game Programming and Computer Graphics"
	vec3 N = normalize(billboardNormal);
	vec4 L = vec4(N, -1 * dot(N, billboard[0])); // 4D billboard plane vector
	
	// because the two inputs are normalized reflectDir is normalized
	vec3 reflectDir = reflect(viewDirection, normalize(fs_in.normalDirection));
	
	float LdotV = dot(L, vec4(reflectDir, 0.0));
	if(abs(LdotV) < 0.01) // no intersection occurs
		color = vec4(1.0, 0.0, 0.0, 1.0);
	else
		color = vec4(0.0, 1.0, 0.0, 1.0);
	
	

    // Write final color to the framebuffer
	//vec4 test = vec4(billboard[0], 1.0) + vec4(-137.644196, 76.3608627, 82.6696091, -1.0); checksout
	//vec4 test = vec4(billboard[1], 1.0) + vec4(-137.644196, 76.3608627, -117.330391, -1.0); checksout
	//vec4 test = vec4(billboard[3], 1.0) + vec4(-141.01663, -123.610710, 82.6696091, -1.0);
	//color = vec4(length(test));
	//vec3 test = vec3(billboard[3]) + vec3(-141.01663, -123.610710, 82.6696091);
	//color = vec4(length(test), length(test), length(test), 1.0);
	
	
	//color = vec4(reflectDir, 1.0);
	//color = vec4(normalize(fs_in.normalDirection), 1.0);
    //color = fs_in.color;
	//color = texture(tex_color, fs_in.texcoord);
}
