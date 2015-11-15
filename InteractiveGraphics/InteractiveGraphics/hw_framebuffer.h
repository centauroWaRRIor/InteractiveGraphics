#pragma once
#include "framebuffer.h"
class HWFrameBuffer :
	public FrameBuffer
{
public:
	// function that is always called back by system and never called directly by programmer
	// programmer triggers framebuffer update by calling FrameBuffer::redraw(), which makes
	// system call draw()
	virtual void draw() override;

	HWFrameBuffer(int u0, int v0, unsigned int _w, unsigned int _h); // constructor, top left coords and resolution);
	virtual ~HWFrameBuffer();
};

