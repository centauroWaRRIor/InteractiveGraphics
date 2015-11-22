#pragma once
#include <vector>
using std::vector;
#include <utility>
using std::pair;
using std::make_pair;
#include <unordered_map>
using std::unordered_map;
#include <GL/glew.h> // opengl
#include "framebuffer.h"
#include "tmesh.h"
#include "texture.h"
#include "ppc.h"

class HWFrameBuffer :
	public FrameBuffer
{
protected: // need child classes to have access to these members

	// scene's tmeshes, camera and textures need to be registered with this class for rendering
	// because all the opengl drawing needs to be done from the draw method
	vector<TMesh *> tMeshArray;
	vector<pair<Texture *, GLuint>>  texturesInfo;
	PPC *camera;
	// user specfifies which texture go with which Tmesh
	unordered_map<TMesh *, GLuint> tMeshTextureMap;

	// texture handles
	GLuint  *glTextureHandles;

	// used to initialize opengl state once
	bool isGlewInit; // opengl extension wrangler utility

	// texturing support
	void loadTextures(void);

public:

	// because this function does not override the pure virtual draw function
	// from base class, this class is also still abstract. No need to redefine it
	// virtual void draw() = 0; 

	HWFrameBuffer(
		int u0, int v0, // top left coords
		unsigned int _w, unsigned int _h); // resolution 
	virtual ~HWFrameBuffer();

	// because this function does not override the pure virtual functions
	// from base class, this class is also still abstract. No need to redefine it
	//virtual void keyboardHandle(void) = 0;
	//virtual void mouseLeftClickDragHandle(int event) = 0;
	//virtual void mouseRightClickDragHandle(int event) = 0;

	// Register TMeshes, textures and PPC. Also establish tMesh texture corerspondance
	// Reason behind this API is that all openGL drawing needs to be issued from draw function
	void registerTMesh(TMesh *TMeshPtr);
	void registerPPC(PPC *PpcPtr);
	void registerTexture(Texture *texture);
	bool assignTMeshTexture(unsigned int tMeshIndex, unsigned int textureIndex);
};

