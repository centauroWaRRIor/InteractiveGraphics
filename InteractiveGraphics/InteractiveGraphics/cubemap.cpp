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
	// build the cube map's six faces
	cubeMapFaces[0] = new Texture(masterTexObject,
		2 * envMapResWidth, 3 * envMapResWidth,
		1 * envMapResHeight, 2 * envMapResHeight);
	cubeMapFacesCams[0]->positionRelativeToPoint(
		V3(0.0f, 0.0f, 0.0f),
		V3(1.0f, 0.0f, 0.0f), // look at pos x direction
		V3(0.0f, 1.0f, 0.0f),
		0.0f);

	cubeMapFaces[1] = new Texture(masterTexObject,
		1 * envMapResWidth, 2 * envMapResWidth,
		1 * envMapResHeight, 2 * envMapResHeight);
	cubeMapFacesCams[1]->positionRelativeToPoint(
		V3(0.0f, 0.0f, 0.0f),
		V3(-1.0f, 0.0f, 0.0f), // look at neg x direction
		V3(0.0f, 1.0f, 0.0f),
		0.0f);

	cubeMapFaces[2] = new Texture(masterTexObject,
		2 * envMapResWidth, 3 * envMapResWidth,
		2 * envMapResHeight, 3 * envMapResHeight);
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

Texture * CubeMap::getCubeFace(unsigned int i)
{
	if (i >= 6)
		return nullptr;
	else
		return cubeMapFaces[i];
}
