#pragma once
#include <vector>
using std::vector;
#include "v3.h"
// Forward delcarations
class SWFrameBuffer;
class PPC;
class TMesh;

class Light
{
protected:
	bool isPointLgiht; // is point light vs directional light
	bool isUsingCubemap; // cubemap is an option only for point lights
	V3 position; // used for point lights
	V3 direction; // used for directional lights (normalized)
	V3 color;
	V3 matColor;
	float ambientK;

	SWFrameBuffer **shadowMapCube;
	PPC **shadowMapCams;
	unsigned int shadowMapsN;
	unsigned int shadowMapResWidth;
	unsigned int shadowMapResHeight;
	float shadowMapResHfov;

	void cleanShadowMaps(void);
	void setUpShadowMapCams(void);

public:
	Light(bool isPointLight = true, float hfov = 0.0f);
	~Light();

	// return lit color for triangle vertex
	V3 computeDiffuseContribution(const V3 &triangleVertex, const V3 &normal) const;
	// return whether or not 3D point is in shadow casted by this light
	bool isPointInShadow(const V3 &point) const;
	// renders array of triangle meshes into shadow maps
	// Note: Using vector as opposed to TMesh *TMeshArray or
	// TMesh TMeshArray[] because the TMesh array is not generated at once
	// but rather sporadically and therefore the memory for the array is not 
	// allocated contiguosly. Vector allows us to pass a list of pointers. 
	void buildShadowMaps(
		vector<TMesh *> &tMeshArray,
		bool isDbgShowShadowMaps = false,
		bool isDrawModeFlat = true);
	// draws itself for visual debug
	void draw(SWFrameBuffer &fb, const PPC &ppc, V3 &color) const;

	// getters
	bool getIsPointLight(void) const { return isPointLgiht; }
	V3 getPosition(void) const { return position; }
	V3 getDirection(void) const { return direction; }
	V3 getColor(void) const { return color; }
	V3 getMatColor(void) const { return matColor; }
	float getAmbientK(void) const { return ambientK; }

	// setters
	void setPosition(const V3 &pos) { position = pos; };
	void setDirection(const V3 &dir) { direction = dir; };
	void setColor(const V3 &col) { color = col; }
	void setMatColor(const V3 &matCol) { matColor = matCol; }
	void setAmbientK(float ka) { ambientK = ka; }
	void setIsUsingCubemap(bool value) { isUsingCubemap = value; }
};

