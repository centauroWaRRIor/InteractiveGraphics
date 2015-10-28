#include "texture.h"
#include "lodepng.h"
#include "v3.h"
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

Texture::Texture(const Texture & otherTexture, 
	unsigned int beginS, unsigned int endS, 
	unsigned int beginT, unsigned int endT)
{
	if((beginS > endS) || (beginT > endS))
		cerr << "ERROR: Bad parameters supplied to Texture constructor..." << endl;
	else {

		texWidth = endS - beginS;
		texHeight = endT - beginT;

		unsigned int texelIndex;
		unsigned char red, green, blue, alpha;
		for (unsigned int i = beginT; i < endT; i++) {
			for (unsigned int j = beginS; j < endS; j++) {

				// grab pixel from other texture
				texelIndex = (i * otherTexture.texWidth + j) * 4;
				red = otherTexture.texels[texelIndex + 0];
				green = otherTexture.texels[texelIndex + 1];
				blue = otherTexture.texels[texelIndex + 2];
				alpha = otherTexture.texels[texelIndex + 3];

				// copy into tihs texture
				texels.push_back(red);
				texels.push_back(green);
				texels.push_back(blue);
				texels.push_back(alpha);
			}
		}
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
	float dS = floatS - ((int)floatS);
	float dT = floatT - ((int)floatT);
	// get delta from texel center
	dS -= 0.5f;
	dT -= 0.5f;
	// negative deltas are a special case in which we shift the c0,c1,c2,c3 grid
	// one texel to the left or one texel up
	if (dS < 0.0f) {
		// shift sample grid one texel to the left;
		floatS--;
		// wrap around
		if (floatS < 0.0f) {
			floatS = (float) ((texWidth - 1) + dS);
		}
		// adjust ds relative to new grid
		dS = floatS - ((int)floatS);
	}
	if (dT < 0.0f) {
		// shift sample grid one texel up
		floatT--;
		// wrap around
		if (floatT < 0.0f) {
			floatT = (float)(texHeight - 1 + dT);
		}
		// adjust dt relative to new grid
		dT = floatT - ((int)floatT);
	}

	unsigned int baseS = (unsigned int)(floatS); // always round towards 0
	unsigned int baseT = (unsigned int)(floatT); // always round towards 0

	unsigned char red, green, blue, alpha;

	// compute C0
	unsigned int intS = baseS;
	unsigned int intT = baseT;
	intT = intT = (texHeight - 1) - intT; // t needs to be inverted
	unsigned int texelIndex = (intT * texWidth + intS) * 4;
	red = texels[texelIndex + 0];
	green = texels[texelIndex + 1];
	blue = texels[texelIndex + 2];
	alpha = texels[texelIndex + 3];	
	V3 C0((float)red / 255.0f, (float)green / 255.0f, (float)blue / 255.0f);
	
	// compute C1
	intS = baseS;
	intT = (baseT + 1) % (texHeight);
	intT = intT = (texHeight - 1) - intT; // t needs to be inverted
	texelIndex = (intT * texWidth + intS) * 4;
	red = texels[texelIndex + 0];
	green = texels[texelIndex + 1];
	blue = texels[texelIndex + 2];
	alpha = texels[texelIndex + 3];
	V3 C1((float)red / 255.0f, (float)green / 255.0f, (float)blue / 255.0f);

	// compute C2
	intS = (baseS + 1) % (texWidth);
	intT = (baseT + 1) % (texHeight);
	intT = intT = (texHeight - 1) - intT; // t needs to be inverted
	texelIndex = (intT * texWidth + intS) * 4;
	red = texels[texelIndex + 0];
	green = texels[texelIndex + 1];
	blue = texels[texelIndex + 2];
	alpha = texels[texelIndex + 3];
	V3 C2((float)red / 255.0f, (float)green / 255.0f, (float)blue / 255.0f);

	// compute C3
	intS = (baseS + 1) % (texWidth);
	intT = baseT;
	intT = intT = (texHeight - 1) - intT; // t needs to be inverted
	texelIndex = (intT * texWidth + intS) * 4;
	red = texels[texelIndex + 0];
	green = texels[texelIndex + 1];
	blue = texels[texelIndex + 2];
	alpha = texels[texelIndex + 3];
	V3 C3((float)red / 255.0f, (float)green / 255.0f, (float)blue / 255.0f);
	
	V3 bilinearResult = C0 * ((1 - dS) * (1 - dT)) + C1 * ((1 - dS) * dT)
		+ C2 * (dS * dT) + C3 * (dS * (1 - dT));

	return bilinearResult.getColor();
#if 0
--C0-----C3--
   |     |
   |     |
--C1-----C2--
#endif
}
