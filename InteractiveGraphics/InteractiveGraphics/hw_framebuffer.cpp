#include "hw_framebuffer.h"

void HWFrameBuffer::draw()
{
	/*	if (isgpu && cgi == NULL) {
	cgi = new CGInterface();
	cgi->PerSessionInit();
	soi = new ShaderOneInterface();
	soi->PerSessionInit(cgi);
	}
	*/
	// clear framebuffer
	glClearColor(0.0f, 0.0f, 0.0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const float nearPlaneValue = 10.0f;
	const float farPlaneValue = 1000.0f;
	if(camera) {
		// set view intrinsics
		camera->setGLIntrinsics(nearPlaneValue, farPlaneValue);
		// set view extrinsics
		camera->setGLExtrinsics();
	}

	// render all triangle meshes with hardware
	vector<TMesh *>::const_iterator it;
	for (it = tMeshArray.begin(); it != tMeshArray.end(); ++it) {
		TMesh *tMeshPtr = *it;
		tMeshPtr->hardwareDraw();
	}
}

HWFrameBuffer::HWFrameBuffer(int u0, int v0, unsigned int _w, unsigned int _h):
	FrameBuffer(u0, v0, _w, _h)
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