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

unsigned int Texture::sampleTex(float floatS, float floatT) const
{
	// assumes input s and t go from [0-1]
	// clamps otherwise
	unsigned int s = (unsigned int) clip(floatS, 0.0f, 1.0f) * texWidth;
	unsigned int t = (unsigned int) clip(floatT, 0.0f, 1.0f) * texHeight;
	unsigned char red, green, blue, alpha;
	unsigned int sampleColor;
		
	red = texels[s * texWidth + t + 0];
	green = texels[s * texWidth + t + 1];
	blue = texels[s * texWidth + t + 2];
	alpha = texels[s * texWidth + t + 3];

	((unsigned char*)(&sampleColor))[0] = red;
	((unsigned char*)(&sampleColor))[1] = green;
	((unsigned char*)(&sampleColor))[2] = blue;
	((unsigned char*)(&sampleColor))[3] = alpha;

	return sampleColor;
}
