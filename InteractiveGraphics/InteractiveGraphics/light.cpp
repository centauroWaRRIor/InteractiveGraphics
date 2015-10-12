#include "light.h"
#include "framebuffer.h"
#include "ppc.h"

Light::Light(const V3 &pos, const V3 &col)
	: position(pos), color(col)
{
	// set up shadow maps
	shadowMapsN = 4;
	shadowMapCube = new FrameBuffer*[shadowMapsN];
	for (unsigned int i = 0; i < shadowMapsN; i++) {
		shadowMapCube[i] = new FrameBuffer(0, 0, 
			shadowMapResWidth, shadowMapResHeight);
	}
	// set up shadow map aux camera
	shadowMapResWidth = 512;
	shadowMapResHeight = 512;
	shadowMapResHfov = 550.0f;
	shadowMapAuxCam = new PPC(
		shadowMapResHfov, 
		shadowMapResWidth, 
		shadowMapResHeight);
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

void Light::cleanShadowMaps(void)
{

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
	for (unsigned int i = 0; i < shadowMapsN; i++) {
		delete shadowMapCube[i];
	}
	delete shadowMapAuxCam;
}
