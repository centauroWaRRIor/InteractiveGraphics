#pragma once
#include <vector>
using std::vector;
#include <utility>
using std::pair;
using std::make_pair;
#include <unordered_map>
using std::unordered_map;
#include "framebuffer.h"
#include "tmesh.h"
#include "texture.h"
#include "ppc.h"
// this needs to be a forward delcaration due to 
// gl.h compilation order sensibility.
class ShaderProgram;

class HWFrameBuffer :
	public FrameBuffer
{
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

	// programmable pipeline support
	bool isProgrammable; // true for shaders support, false for fixed pipeline functionality
	void loadShaders(void);
	void loadTextures(void);

	// all shader programs are listed here
	ShaderProgram *oldGLSLProgram;
	ShaderProgram *fixedPipelineProgram;

public:
	// function that is always called back by system and never called directly by programmer
	// programmer triggers framebuffer update by calling FrameBuffer::redraw(), which makes
	// system call draw()
	virtual void draw() override;

	HWFrameBuffer(
		int u0, int v0, // top left coords
		unsigned int _w, unsigned int _h, // resolution
		bool isProgrammable); // true for shaders support, false for fixed pipeline functionality
	virtual ~HWFrameBuffer();

	virtual void keyboardHandle(void) override;
	virtual void mouseLeftClickDragHandle(int event) override;
	virtual void mouseRightClickDragHandle(int event) override;

	// Register TMeshes, textures and PPC. Also establish tMesh texture corerspondance
	// Reason behind this API is that all openGL drawing needs to be issued from draw function
	void registerTMesh(TMesh *TMeshPtr);
	void registerPPC(PPC *PpcPtr);
	void registerTexture(Texture *texture);
	bool assignTMeshTexture(unsigned int tMeshIndex, unsigned int textureIndex);
};

