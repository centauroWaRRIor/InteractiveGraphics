#include "lightprojector.h"
#include "ppc.h"

LightProjector::LightProjector(const string & texFilename) :
	Light(false) // call base class constructor for a directional light
{
	texObject = new Texture(texFilename);
}

LightProjector::~LightProjector()
{
	if (texObject)
		delete texObject;
	// note to self:
	// c++ automagically calls Light destructor
	// no need to explicitly call it here
}

bool LightProjector::getProjectedColor(const V3 & point, unsigned int &outColor) const
{
	// determine if point projects to shadow region or not
	static float const epsilon = 0.15f;
	PPC *projectorPPC = nullptr;
	V3 projP;
	unsigned int texelColor;
	bool isProjValid;
	// project 3d point into shadow map for query	
	isProjValid = shadowMapCams[0]->project(point, projP);
	// be very strict with the projection:
	// no projection if:
	// point is left of view frustrum or is right of view frustrum
	// or is above view frustrum or is below of view frustrum
	if (isProjValid &&
		(projP[0] > 0.0f) && (projP[0] < shadowMapCams[0]->getWidth()) &&
		(projP[1] > 0.0f) && (projP[1] < shadowMapCams[0]->getHeight()) &&
		// if projection was valid, query shadow map to see if in shadow or not
		shadowMapCube[0]->isDepthTestPass(projP, epsilon)) {

		// is hit by the light of this light projector

		// derive s,t [0-1] assuming texture is applied to shadowmap framebuffer
		projP[0] = projP[0] / shadowMapCams[0]->getWidth();
		projP[1] = (shadowMapCams[0]->getHeight() - projP[1]) / 
			shadowMapCams[0]->getHeight(); // v is inverted
		// sample texture using s,t of the projected pixel coordinates
		// using near clamp for two reasons: No need for tiling since uvs come
		// from screen coordinates and second reason is that bilinear does not
		// support alpha at the time.
		texelColor = texObject->sampleTexNearClamp(projP[0], projP[1]);
		outColor = texelColor;
		return true;
	}
	outColor = 0x00000000;
	return false;
}