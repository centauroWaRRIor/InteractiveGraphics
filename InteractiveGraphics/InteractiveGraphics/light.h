#pragma once
#include "v3.h"
class Light
{
private:
	V3 position;
	V3 color;
	V3 matColor;
	float ambientK;
public:
	Light();
	Light(const V3 &position, const V3 &color);
	~Light();

	V3 computeDiffuseContribution(const V3 &triangleVertex, const V3 &normal) const;

	// getters
	V3 getPosition(void) const{ return position; }
	V3 getColor(void) const { return color; }
	V3 getMatColor(void) const { return matColor; }
	float getAmbientK(void) const { return ambientK; }

	// setters
	void setPosition(const V3 &pos) { position = pos; }
	void setColor(const V3 &col) { color = col; }
	void setMatColor(const V3 &matCol) { matColor = matCol; }
	void setAmbientK(float ka) { ambientK = ka; }

};

