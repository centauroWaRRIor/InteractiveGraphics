#include "texture.h"
#include "lodepng.h"
using std::size_t;
#include <iostream>
using std::endl;
using std::cerr;
#include <algorithm>
using std::min;
using std::max;

void Texture::loadPngTexture(const string &filename)
{
	//decode
	unsigned error = lodepng::decode(texels, texWidth, texHeight, filename.c_str());

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
}

float Texture::clip(float n, float lower, float upper) const
{
	return max(lower, min(n, upper));
}

Texture::Texture(const string &filename)
{
	if (filename.find(".png") != std::string::npos) {
		loadPngTexture(filename);
	}
	else if (filename.find(".tiff") != std::string::npos) {
		cerr << "ERROR: tiff support is still under construction..."  << endl;
	}
	else {
		cerr << "ERROR: Uknown texture file format not supported..." << endl;
	}
}

unsigned int Texture::sampleTexNearClamp(float floatS, float floatT) const
{
	// clamps [0-1] and then maps to [0, texWidth - 1]
	floatS = clip(floatS, 0.0f, 1.0f) * (texWidth - 1);
	unsigned int intS = (unsigned int)(floatS + 0.5f); // round up >5 or down <5
	// clamps [0-1] and then maps to [0, texHeight - 1]
	floatT = clip(floatT, 0.0f, 1.0f) * (texHeight - 1);
	unsigned int intT = (unsigned int)(floatT + 0.5f); // round up >5 or down <5
	// t needs to be inverted
	intT = (texHeight - 1) - intT;

	unsigned char red, green, blue, alpha;
	unsigned int sampleColor;
	
	unsigned int texelIndex = (intT * texWidth + intS) * 4;
	red = texels[texelIndex + 0];
	green = texels[texelIndex + 1];
	blue = texels[texelIndex + 2];
	alpha = texels[texelIndex + 3];

	((unsigned char*)(&sampleColor))[0] = red;
	((unsigned char*)(&sampleColor))[1] = green;
	((unsigned char*)(&sampleColor))[2] = blue;
	((unsigned char*)(&sampleColor))[3] = alpha;

	return sampleColor;
}

unsigned int Texture::sampleTexNearTile(float floatS, float floatT) const
{
	// tile s,t by only keeping decimal part
	if (floatS > 1.0)
		floatS = floatS - ((int)floatS);
	if (floatT > 1.0)
		floatT = floatT - ((int)floatT);
	// clips negative numbers and then maps [0,1) to [0, texWidth - 1]
	floatS = clip(floatS, 0.0f, 1.0f) * (texWidth - 1);
	unsigned int intS = (unsigned int)(floatS + 0.5f); // round up >5 or down <5
	// clips negative numbers and then maps [0,1) to [0, texHeight - 1]
	floatT = clip(floatT, 0.0f, 1.0f) * (texHeight - 1);
	unsigned int intT = (unsigned int)(floatT + 0.5f); // round up >5 or down <5
	// t needs to be inverted
	intT = (texHeight - 1) - intT;

	unsigned char red, green, blue, alpha;
	unsigned int sampleColor;

	unsigned int texelIndex = (intT * texWidth + intS) * 4;
	red = texels[texelIndex + 0];
	green = texels[texelIndex + 1];
	blue = texels[texelIndex + 2];
	alpha = texels[texelIndex + 3];

	((unsigned char*)(&sampleColor))[0] = red;
	((unsigned char*)(&sampleColor))[1] = green;
	((unsigned char*)(&sampleColor))[2] = blue;
	((unsigned char*)(&sampleColor))[3] = alpha;

	return sampleColor;
}

unsigned int Texture::sampleTexBilinearTile(float floatS, float floatT) const
{
	// tile s,t by only keeping decimal part
	if (floatS > 1.0)
		floatS = floatS - ((int)floatS);
	if (floatT > 1.0)
		floatT = floatT - ((int)floatT);
	// clips negative numbers and then maps [0,1) to [0, texWidth - 1]
	floatS = clip(floatS, 0.0f, 1.0f) * (texWidth - 1);
	// clips negative numbers and then maps [0,1) to [0, texHeight - 1]
	floatT = clip(floatT, 0.0f, 1.0f) * (texHeight - 1);
	
	// get decimal parts
	float dS = floatT - ((int)floatT);
	float dT = floatT - ((int)floatT);

	unsigned int intS = (unsigned int)(floatS + 0.5f); // round up >5 or down <5
	unsigned int intT = (unsigned int)(floatT + 0.5f); // round up >5 or down <5

	// t needs to be inverted
	intT = (texHeight - 1) - intT;

	// hint use modulo in order to wrap up in the corner cases
	
	// t needs to be inverted
	//intT = (texHeight - 1) - intT;
	//unsigned char red, green, blue, alpha;
	//unsigned int sampleColor;

	//unsigned int texelIndex = (intT * texWidth + intS) * 4;
	//red = texels[texelIndex + 0];
	//green = texels[texelIndex + 1];
	//blue = texels[texelIndex + 2];
	//alpha = texels[texelIndex + 3];

	//((unsigned char*)(&sampleColor))[0] = red;
	//((unsigned char*)(&sampleColor))[1] = green;
	//((unsigned char*)(&sampleColor))[2] = blue;
	//((unsigned char*)(&sampleColor))[3] = alpha;

	//return sampleColor;
	return 0;
}
