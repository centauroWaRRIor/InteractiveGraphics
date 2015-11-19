#pragma once
#include "framebuffer.h"
#include "tmesh.h"
#include "ppc.h"
#include <vector>
using std::vector;

class HWFrameBuffer :
	public FrameBuffer
{
	// scene's tmeshes and camera need to be registered with this class for rendering
	// because all the opengl drawing needs to be done from the draw method
	vector<TMesh *> tMeshArray;
	PPC *camera;

	bool isGlewInit; // opengl extension wrangler utility

	// programmable pipeline support
	bool isProgrammable; // true for shaders support, false for fixed pipeline functionality
	void loadShaders(void);
	unsigned int fixedPipelineProgram;

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
};

