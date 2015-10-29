#include "cubemap.h"

CubeMap::CubeMap(const string & texFilename)
{
	Texture masterTexObject(texFilename);
	envMapResWidth = masterTexObject.getTexWidth() / 3;
	envMapResHeight = masterTexObject.getTexHeight() / 4;
	float envMapResHfov = 90.0f;
	envMapN = 6;
	cubeMapFacesCams = new PPC*[envMapN];
	cubeMapFaces = new Texture*[envMapN];

	for (unsigned int i = 0; i < envMapN; i++) {
		cubeMapFacesCams[i] = new PPC(90.0f, envMapResWidth, envMapResHeight);
	}

	// build the cube map's six faces and cameras
	cubeMapFaces[0] = new Texture(masterTexObject,
		2 * envMapResWidth, 3 * envMapResWidth,
		1 * envMapResHeight, 2 * envMapResHeight);
	cubeMapFacesCams[0]->positionRelativeToPoint(
		V3(0.0f, 0.0f, 0.0f),
		V3(1.0f, 0.0f, 0.0f), // look at pos x direction
		V3(0.0f, 1.0f, 0.0f),
		0.0f);

	cubeMapFaces[1] = new Texture(masterTexObject,
		0 * envMapResWidth, 1 * envMapResWidth,
		1 * envMapResHeight, 2 * envMapResHeight);
	cubeMapFacesCams[1]->positionRelativeToPoint(
		V3(0.0f, 0.0f, 0.0f),
		V3(-1.0f, 0.0f, 0.0f), // look at neg x direction
		V3(0.0f, 1.0f, 0.0f),
		0.0f);

	cubeMapFaces[2] = new Texture(masterTexObject,
		1 * envMapResWidth, 2 * envMapResWidth,
		3 * envMapResHeight, 4 * envMapResHeight);
	cubeMapFaces[2]->flipAboutY();
	cubeMapFacesCams[2]->positionRelativeToPoint(
		V3(0.0f, 0.0f, 0.0f),
		V3(0.0f, 0.0f, -1.0f), // look at pos z direction
		V3(0.0f, 1.0f, 0.0f),
		0.0f);

	cubeMapFaces[3] = new Texture(masterTexObject,
		1 * envMapResWidth, 2 * envMapResWidth,
		1 * envMapResHeight, 2 * envMapResHeight);
	cubeMapFacesCams[3]->positionRelativeToPoint(
		V3(0.0f, 0.0f, 0.0f),
		V3(0.0f, 0.0f, 1.0f), // look at neg z direction
		V3(0.0f, 1.0f, 0.0f),
		0.0f);

	cubeMapFaces[4] = new Texture(masterTexObject,
		1 * envMapResWidth, 2 * envMapResWidth,
		0 * envMapResHeight, 1 * envMapResHeight);
	cubeMapFacesCams[4]->positionRelativeToPoint(
		V3(0.0f, 0.0f, 0.0f),
		V3(0.0f, 1.0f, 0.0f), // look at pos y direction
		V3(0.0f, 0.0f, 1.0f),
		0.0f);

	cubeMapFaces[5] = new Texture(masterTexObject,
		1 * envMapResWidth, 2 * envMapResWidth,
		2 * envMapResHeight, 3 * envMapResHeight);
	cubeMapFacesCams[5]->positionRelativeToPoint(
		V3(0.0f, 0.0f, 0.0f),
		V3(0.0f, -1.0f, 0.0f), // look at neg y direction
		V3(0.0f, 0.0f, 1.0f),
		0.0f);

	// its the same for all cameras so it doesn't matter who
	// supplies this value here.
	cubeMapFocalLength = cubeMapFacesCams[0]->getFocalLength();
	currentLookAtFace = 0;
}


CubeMap::~CubeMap()
{
	for (unsigned int i = 0; i < envMapN; i++) {
		delete cubeMapFacesCams[i];
		delete cubeMapFaces[i];
	}
	delete[] cubeMapFacesCams;
	delete[] cubeMapFaces;
}

Texture * CubeMap::getCubeFace(unsigned int i) const
{
	if (i >= 6)
		return nullptr;
	else
		return cubeMapFaces[i];
}

V3 CubeMap::getColor(const V3 & direction)
{
	// use direction to create a 3D point at the focal plane.
	V3 lookAt3DPoint = cubeMapCenter + (direction * cubeMapFocalLength);
	V3 projectedPoint;
	unsigned int returnColor;
	bool isProjValid = false;
	// find the face that sees this point but start by the one used last time
	// to leverage locality principle.
	while(!isProjValid) {

		isProjValid = cubeMapFacesCams[currentLookAtFace]->project(lookAt3DPoint, projectedPoint);
		// be very strict with the projection: no projection if:
		// - point is left of view frustrum or is right of view frustrum
		// - or is above view frustrum or is below of view frustrum
		if (isProjValid &&
			(projectedPoint[0] > 0.0f) && (projectedPoint[0] < envMapResWidth) &&
			(projectedPoint[1] > 0.0f) && (projectedPoint[1] < envMapResHeight)) {

			// go from x,y to s,t which ranges from [0,1]
			float s = projectedPoint[0] / (envMapResWidth - 1.0f);
			float t = projectedPoint[1] / (envMapResWidth - 1.0f);
			returnColor = cubeMapFaces[currentLookAtFace]->sampleTexBilinearTile(s, t);
			return V3(returnColor);
		}
		else
			currentLookAtFace++; // try with a different face of the cube
	} 
	// This should not ever happen. Any ray direction should be contained by a cubemap
	return V3();
}

V3 CubeMap::getSpecularReflectionColor(const V3 & normal) const
{
	return V3();
}

V3 CubeMap::getFirstSurfaceRefraction(const V3 & direction) const
{
	return V3();
}
