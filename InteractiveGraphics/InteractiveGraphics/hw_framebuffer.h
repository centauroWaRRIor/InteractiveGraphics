#pragma once
#include "framebuffer.h"
#include "tmesh.h"
#include "ppc.h"
#include <vector>
using std::vector;

class HWFrameBuffer :
	public FrameBuffer
{
	vector<TMesh *> tMeshArray;
	PPC *camera;
public:
	// function that is always called back by system and never called directly by programmer
	// programmer triggers framebuffer update by calling FrameBuffer::redraw(), which makes
	// system call draw()
	virtual void draw() override;

	HWFrameBuffer(int u0, int v0, unsigned int _w, unsigned int _h); // constructor, top left coords and resolution);
	virtual ~HWFrameBuffer();

	virtual void keyboardHandle(void) override;
	virtual void mouseLeftClickDragHandle(int event) override;
	virtual void mouseRightClickDragHandle(int event) override;

	// TODO comment on this
	void registerTMesh(TMesh *TMeshPtr);
	void registerPPC(PPC *PpcPtr);
};

