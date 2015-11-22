#include "hw_fixedpipeline.h"
#include <iostream>
using std::cout;
using std::endl;

HWFixedPipeline::HWFixedPipeline(
	int u0, int v0, // top left coords
	unsigned int _w, unsigned int _h) :// resolution)
	HWFrameBuffer(u0, v0, _w, _h)
{
}


HWFixedPipeline::~HWFixedPipeline()
{
}

void HWFixedPipeline::draw()
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
		loadTextures();
		isGlewInit = true;
	}

	// clear framebuffer
	glClearColor(0.0f, 0.0f, 1.0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	// set perspective and model-view matrices
	const float nearPlaneValue = 10.0f;
	const float farPlaneValue = 1000.0f;
	// use old fixed pipeline (deprecated) glFrustum and gluLookAt
	// to set the perspective matrix and modelview matrix respectively
	if (camera) {
		// set view intrinsics
		camera->setGLIntrinsics(nearPlaneValue, farPlaneValue);
		// set view extrinsics
		camera->setGLExtrinsics();
	}

	// render all triangle meshes through hardware
	GLuint glTexHandle;
	vector<TMesh *>::const_iterator it;
	for (it = tMeshArray.begin(); it != tMeshArray.end(); ++it) {

		TMesh *tMeshPtr = *it;
		if (tMeshPtr->getIsTexCoordsAvailable()) {

			// loopk up in hash map the texture to bind
			glTexHandle = tMeshTextureMap[tMeshPtr];	// it doesn't matter if entry is not existant
														// and this operation ends up creating a new entry, 
														// it won't be used anyawys
														// bind texture
			glActiveTexture(GL_TEXTURE0); // bind color texture to texture unit 0
			glBindTexture(GL_TEXTURE_2D, glTexHandle);


			glUseProgram(0);
			tMeshPtr->hwGLFixedPiepelineDraw();

		}
		else {


			glUseProgram(0);
			tMeshPtr->hwGLFixedPiepelineDraw();


		}
	}
}

void HWFixedPipeline::keyboardHandle(void)
{
}

void HWFixedPipeline::mouseLeftClickDragHandle(int event)
{
}

void HWFixedPipeline::mouseRightClickDragHandle(int event)
{
}
