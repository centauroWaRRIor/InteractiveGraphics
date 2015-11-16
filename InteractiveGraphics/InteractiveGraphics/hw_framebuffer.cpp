#include <GL/glew.h>
#include <iostream>
using std::cout;
using std::endl;
#include "hw_framebuffer.h"

void HWFrameBuffer::draw()
{
	// initialize opengl extension wrangler utility
	if (!isGlewInit) {
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			cout << "Error: " << glewGetErrorString(err) << endl;
		}
		cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
		isGlewInit = true;
	}

	/*	if (isgpu && cgi == NULL) {
	cgi = new CGInterface();
	cgi->PerSessionInit();
	soi = new ShaderOneInterface();
	soi->PerSessionInit(cgi);
	}
	*/
	// clear framebuffer
	glClearColor(0.0f, 0.0f, 1.0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const float nearPlaneValue = 10.0f;
	const float farPlaneValue = 1000.0f;
	if (camera) {
		// set view intrinsics
		camera->setGLIntrinsics(nearPlaneValue, farPlaneValue);
		// set view extrinsics
		camera->setGLExtrinsics();
	}

	glEnable(GL_DEPTH_TEST);

	// render all triangle meshes with hardware
	vector<TMesh *>::const_iterator it;
	for (it = tMeshArray.begin(); it != tMeshArray.end(); ++it) {
		TMesh *tMeshPtr = *it;
		if(!tMeshPtr->getisHwSupportEnabled())
			tMeshPtr->createGL_VAO(); // enables hw support for this TMesh 
		tMeshPtr->hardwareDraw();
	}
}

HWFrameBuffer::HWFrameBuffer(int u0, int v0, unsigned int _w, unsigned int _h):
	FrameBuffer(u0, v0, _w, _h),
	isGlewInit(false)
{

}


HWFrameBuffer::~HWFrameBuffer()
{
}

void HWFrameBuffer::registerTMesh(TMesh * tMeshPtr)
{
	tMeshArray.push_back(tMeshPtr);
}

void HWFrameBuffer::registerPPC(PPC * ppcPtr)
{
	camera = ppcPtr;
}

void HWFrameBuffer::keyboardHandle(void)
{

}

void HWFrameBuffer::mouseLeftClickDragHandle(int event)
{

}

void HWFrameBuffer::mouseRightClickDragHandle(int event)
{

}