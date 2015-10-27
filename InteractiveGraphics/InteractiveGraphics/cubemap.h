#pragma once
#include "texture.h"
#include "ppc.h"

class CubeMap
{
	Texture **cubeMapFaces;
	PPC **cubeMapFacesCams;
	unsigned int envMapN;
	unsigned int envMapResWidth;
	unsigned int envMapResHeight;
	float envMapResHfov;

public:
	CubeMap(const string & texFilename);
	~CubeMap();
};

