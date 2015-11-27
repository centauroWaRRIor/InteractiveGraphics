#version 420 core

// uniforms
// note: using this syntax I avoid uploading the uniform manually
//       binding 0 corresponds to GL_TEXTURE0 unit 
//       binding 1 corresponds to GL_TEXTURE1 unit so on and so forth
layout (binding = 0) uniform sampler2D tex_color;

uniform vec3 eyePosition;
uniform vec3 billboard[4]; // billboard vertices
uniform vec2 billboardTcs[4]; // billboard texture coordinates

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

// this function calculates the barycentric coordinates of P with respect to
// the triangles P0, P1, and P2 by way of calculating the wegihts w0, w1, w2
// such that P = w0P0 + w1P1 + w2P2. Further, these weights can be used to 
// interpolate other vertex attributes within the triangle such as colors and
// texture coordinates.
bool intersectTriangle(
	in vec3 P, // 
	in vec3 P0,
	in vec3 P1, 
	in vec3 P2, 
	out vec3 weights)
{
	// the math magic here was obtained from "Mathenatics for 3D Game Programming 
	// and Computer Graphics" by Eric Lengyel
	vec3 R = P - P0;
	vec3 Q1 = P1 - P0;
	vec3 Q2 = P2 - P0;
	float Q1dotQ2 = dot(Q1, Q2);
	float Q1Squared = dot(Q1, Q1);
	float Q2Squared = dot(Q2, Q2);
	
	mat2 matrixA;
	matrixA[0][0] = Q2Squared;
	matrixA[0][1] = -Q1dotQ2;
	matrixA[1][0] = -Q1dotQ2;
	matrixA[1][1] = Q1Squared;
	
	vec2 B;
	B[0] = dot(R, Q1);
	B[1] = dot(R, Q2);
	
	float C = 1 / (Q1Squared*Q2Squared - pow(Q1dotQ2, 2));
	
	vec2 weightsResult = (matrixA * B) * C;
	weights[1] = weightsResult[0];
	weights[2] = weightsResult[1];
	weights[0] = 1 - weights[1] - weights[2];
	
	return (weights[0] >= 0) && (weights[1] >= 0) && (weights[2] >= 0);
}

void main(void)
{
    vec3 viewDirection = fs_in.modelSpaceXYZ - eyePosition;
	viewDirection = normalize(viewDirection);
	
	vec3 billboardNormal = cross(billboard[1] - billboard[0], billboard[3] - billboard[0]);
	
	// from "Mathematics for 3D Game Programming and Computer Graphics"
	vec3 N = normalize(billboardNormal);
	vec4 L = vec4(N, dot(-N, billboard[0])); // 4D billboard plane vector
	
	// because the two inputs are normalized reflectDir is normalized
	vec3 reflectDir = reflect(viewDirection, normalize(fs_in.normalDirection));
	
	vec4 V = vec4(reflectDir, 0.0); // rays direction
	float LdotV = dot(L, V);
	if(abs(LdotV) < 0.01) // no intersection occurs
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else 
	{
		// find P (intersection point)-> P(t) = S + tV
		vec4 S = vec4(fs_in.modelSpaceXYZ, 1.0); // starting ray position
		float t = dot(L, S) / LdotV;
		t *= -1.0;
		// compute P
		vec4 P = S + (t * V);
		vec3 barycentricWeights;
		//vec2 barycentricTcs_s, barycentricTcs_t; 
		vec2 barycentricTcs;
		// billboard is made of triangles
		// 1) billboard[0], billboard[1], billboard[3]
		// 2) billboard[1], billboard[2], billboard[3]

		// try triangle 1/2 of billboard 1
		if(t > 0 && 
			intersectTriangle(
				P.xyz,
				billboard[0],
				billboard[1],
				billboard[3],
				barycentricWeights)) 
		{
			// compute texture coordinates using barycentric weights
			barycentricTcs = 
				barycentricWeights[0]*billboardTcs[0] +
				barycentricWeights[1]*billboardTcs[1] +
				barycentricWeights[2]*billboardTcs[3];

			color = texture(tex_color, barycentricTcs);
		}
		// try triangle 2/2 of billboard 2
		else if(t > 0 && 
			intersectTriangle(
				P.xyz,
				billboard[1],
				billboard[2],
				billboard[3],
				barycentricWeights))
		{
			// compute texture coordinates using barycentric weights
			barycentricTcs = 
				barycentricWeights[0]*billboardTcs[1] +
				barycentricWeights[1]*billboardTcs[2] +
				barycentricWeights[2]*billboardTcs[3];
				
			color = texture(tex_color, barycentricTcs);
		}
		else {
				color = vec4(1.0, 1.0, 0.0, 1.0);
		}
	}
	
    // Write final color to the framebuffer
	//vec4 test = vec4(billboard[0], 1.0) + vec4(-137.644196, 76.3608627, 82.6696091, -1.0); checksout
	//vec4 test = vec4(billboard[1], 1.0) + vec4(-137.644196, 76.3608627, -117.330391, -1.0); checksout
	//vec4 test = vec4(billboard[3], 1.0) + vec4(-141.01663, -123.610710, 82.6696091, -1.0);
	//color = vec4(length(test));
	//vec3 test = vec3(billboard[3]) + vec3(-141.01663, -123.610710, 82.6696091);
	//color = vec4(length(test), length(test), length(test), 1.0);
	
	
	//color = vec4(billboard[3], 1.0);
	//color = vec4(reflectDir, 1.0);
	//color = vec4(normalize(fs_in.normalDirection), 1.0);
    //color = fs_in.color;
	//color = texture(tex_color, fs_in.texcoord);
}
