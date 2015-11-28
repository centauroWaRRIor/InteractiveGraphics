#pragma once
#include "hw_framebuffer.h"

class TMesh; // used for storing billboards

// this needs to be a forward delcaration due to 
// gl.h compilation order sensibility.
class ShaderProgram;

class HWReflections :
	public HWFrameBuffer
{
	// all shader programs are listed here
	ShaderProgram *fixedPipelineProgram;
	ShaderProgram *fixedPipelineProgramNoTexture;
	ShaderProgram *reflectionShader;

	// TODO initialize in the constructor
	GLuint renderToTextureFramebuffer = 0;
	GLuint renderedTexture;
	GLuint renderToTextureDepthbuffer;
	GLenum renderToTextureDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };

	// TODO comment
	unsigned int reflectorTMeshIndex;

	// only a maximum of two impostor billboards supported at the time
	static const unsigned int MAX_IMPOSTORS = 2;
	TMesh impostorBillboards[MAX_IMPOSTORS];

	void loadShaders(void);
	bool initRenderTextureTarget(void);
	bool createRenderTextureTarget(const PPC &ppc, unsigned int tMeshIndex);
	bool createImpostorBillboards(void);
public:
	HWReflections(int u0, int v0, // top left coords
		unsigned int _w, unsigned int _h); // resolution
	virtual ~HWReflections();

	// function that is always called back by system and never called directly by programmer
	// programmer triggers framebuffer update by calling FrameBuffer::redraw(), which makes
	// system call draw()
	virtual void draw() override;

	// TODO: Document
	void setReflectorTMesh(unsigned int index);

	virtual void keyboardHandle(void) override;
	virtual void mouseLeftClickDragHandle(int event) override;
	virtual void mouseRightClickDragHandle(int event) override;
};