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
	//~Texture(); // automatic destructor should suffice

	unsigned int getTexWidth(void) const { return texWidth; }
	unsigned int getTexHeight(void) const { return texHeight; }

	unsigned int sampleTexClamp(float s, float t) const;
	unsigned int sampleTexTile(float s, float t) const;
};