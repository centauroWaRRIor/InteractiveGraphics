#include "cubemap.h"

CubeMap::CubeMap(const string & texFilename)
{
	Texture masterTexObject(texFilename);
	envMapResWidth = masterTexObject.getTexWidth() / 3;
	envMapResHeight = masterTexObject.getTexHeight() / 4;
	float envMapResHfov = 90.0f;
	envMapN = 6.0f;
	cubeMapFacesCams = new PPC*[envMapN];
	cubeMapFaces = new Texture*[envMapN];

	// TODO build six faces using the camera directions and 
	// the texture special constructor using sub texture
	// uvs

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