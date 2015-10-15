#pragma once
#include <vector>
using std::vector;
#include "v3.h"
// Forward delcarations
class FrameBuffer;
class PPC;
class TMesh;

class Light
{
private:
	V3 position;
	V3 color;
	V3 matColor;
	float ambientK;
	FrameBuffer **shadowMapCube;
	PPC **shadowMapCams;
	unsigned int shadowMapsN;
	unsigned int shadowMapResWidth;
	unsigned int shadowMapResHeight;
	float shadowMapResHfov;

	void cleanShadowMaps(void);
	void setUpShadowMapCams(void);
public:
	Light();
	Light(const V3 &position, const V3 &color);
	~Light();

	// return lit color for triangle vertex
	V3 computeDiffuseContribution(const V3 &triangleVertex, const V3 &normal) const;
	// return whether or not 3D poitn is in shadow casted by this light
	bool isPointInShadow(const V3 &point) const;
	// renders array of triangle meshes into shadow maps
	// Note: Using vector as opposed to TMesh *TMeshArray or
	// TMesh TMeshArray[] because the TMesh array is not generated at once
	// but rather sporadically and therefore the memory for the array is not 
	// allocated contiguosly. Vector allows us to pass a list of pointers. 
	void buildShadowMaps(vector<TMesh *> &tMeshArray);

	// getters
	V3 getPosition(void) const{ return position; }
	V3 getColor(void) const { return color; }
	V3 getMatColor(void) const { return matColor; }
	float getAmbientK(void) const { return ambientK; }

	// setters
	void setColor(const V3 &col) { color = col; }
	void setMatColor(const V3 &matCol) { matColor = matCol; }
	void setAmbientK(float ka) { ambientK = ka; }
	void setPosition(const V3 &pos);
};

