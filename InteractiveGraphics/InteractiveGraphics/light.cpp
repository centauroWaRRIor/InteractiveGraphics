#include "light.h"
#include "framebuffer.h"
#include "tmesh.h"
#include "ppc.h"

Light::Light(const V3 &pos, const V3 &col)
	: position(pos), color(col)
{
	// default material color is black
	ambientK = 0.0f;

	shadowMapsN = 6;
	shadowMapResWidth = 512;
	shadowMapResHeight = 512;
	shadowMapResHfov = 50.0f;

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

Light::Light()
{
	// default position is at (0.0f,0.0f,0.0f)
	// default color is black
	// default material color is black
	ambientK = 0.0f;

	shadowMapsN = 6;
	shadowMapResWidth = 512;
	shadowMapResHeight = 512;
	shadowMapResHfov = 50.0f;

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

bool Light::isPointInShadow(const V3 & point) const
{
	V3 projP;
	bool isProjValid;
	// assumes the shadow map cube is up to date at this point
	// iterate through shadow maps until we find an answer
	for (unsigned int i = 0; i < shadowMapsN; i++) {
		// project 3d point into shadow map for query
		isProjValid = shadowMapCams[i]->project(point, projP);
		if (isProjValid) {
			// if projection was valid query shadow map
			return shadowMapCube[i]->isOneOverWCloser(projP);
		}
	}
	// couldn't find a clear answer from shadow map cube so assume no
	return false;
}

void Light::buildShadowMaps(
	vector<TMesh *> &tMeshArray)
{
	V3 filColors[3] = { 
		V3(1.0f, 0.0f, 0.0f),
		V3(0.0f, 1.0f, 0.0f),
		V3(0.0f, 0.0f, 1.0f),
	};
	cleanShadowMaps();
	setUpShadowMapCams();
	// For all the shadow maps (6 sides of shadow map cube)
	for (unsigned int i = 0; i < shadowMapsN; i ++ ) {
		// Render all geometry into this shadow map
		unsigned int j = 0;
		for (vector<TMesh *>::iterator it = tMeshArray.begin();
			 it != tMeshArray.end(); 
			 ++it, j++)
		{
				(*it)->drawFilledFlat(
					*shadowMapCube[i],
					*shadowMapCams[i],
					filColors[j % 3].getColor());
		}

	}
	
	// For debug, visualize this shadowMaps
	for (unsigned int i = 0; i < shadowMapsN; i++) {
		shadowMapCube[i]->show();
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
