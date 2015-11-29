#version 420 core

// uniforms
// note: using this syntax I avoid uploading the uniform manually
//       binding 0 corresponds to GL_TEXTURE0 unit 
//       binding 1 corresponds to GL_TEXTURE1 unit so on and so forth
layout (binding = 0) uniform sampler2D billboardTexColor_1;
layout (binding = 1) uniform sampler2D billboardTexColor_2;
layout (binding = 2) uniform samplerCube tex_cubemap;

uniform vec3 eyePosition;

uniform vec3 billboardVerts_1[4]; // billboard vertices
uniform vec2 billboardTcs_1[4]; // billboard texture coordinates
uniform vec3 billboardVerts_2[4]; // billboard vertices
uniform vec2 billboardTcs_2[4]; // billboard texture coordinates

// Output
layout (location = 0) out vec4 color;
// note: default layout(location = 0) is piped to the back buffer
//       so we can just do out vec4 color;

// Input from vertex shader
in VS_OUT
{
    vec4 color;
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

bool calcImpostorReflectColor(
	in vec3 P, 
	in vec3 billboardVerts[4], 
	in vec2 billboardTcs[4],
	in sampler2D billboardTexColor,
	out vec4 color)
{
	// billboard is made of triangles
	// 1) billboardVerts[0], billboardVerts[1], billboardVerts[3]
	// 2) billboardVerts[1], billboardVerts[2], billboardVerts[3]

	vec3 barycentricWeights;
	vec2 barycentricTcs;
	
	// try triangle 1 of billboard
	if(intersectTriangle(
		P,
		billboardVerts[0],
		billboardVerts[1],
		billboardVerts[3],
		barycentricWeights)) 
	{
		// compute texture coordinates using barycentric weights
		barycentricTcs = 
			barycentricWeights[0]*billboardTcs[0] +
			barycentricWeights[1]*billboardTcs[1] +
			barycentricWeights[2]*billboardTcs[3];

		color = texture(billboardTexColor, barycentricTcs);
		return true;
	}
	// try triangle 2 of billboard
	else if(intersectTriangle(
			P.xyz,
			billboardVerts[1],
			billboardVerts[2],
			billboardVerts[3],
			barycentricWeights))
	{
		// compute texture coordinates using barycentric weights
		barycentricTcs = 
			barycentricWeights[0]*billboardTcs[1] +
			barycentricWeights[1]*billboardTcs[2] +
			barycentricWeights[2]*billboardTcs[3];
				
		color = texture(billboardTexColor, barycentricTcs);
		return true;
	}
	else {
		color = vec4(0.0, 0.0, 0.0, 1.0); // don't care color, not used
		return false;
	}
}

void main(void)
{
	const float INFINITY = 1e10;
	const float MAXINTERSECTDISTANCE = 500.0;//220.0;
	float intersectDistance = INFINITY;
	float t, LdotV, mixFactor;
	vec4 reflectedColor, S, P, L, V;
	vec3 billboardNormal;
	
	// calculate reflected ray direction
    vec3 viewDirection = fs_in.modelSpaceXYZ - eyePosition;
	viewDirection = normalize(viewDirection);	
	// because the two inputs are normalized reflectDir is normalized
	vec3 reflectDir = reflect(viewDirection, normalize(fs_in.normalDirection));
	
	// use ray to compute color from environment
	//vec4 envColor  = texture(tex_cubemap, reflectDir) * fs_in.color; // this works but makes harder to see reflection
	vec4 envColor  = texture(tex_cubemap, reflectDir);
	
	// 4D plane equation is from "Mathematics for 3D Game Programming and Computer Graphics"
	// calculate plane equation for billboard1
	billboardNormal = 
		cross(	billboardVerts_1[1] - billboardVerts_1[0], 
				billboardVerts_1[3] - billboardVerts_1[0]);
	billboardNormal = normalize(billboardNormal);
	// billboard's plane equation is represented as a 4D vector
	L = vec4(billboardNormal, dot(-billboardNormal, billboardVerts_1[0])); 
	// L dot V helps determining if there is an intersection possibility
	V = vec4(reflectDir, 0.0); // rays direction
	LdotV = dot(L, V);
	
	// no chance of intersection occurs if = 0.0 (reflected ray is parallel to billboard plane in that case)
	if(abs(LdotV) > 0.01)
	{	
		// safe to proceed (no division by zero danger)
		
		// find P (intersection point)-> P(t) = S + tV
		S = vec4(fs_in.modelSpaceXYZ, 1.0); // starting ray position
		t = dot(L, S) / LdotV;
		t *= -1.0;
		// compute P
		P = S + (t * V);
		
		if(	(t > 0) && 
			(length(P.xyz) < intersectDistance) && 
			calcImpostorReflectColor(
				P.xyz, 
				billboardVerts_1, 
				billboardTcs_1,
				billboardTexColor_1,
				reflectedColor) &&
			reflectedColor.a == 1.0) 
		{
			intersectDistance = length(P.xyz);
		}		
	}
	
	// repeat same process from above with billboard 2
	billboardNormal = 
		cross(	billboardVerts_2[1] - billboardVerts_2[0], 
				billboardVerts_2[3] - billboardVerts_2[0]);
	billboardNormal = normalize(billboardNormal);
	// billboard's plane equation is represented as a 4D vector
	L = vec4(billboardNormal, dot(-billboardNormal, billboardVerts_2[0])); 
	// L dot V helps determining if there is an intersection possibility
	V = vec4(reflectDir, 0.0); // rays direction
	LdotV = dot(L, V);
	
	// no chance of intersection occurs if = 0.0 (reflected ray is parallel to billboard plane in that case)
	if(abs(LdotV) > 0.01)
	{	
		// safe to proceed (no division by zero danger)
		
		// find P (intersection point)-> P(t) = S + tV
		S = vec4(fs_in.modelSpaceXYZ, 1.0); // starting ray position
		t = dot(L, S) / LdotV;
		t *= -1.0;
		// compute P
		P = S + (t * V);
		
		if(	(t > 0) && 
			(length(P.xyz) < intersectDistance) && 
			calcImpostorReflectColor(
				P.xyz, 
				billboardVerts_2, 
				billboardTcs_2,
				billboardTexColor_2,
				reflectedColor) &&
			reflectedColor.a == 1.0)
		{
			intersectDistance = length(P.xyz);
		}		
	}
	
	if(intersectDistance == INFINITY) // if there was no intersection with ANY billboard
		color = envColor;
	else
	{
		// modulate reflected color by the distance to intersection
		mixFactor = intersectDistance / MAXINTERSECTDISTANCE;
		mixFactor = clamp(mixFactor, 0.0, 1.0);
		mixFactor = 1.0 - mixFactor; // the farther the less influence it has
		// reflected color is always the color of the closest reflection modulated by the
		// maximum distance allowed
		color = mix(envColor, reflectedColor, mixFactor);
	}
	
    // some useful for debug only
	//vec4 test = vec4(billboardVerts_1[0], 1.0) + vec4(-137.644196, 76.3608627, 82.6696091, -1.0); checksout
	//vec4 test = vec4(billboardVerts_1[1], 1.0) + vec4(-137.644196, 76.3608627, -117.330391, -1.0); checksout
	//vec4 test = vec4(billboardVerts_1[3], 1.0) + vec4(-141.01663, -123.610710, 82.6696091, -1.0);
	//color = vec4(length(test));
	//vec3 test = vec3(billboardVerts_1[3]) + vec3(-141.01663, -123.610710, 82.6696091);
	//color = vec4(length(test), length(test), length(test), 1.0);
	
	//color = vec4(billboardVerts_1[3], 1.0);
	//color = vec4(reflectDir, 1.0);
	//color = vec4(normalize(fs_in.normalDirection), 1.0);
}