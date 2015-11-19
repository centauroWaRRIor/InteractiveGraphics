#pragma once
#include <string>
using std::string;
#include <vector>
using std::vector;

// Implements a texture class for texturing operations
class Texture {

private:
	vector<unsigned char> texels;
	unsigned int texWidth, texHeight;

	void loadPngTexture(const string &filename);
	float clip(float n, float lower, float upper) const;
public:
	Texture(const string &filename);
	// constructs a texture out of a sub section of 
	// another texture. Useful for building env maps.
	Texture(const Texture &otherTexture, 
		unsigned int beginS, unsigned int endS,
		unsigned int beginT, unsigned int endT);
	//~Texture(); // automatic destructor should suffice

	unsigned int getTexWidth(void) const { return texWidth; }
	unsigned int getTexHeight(void) const { return texHeight; }
	vector<unsigned char> getTexels(void) const { return texels; }
	vector<unsigned char>& getTexelsRef(void) { return texels; }

	// supports alpha texture
	unsigned int sampleTexNearClamp(float s, float t) const;
	// supports alpha texture
	unsigned int sampleTexNearTile(float s, float t) const;
	// does not support alpha texture due to use of V3 to 
	// do vector interpolation (alpha the 4 component gets lost)
	unsigned int sampleTexBilinearTile(float s, float t) const;
	// flips image upside down
	void flipAboutY(void);
};