#pragma once
#include <GL/glew.h> // opengl
#include <vector>
using std::vector;
#include <utility>
using std::pair;
using std::make_pair;
#include "framebuffer.h"
#include "tmesh.h"
#include "texture.h"
#include "ppc.h"

class HWFrameBuffer :
	public FrameBuffer
{
	// scene's tmeshes, camera and textures need to be registered with this class for rendering
	// because all the opengl drawing needs to be done from the draw method
	vector<TMesh *> tMeshArray;
	vector<pair<Texture *, GLuint>>  texturesInfo;
	PPC *camera;
	// TODO: Add Hash map so user specfifies which textures go with which Tmeshes (use pointer 
	// as key and use texture name as map value)

	// texture handles
	GLuint  *glTextureHandles;

	// used to initialize opengl state once
	bool isGlewInit; // opengl extension wrangler utility

	// programmable pipeline support
	bool isProgrammable; // true for shaders support, false for fixed pipeline functionality
	void loadShaders(void);
	void loadTextures(void);

	// TODO: Eventually have all possible shader programs listed here
	GLuint fixedPipelineProgram;

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

	// TODO comment on this
	void registerTMesh(TMesh *TMeshPtr);
	void registerPPC(PPC *PpcPtr);
	void registerTexture(Texture *texture);
};

