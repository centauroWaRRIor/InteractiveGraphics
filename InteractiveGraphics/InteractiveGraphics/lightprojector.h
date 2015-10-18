#pragma once
#include "light.h"
#include "texture.h"
class LightProjector :
	public Light
{
	Texture *texObject;
public:
	LightProjector(const string &texFilename);
	~LightProjector();

	// output color needs to be unsigned int in order to also return the alpha
	// if we were using V3 for color we would lose alpha
	bool getProjectedColor(const V3 &point, unsigned int &outColor) const;
};

