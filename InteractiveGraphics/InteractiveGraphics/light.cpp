#include "light.h"
#include "framebuffer.h"
#include "tmesh.h"
#include "ppc.h"
#include "scene.h"

Light::Light(bool isPointLight) :
	isPointLgiht(isPointLight),
	position(V3()),
	direction(V3(0.0f, 0.0f, -1.0f)), // this can't be zero by default or it will affect positioning
	color(V3()),
	matColor(V3()),
	ambientK(0.0f),
	shadowMapCube(nullptr),
	shadowMapCams(nullptr),
	shadowMapsN(0),
	// because the pixel will be unprojected to 3D using the rendering camera,
	// these shadow map cameras have to match its view frustrum.
	shadowMapResWidth(Scene::K_W),
	shadowMapResHeight(Scene::K_H),
	shadowMapResHfov(Scene::K_HFOV)

{
	if (isPointLight) {
		shadowMapsN = 6;
	}
	else { // directional light 
		shadowMapsN = 1;
	}

	shadowMapCube = new FrameBuffer*[shadowMapsN];
	shadowMapCams = new PPC*[shadowMapsN];

	for (unsigned int i = 0; i < shadowMapsN; i++) {
		// set up shadow maps
		shadowMapCube[i] = new FrameBuffer(0, 0,
			shadowMapResWidth, shadowMapResHeight);
		// set up shadow maps aux cameras
		shadowMapCams[i] = new PPC(
			shadowMapResHfov,
			shadowMapResWidth,
			shadowMapResHeight);
	}
	// position and orient shadow maps aux cameras
	setUpShadowMapCams();
}


Light::~Light()
{
	for (unsigned int i = 0; i < shadowMapsN; i++) {
		delete shadowMapCube[i];
		delete shadowMapCams[i];
	}
	delete[] shadowMapCams;
	delete[] shadowMapCube;
}

V3 Light::computeDiffuseContribution(const V3 & triangleVertex, const V3 & normal) const
{
	V3 lightVector;
	if (isPointLgiht) {
		lightVector = position - triangleVertex;
	}
	else
		lightVector = direction * (-1.0f);
	lightVector.normalize();

	float kd = lightVector * normal;
	if (kd < 0.0f)
		kd = 0.0f;
	// kd goes from 0 to 1

	V3 ret = matColor * (ambientK + kd * (1.0f - ambientK));

	return ret;
}

bool Light::isPointInShadow(const V3 & point) const
{
	static float const epsilon = 0.15f;
	V3 projP;
	bool isProjValid;
	// assumes the shadow map cube is up to date at this point
	// iterate through shadow maps until we find an answer
	for (unsigned int i = 0; i < shadowMapsN; i++) {
		// project 3d point into shadow map for query
		isProjValid = shadowMapCams[i]->project(point, projP);
		// be very strict with the projection:
		// no projection if:
		// point is left of view frustrum or is right of view frustrum
		// or is above view frustrum or is below of view frustrum
		if (isProjValid &&
			(projP[0] > 0.0f) && (projP[0] < shadowMapCams[i]->getWidth()) &&
			(projP[1] > 0.0f) && (projP[1] < shadowMapCams[i]->getHeight())) {
			// if projection was valid, query shadow map
			return !(shadowMapCube[i]->isDepthTestPass(projP, epsilon));
		}
	}
	// couldn't find a clear answer from shadow map cube so assume no
	return false;
}

void Light::buildShadowMaps(
	vector<TMesh *> &tMeshArray,
	bool isDbgShowShadowMaps)
{
	V3 filColors[3] = { // for visual debug (isDbgShowShadowMaps)
		V3(1.0f, 0.0f, 0.0f),
		V3(0.0f, 1.0f, 0.0f),
		V3(0.0f, 0.0f, 1.0f),
	};
	cleanShadowMaps();
	setUpShadowMapCams();
	// For all the shadow maps 
	// (6 sides of shadow map cube for point lights or 
	// just one shadow map for directional lights)
	for (unsigned int i = 0; i < shadowMapsN; i ++ ) {
		// Render all geometry into this shadow map
		unsigned int j = 0;
		for (vector<TMesh *>::iterator it = tMeshArray.begin();
			 it != tMeshArray.end(); 
			 ++it, j++)
		{
				(*it)->drawFilledFlatWithDepth(
					*shadowMapCube[i],
					*shadowMapCams[i],
					filColors[j % 3].getColor());
		}

	}
	
	// For debug, visualize these shadowMaps as framebuffers
	if (isDbgShowShadowMaps) {
		for (unsigned int i = 0; i < shadowMapsN; i++) {
			shadowMapCube[i]->show();
		}
	}
}

void Light::draw(FrameBuffer & fb, const PPC & ppc, V3 & color) const
{
	const float lightDotSize = 10.0f;
	V3 projLightPos;
	if (ppc.project(this->position, projLightPos)) {
		// Draw light as camera aligned circle
		fb.draw2DCircleIfCloser(projLightPos, lightDotSize, color);
	}
}

void Light::cleanShadowMaps(void)
{
	for (unsigned int i = 0; i < shadowMapsN; i++) {
		shadowMapCube[i]->clearZB(0.0f);
		shadowMapCube[i]->set(0xFFFFFFFF);
	}
}

void Light::setUpShadowMapCams(void)
{
	// assumes the camera objects have been built at this point
	if (isPointLgiht) {
		shadowMapCams[0]->positionRelativeToPoint(
			position,
			V3(1.0f, 0.0f, 0.0f), // look at pos x direction
			V3(0.0f, 1.0f, 0.0f),
			0.0f);
		shadowMapCams[1]->positionRelativeToPoint(
			position,
			V3(-1.0f, 0.0f, 0.0f), // look at neg x direction
			V3(0.0f, 1.0f, 0.0f),
			0.0f);
		shadowMapCams[2]->positionRelativeToPoint(
			position,
			V3(0.0f, 0.0f, -1.0f), // look at pos z direction
			V3(0.0f, 1.0f, 0.0f),
			0.0f);
		shadowMapCams[3]->positionRelativeToPoint(
			position,
			V3(0.0f, 0.0f, 1.0f), // look at neg z direction
			V3(0.0f, 1.0f, 0.0f),
			0.0f);
		shadowMapCams[4]->positionRelativeToPoint(
			position,
			V3(0.0f, 1.0f, 0.0f), // look at pos y direction
			V3(0.0f, 0.0f, 1.0f),
			0.0f);
		shadowMapCams[5]->positionRelativeToPoint(
			position,
			V3(0.0f, -1.0f, 0.0f), // look at neg y direction
			V3(0.0f, 0.0f, 1.0f),
			0.0f);
	}
	else {
		shadowMapCams[0]->positionRelativeToPoint(
			position,
			direction,
			V3(0.0f, 1.0f, 0.0f),
			0.0f);
	}
}
