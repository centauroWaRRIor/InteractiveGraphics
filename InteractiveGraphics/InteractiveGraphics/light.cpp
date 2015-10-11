#include "light.h"

Light::Light(const V3 &pos, const V3 &col)
	: position(pos), color(col)
{

}

V3 Light::computeDiffuseContribution(const V3 & triangleVertex, const V3 & normal) const
{
	V3 lightVector = position - triangleVertex;
	lightVector.normalize();
	float kd = lightVector * normal;
	if (kd < 0.0f)
		kd = 0.0f;
	// kd goes from 0 to 1

	V3 ret = matColor * (ambientK + kd * (1.0f - ambientK));

	return ret;
}

Light::Light()
{
	// default position is at (0.0f,0.0f,0.0f)
	// default color is black
	// default material color is black
	ambientK = 0.0f;
}


Light::~Light()
{
}
