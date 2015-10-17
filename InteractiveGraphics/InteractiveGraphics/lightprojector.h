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

	bool getProjectedColor(const V3 &point, V3 &outColor) const;
};

