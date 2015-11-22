#include "hw_framebuffer.h"
#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <cstdio>
#include <stdexcept>

bool HWFrameBuffer::assignTMeshTexture(unsigned int tMeshIndex, unsigned int textureIndex)
{
	TMesh *tMesh = nullptr;
	try {
		tMesh = tMeshArray.at(tMeshIndex);      // vector::at throws an out-of-range
		texturesInfo.at(textureIndex); // just makes sure this texture object has been registered
		// create map entry in hash map
		tMeshTextureMap[tMesh] = (GLuint) textureIndex; // glTexture handles have not been assigned yet because
		// textures have not yet being loaded with openGL. defer this step until later. Put textureIndex for now
	}
	catch (const std::out_of_range& oor) {
		cout << "Out of Range error: " << oor.what() << '\n';
		return false;
	}
	return true;
}

void HWFrameBuffer::loadTextures(void)
{
	vector<pair<Texture *, GLuint>>::iterator it;
	for (it = texturesInfo.begin(); it != texturesInfo.end(); ++it) {

		GLuint *glTexHandle = &(it->second);
		GLsizei width = it->first->getTexWidth();
		GLsizei height = it->first->getTexHeight();

		// Generate a name for the texture
		glGenTextures(1, glTexHandle);

		// Now bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, *glTexHandle);

		// Specify the amount of storage we want to use for the texture
		glTexStorage2D(
			GL_TEXTURE_2D,  // 2D texture
			8,				// 8 mipmap levels
			GL_RGBA8,		// 8-bit RGBA data
			width, height);  

		// Define some data to upload into the texture
		vector<unsigned char> &dataVector = it->first->getTexelsRef();
		unsigned char * data = &dataVector[0];

		// Assume the texture is already bound to the GL_TEXTURE_2D target
		glTexSubImage2D(GL_TEXTURE_2D,  // 2D texture
			0,				            // Level 0
			0, 0,						// Offset 0, 0
			width, height,				// width x height texels, replace entire image
			GL_RGBA,					// Four channel data
			GL_UNSIGNED_BYTE,			// Floating point data
			data);						// Pointer to data

		glGenerateMipmap(GL_TEXTURE_2D);

		// when not using mimpmaps
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// when using mipmaps
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// the texture wraps over at the edges (repeat)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// GL now has our data
	}

	// now that gl texture handles have been generated update the hashmap with these values
	unordered_map<TMesh *, GLuint>::iterator hashMapIt;
	for (hashMapIt = tMeshTextureMap.begin(); hashMapIt != tMeshTextureMap.end(); ++hashMapIt) {
		GLuint textureIndex = hashMapIt->second;
		GLuint glTextHandle = texturesInfo[textureIndex].second;
		hashMapIt->second = glTextHandle;
	}
}

HWFrameBuffer::HWFrameBuffer(
	int u0, int v0, 
	unsigned int _w, unsigned int _h):
	FrameBuffer(u0, v0, _w, _h),
	isGlewInit(false)
{
}

HWFrameBuffer::~HWFrameBuffer()
{
	// delete all the textures
	vector<pair<Texture *, GLuint>>::iterator it;
	for (it = texturesInfo.begin(); it != texturesInfo.end(); ++it) {
		GLuint *glTexHandle = &(it->second);
		// delete texture from gpu card (video memory)
		glDeleteTextures(1, glTexHandle);
	}
}

void HWFrameBuffer::registerTMesh(TMesh * tMeshPtr)
{
	// only allowed to be done once at init
	if (!isGlewInit) {
		tMeshArray.push_back(tMeshPtr);
	}
}

void HWFrameBuffer::registerPPC(PPC * ppcPtr)
{
	// only allowed to be done once at init
	if (!isGlewInit) {
		camera = ppcPtr;
	}
}

void HWFrameBuffer::registerTexture(Texture * texture)
{
	// only allowed to be done once at init
	if (!isGlewInit) {
		texturesInfo.push_back(make_pair(texture, 0));
	}
}