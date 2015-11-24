#include "hw_reflections.h"
#include "hw_shaderprogram.h"
#include "ppc.h"
#include "v3.h"
#include <iostream>
using std::cout;
using std::endl;

HWReflections::HWReflections(
	int u0, int v0, 
	unsigned int _w, unsigned int _h):
	HWFrameBuffer(u0, v0, _w, _h),
	fixedPipelineProgramNoTexture(nullptr),
	fixedPipelineProgram(nullptr),
	reflectorTMeshIndex(0)
{
}

HWReflections::~HWReflections()
{
	// undo render to target setup 
	glDeleteFramebuffers(1, &renderToTextureFramebuffer);
	glDeleteTextures(1, &renderedTexture);
	glDeleteRenderbuffers(1, &renderToTextureDepthbuffer);
	// delete all the programs
	delete fixedPipelineProgramNoTexture;
	delete fixedPipelineProgram;
}

void HWReflections::loadShaders(void)
{
	list<string> shaderList1;
	shaderList1.push_back("glsl/fixedPipelineNoTexture.vs.glsl");
	shaderList1.push_back("glsl/fixedPipelineNoTexture.fs.glsl");
	list<string> shaderList2;
	shaderList2.push_back("glsl/fixedPipeline.vs.glsl");
	shaderList2.push_back("glsl/fixedPipeline.fs.glsl");

	fixedPipelineProgramNoTexture = new ShaderProgram(shaderList1);
	fixedPipelineProgramNoTexture->createUniform("proj_matrix");
	fixedPipelineProgramNoTexture->createUniform("mv_matrix");

	fixedPipelineProgram = new ShaderProgram(shaderList2);
	fixedPipelineProgram->createUniform("proj_matrix");
	fixedPipelineProgram->createUniform("mv_matrix");
}

bool HWReflections::createRenderTextureTarget(void)
{
	// -----------------------------------------------
	// Render to Texture - set up code is placed here
	// -----------------------------------------------

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	glGenFramebuffers(1, &renderToTextureFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, renderToTextureFramebuffer);

	// The texture we're going to render to
	glGenTextures(1, &renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// The depth buffer
	glGenRenderbuffers(1, &renderToTextureDepthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderToTextureDepthbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderToTextureDepthbuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	// Set the list of draw buffers.
	glDrawBuffers(1, renderToTextureDrawBuffers); // "1" is the size of DrawBuffers

   // Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	else
		return true;
}

bool HWReflections::createImpostorBillboards(void)
{
	unsigned int impostorsCreated = 0;
	unsigned int tMeshIndex = 0;

	V3 reflectionCentroid = tMeshArray[reflectorTMeshIndex]->getCenter();
	V3 tMeshCentroid;
	V3 a, b;
	V3 lookAtVector;
	V3 bottomLeftCorner, upperRightCorner;

	PPC ppc(55.0, 1280, 720);

	while (impostorsCreated < MAX_IMPOSTORS && tMeshIndex < tMeshArray.size()) {
		// make sure we are not creating an impostor billboard for the teapot
		if (tMeshIndex != reflectorTMeshIndex) {

			tMeshCentroid = tMeshArray[tMeshIndex]->getCenter();
			ppc.positionAndOrient(reflectionCentroid, tMeshCentroid, V3(0.0f, 1.0f, 0.0f));

			// use camera plane axis directions to create billboard without too much math
			a = ppc.getLowerCaseA();
			b = ppc.getLowerCaseB();

			lookAtVector = tMeshCentroid - reflectionCentroid;
			bottomLeftCorner = reflectionCentroid + lookAtVector - (a * 100.0f) + (b * 100.0f);
			upperRightCorner = reflectionCentroid + lookAtVector + (a * 100.0f) - (b * 100.0f);
			impostorBillboards[impostorsCreated].createQuadTMesh(bottomLeftCorner, upperRightCorner, false);

			++impostorsCreated;
		}
		++tMeshIndex;
	}
	return false;
}

void HWReflections::draw()
{
	// initialize opengl extension wrangler utility
	if (!isGlewInit) {
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			cout << "Error: " << glewGetErrorString(err) << endl;
			return;
		}
		cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
		loadShaders();
		//loadTextures();
		//createImpostorBillboards();
		isGlewInit = true;
	}

	glUseProgram(fixedPipelineProgramNoTexture->getGLProgramHandle());

	vector<TMesh *>::const_iterator it;
	// render all triangle meshes through hardware
	for (it = tMeshArray.begin(); it != tMeshArray.end(); ++it) {

		TMesh *tMeshPtr = *it;
		glUseProgram(fixedPipelineProgramNoTexture->getGLProgramHandle());

		if (!tMeshPtr->getIsGLVertexArrayObjectCreated()) {
			tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh 
		}
		tMeshPtr->hwGLVertexArrayObjectDraw();
	}
	/*
	// draw reflector Tmesh through hardware
	if (!tMeshArray[reflectorTMeshIndex]->getIsGLVertexArrayObjectCreated()) {
		tMeshArray[reflectorTMeshIndex]->createGLVertexArrayObject(); // enables hw support for this TMesh 
	}
	tMeshArray[reflectorTMeshIndex]->hwGLVertexArrayObjectDraw();

	// render all impostor billboards through hardware
	vector<TMesh *>::const_iterator it;
	for (it = tMeshArray.begin(); it != tMeshArray.end(); ++it) {

		TMesh *tMeshPtr = *it;
		if (!tMeshPtr->getIsGLVertexArrayObjectCreated()) {
			tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh 
		}
		tMeshPtr->hwGLVertexArrayObjectDraw();
	}
	*/
}

void HWReflections::drawRenderToTexture(void)
{
	// initialize opengl extension wrangler utility
	if (!isGlewInit) {
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			cout << "Error: " << glewGetErrorString(err) << endl;
			return;
		}
		cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
		loadShaders();
		//loadTextures();
		if (!createRenderTextureTarget())
			cout << "Error: Failed while settting up render to texture" << endl;
		else
			cout << "Render to texture set up successfully!" << endl;
		isGlewInit = true;
	}

	// set perspective and model-view matrices
	const float nearPlaneValue = 10.0f;
	const float farPlaneValue = 1000.0f;
	GLfloat perspectiveMatrix[16];
	GLfloat modelviewMatrix[16];
	// use old fixed pipeline (deprecated) glFrustum and gluLookAt
	// to set the perspective matrix and modelview matrix respectively
	if (camera) {
		// set view intrinsics
		camera->setGLIntrinsics(nearPlaneValue, farPlaneValue);
		// set view extrinsics
		camera->setGLExtrinsics();

		// need to send matrices as uniforms when using shaders since I'm using GLSL 420 core
		glGetFloatv(GL_PROJECTION_MATRIX, perspectiveMatrix);
		glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);

		glUseProgram(fixedPipelineProgram->getGLProgramHandle());
		fixedPipelineProgram->uploadMatrixUniform("proj_matrix", perspectiveMatrix);
		fixedPipelineProgram->uploadMatrixUniform("mv_matrix", modelviewMatrix);

		glUseProgram(fixedPipelineProgramNoTexture->getGLProgramHandle());
		fixedPipelineProgramNoTexture->uploadMatrixUniform("proj_matrix", perspectiveMatrix);
		fixedPipelineProgramNoTexture->uploadMatrixUniform("mv_matrix", modelviewMatrix);
	}

	// Render to our render to texture framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, renderToTextureFramebuffer);
	// clear framebuffer
	glClearColor(0.0f, 0.0f, 1.0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// enable depth test
	glEnable(GL_DEPTH_TEST);

	glUseProgram(fixedPipelineProgramNoTexture->getGLProgramHandle());

	vector<TMesh *>::const_iterator it;

	// render all triangle meshes through hardware
	for (it = tMeshArray.begin(); it != tMeshArray.end(); ++it) {

		TMesh *tMeshPtr = *it;
		if (!tMeshPtr->getIsGLVertexArrayObjectCreated()) {
			tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh 
		}
		tMeshPtr->hwGLVertexArrayObjectDraw();
	}

	// Render to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// clear framebuffer
	glClearColor(0.0f, 0.0f, 1.0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// enable depth test
	glEnable(GL_DEPTH_TEST);

	// use previously rendered texture as texture here
	glActiveTexture(GL_TEXTURE0); // bind color texture to texture unit 0
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// render all triangle meshes through hardware
	for (it = tMeshArray.begin(); it != tMeshArray.end(); ++it) {

		TMesh *tMeshPtr = *it;

		if (tMeshPtr->getIsTexCoordsAvailable())
			glUseProgram(fixedPipelineProgram->getGLProgramHandle());
		else
			glUseProgram(fixedPipelineProgramNoTexture->getGLProgramHandle());

		if (!tMeshPtr->getIsGLVertexArrayObjectCreated()) {
			tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh 
		}
		tMeshPtr->hwGLVertexArrayObjectDraw();
	}
}

void HWReflections::setReflectorTMesh(unsigned int index)
{
	// only allowed to be done once at init
	if (!isGlewInit) {
		reflectorTMeshIndex = index;
	}
}

void HWReflections::keyboardHandle(void)
{
}

void HWReflections::mouseLeftClickDragHandle(int event)
{
}

void HWReflections::mouseRightClickDragHandle(int event)
{
}
