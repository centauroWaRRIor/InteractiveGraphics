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
	V3 cubeMapCenter; // (0.0f,0.0f,0.0f) by default
	float cubeMapFocalLength;
	unsigned int currentLookAtFace;

public:
	CubeMap(const string & texFilename);
	~CubeMap();

	Texture *getCubeFace(unsigned int i) const;
	V3 getColor(const V3 &direction);
	V3 getSpecularReflectionColor(const V3 &normal) const;
	V3 getFirstSurfaceRefraction(const V3 &direction) const;
};

