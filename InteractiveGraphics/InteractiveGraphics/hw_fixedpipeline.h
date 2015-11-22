#pragma once
#include "hw_framebuffer.h"
class HWFixedPipeline :
	public HWFrameBuffer
{
public:
	HWFixedPipeline(
		int u0, int v0, // top left coords
		unsigned int _w, unsigned int _h); // resolution
	virtual ~HWFixedPipeline();

	// function that is always called back by system and never called directly by programmer
	// programmer triggers framebuffer update by calling FrameBuffer::redraw(), which makes
	// system call draw()
	virtual void draw() override;

	virtual void keyboardHandle(void) override;
	virtual void mouseLeftClickDragHandle(int event) override;
	virtual void mouseRightClickDragHandle(int event) override;
};

