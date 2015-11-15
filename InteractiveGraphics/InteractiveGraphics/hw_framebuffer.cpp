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
	glClearColor(1.0f, 1.0f, 1.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set view intrinsics
	//float nearPlaneValue = 10.0f;
	//float farPlaneValue = 1000.0f;
	//ppc->SetIntrinsicsHW(nearPlaneValue, farPlaneValue);
	// set view extrinsics
	//ppc->SetExtrinsicsHW();

	// pass all triangle meshes to HW
	//for (int tmi = 0; tmi < tmsN; tmi++) {
	//if (!(tms[tmi] && tms[tmi]->enabled))
	//continue;
	//tms[tmi]->RenderHW();
	//}
}

HWFrameBuffer::HWFrameBuffer(int u0, int v0, unsigned int _w, unsigned int _h):
	FrameBuffer(u0, v0, _w, _h)
{

}


HWFrameBuffer::~HWFrameBuffer()
{
}
