#pragma once
#include "hw_framebuffer.h"

// this needs to be a forward delcaration due to 
// gl.h compilation order sensibility.
class ShaderProgram;

class HWReflections :
	public HWFrameBuffer
{
	// all shader programs are listed here
	ShaderProgram *fixedPipelineProgram;
	ShaderProgram *writeToTextureProgram;

	void loadShaders(void);

public:
	HWReflections(int u0, int v0, // top left coords
		unsigned int _w, unsigned int _h); // resolution
	virtual ~HWReflections();

	// function that is always called back by system and never called directly by programmer
	// programmer triggers framebuffer update by calling FrameBuffer::redraw(), which makes
	// system call draw()
	virtual void draw() override;

	virtual void keyboardHandle(void) override;
	virtual void mouseLeftClickDragHandle(int event) override;
	virtual void mouseRightClickDragHandle(int event) override;
};