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
public:
	Texture(const string &filename);
	//~Texture(); // automatic destructor should suffice

	unsigned int getTexWidth(void) const { return texWidth; }
	unsigned int getTexHeight(void) const { return texHeight; }

	unsigned int sampleTex(float s, float t) const;
};