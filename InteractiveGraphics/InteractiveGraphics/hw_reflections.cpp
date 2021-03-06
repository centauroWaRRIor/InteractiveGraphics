#include "hw_reflections.h"
#include "hw_shaderprogram.h"
#include "cubemap.h"
#include "ppc.h"
#include "v3.h"
#include "scene.h" // used to get access to global scene singleton handle for camera dolly purposes
#include <iostream>
using std::cout;
using std::endl;

HWReflections::HWReflections(
	int u0, int v0, 
	unsigned int _w, unsigned int _h):
	HWFrameBuffer(u0, v0, _w, _h),
	fixedPipelineProgramNoTexture(nullptr),
	fixedPipelineProgram(nullptr),
	reflectionShader(nullptr),
	skyboxShader(nullptr),
	envMapData(nullptr),
	reflectorTMeshIndex(0)
{
}

HWReflections::~HWReflections()
{
	// undo render to target setup 
	glDeleteFramebuffers(1, &renderToTextureFramebuffer);
	glDeleteTextures(2, renderedTexture);
	glDeleteRenderbuffers(1, &renderToTextureDepthbuffer);
	// delete environment cubemap texture setup 
	glDeleteTextures(1, &envMap);
	// delete all the programs
	delete fixedPipelineProgramNoTexture;
	delete fixedPipelineProgram;
	delete reflectionShader;
	delete skyboxShader;
}

void HWReflections::loadShaders(void)
{
	list<string> shaderList1;
	shaderList1.push_back("glsl/fixedPipelineNoTexture.vs.glsl");
	shaderList1.push_back("glsl/fixedPipelineNoTexture.fs.glsl");
	list<string> shaderList2;
	shaderList2.push_back("glsl/fixedPipeline.vs.glsl");
	shaderList2.push_back("glsl/fixedPipeline.fs.glsl");
	list<string> shaderList3;
	shaderList3.push_back("glsl/reflectionShader.vs.glsl");
	shaderList3.push_back("glsl/reflectionShader.fs.glsl");
	list<string> shaderList4;
	shaderList4.push_back("glsl/skybox.vs.glsl");
	shaderList4.push_back("glsl/skybox.fs.glsl");


	fixedPipelineProgramNoTexture = new ShaderProgram(shaderList1);
	fixedPipelineProgramNoTexture->createUniform("proj_matrix");
	fixedPipelineProgramNoTexture->createUniform("mv_matrix");

	fixedPipelineProgram = new ShaderProgram(shaderList2);
	fixedPipelineProgram->createUniform("proj_matrix");
	fixedPipelineProgram->createUniform("mv_matrix");

	reflectionShader = new ShaderProgram(shaderList3);
	reflectionShader->createUniform("proj_matrix");
	reflectionShader->createUniform("mv_matrix");
	reflectionShader->createUniform("eyePosition");
	reflectionShader->createUniform("billboardVerts_1");
	reflectionShader->createUniform("billboardTcs_1");
	reflectionShader->createUniform("billboardVerts_2");
	reflectionShader->createUniform("billboardTcs_2");

	skyboxShader = new ShaderProgram(shaderList4);
	skyboxShader->createUniform("view_matrix");
}

void HWReflections::loadTextures(void)
{
	// load the textures registered so far
	HWFrameBuffer::loadTextures();

	// Set up the environment cubemap texture target
	glGenTextures(1, &envMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envMap);
	Texture *texPointer = envMapData->getCubeFace(0);
	unsigned int envTexWidth = texPointer->getTexWidth();
	unsigned int envTexHeight = texPointer->getTexHeight();

	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, envTexWidth, envTexHeight);
	
	vector<unsigned char> *dataVector = texPointer->getTexelsPtr();
	unsigned char * data = dataVector->data();

	// upload environment cube face data 
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 
					0, 
					0, 0, 
					envTexWidth, envTexHeight, 
					GL_RGBA, 
					GL_UNSIGNED_BYTE, 
					data);
	texPointer = envMapData->getCubeFace(1);
	dataVector = texPointer->getTexelsPtr();
	data = dataVector->data();
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		0,
		0, 0,
		envTexWidth, envTexHeight,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data);
	texPointer = envMapData->getCubeFace(4);
	dataVector = texPointer->getTexelsPtr();
	data = dataVector->data();
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		0,
		0, 0,
		envTexWidth, envTexHeight,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data);
	texPointer = envMapData->getCubeFace(5);
	dataVector = texPointer->getTexelsPtr();
	data = dataVector->data();
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		0,
		0, 0,
		envTexWidth, envTexHeight,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data);
	texPointer = envMapData->getCubeFace(3);
	dataVector = texPointer->getTexelsPtr();
	data = dataVector->data();
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		0,
		0, 0,
		envTexWidth, envTexHeight,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data);
	texPointer = envMapData->getCubeFace(2);
	dataVector = texPointer->getTexelsPtr();
	data = dataVector->data();
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		0,
		0, 0,
		envTexWidth, envTexHeight,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Set up render-to-texture texture targets

	// The textures we're going to render to
	glGenTextures(2, renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture[0]);

	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture[1]);

	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

bool HWReflections::initRenderTextureTarget(unsigned int &renderedTextureHandle)
{
	// -----------------------------------------------
	// Render to Texture - set up code is placed here
	// -----------------------------------------------

	static bool initRenderTextureTargetSuccess = false;

	// do once
	if (!initRenderTextureTargetSuccess) {
		// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
		glGenFramebuffers(1, &renderToTextureFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, renderToTextureFramebuffer);

		// The depth buffer
		glGenRenderbuffers(1, &renderToTextureDepthbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderToTextureDepthbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderToTextureDepthbuffer);

		// Set the list of draw buffers.
		glDrawBuffers(1, renderToTextureDrawBuffers); // "1" is the size of DrawBuffers
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, renderToTextureFramebuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderToTextureDepthbuffer);
	}

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (GLuint) renderedTextureHandle, 0);

   // Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return false;
	}
	else {
		initRenderTextureTargetSuccess = true;
		return true;
	}
}

bool HWReflections::createRenderTextureTarget(
	const PPC &ppc, 
	unsigned int tMeshIndex,
	unsigned int &renderedTextureHandle)
{
	// initialize render to texture setup
	if (!initRenderTextureTarget(renderedTextureHandle)) {
		cout << "Error: Failed while settting up render to texture" << endl;
		return false;
	}
	
	// set perspective and model-view matrices
	const float nearPlaneValue = 10.0f;
	const float farPlaneValue = 1000.0f;
	GLfloat perspectiveMatrix[16];
	GLfloat modelviewMatrix[16];
	// use old fixed pipeline (deprecated) glFrustum and gluLookAt
	// to set the perspective matrix and modelview matrix respectively
	
	// set view intrinsics
	ppc.setGLIntrinsics(nearPlaneValue, farPlaneValue);
	// set view extrinsics
	ppc.setGLExtrinsics();

	// need to send matrices as uniforms when using shaders since I'm using GLSL 420 core
	glGetFloatv(GL_PROJECTION_MATRIX, perspectiveMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);

	glUseProgram(fixedPipelineProgramNoTexture->getGLProgramHandle());
	fixedPipelineProgramNoTexture->uploadMatrixUniform("proj_matrix", perspectiveMatrix);
	fixedPipelineProgramNoTexture->uploadMatrixUniform("mv_matrix", modelviewMatrix);

	// Render to our render to texture framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, renderToTextureFramebuffer);
	// clear framebuffer
	glClearColor(0.0f, 0.0f, 1.0, 0.0f); // background alpha is set to zero
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// enable depth test
	glEnable(GL_DEPTH_TEST);

	// render requested triangle meshes through hardware
	TMesh *tMeshPtr = tMeshArray[tMeshIndex];
	if (!tMeshPtr->getIsGLVertexArrayObjectCreated()) {
		tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh
	}
	tMeshPtr->hwGLVertexArrayObjectDraw();

	// Render to the screen after this
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	V3 p1, p2, p3, p4;

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

			p1 = reflectionCentroid + lookAtVector - (a * 100.0f) + (b * 100.0f);
			p2 = reflectionCentroid + lookAtVector + (a * 100.0f) + (b * 100.0f);
			p3 = reflectionCentroid + lookAtVector + (a * 100.0f) - (b * 100.0f);
			p4 = reflectionCentroid + lookAtVector - (a * 100.0f) - (b * 100.0f);
			impostorBillboards[impostorsCreated].createQuadTMesh(p1, p2, p3, p4, false);

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
		loadTextures();
		createImpostorBillboards();
		
		// with billboards created proceed to set unforms for special reflection shaders
		// set unforms uniquely needed by the reflection shader
		glUseProgram(reflectionShader->getGLProgramHandle());
		GLfloat billboardCoords[3 * 4];
		GLfloat billboardTcs[2 * 4];
		// pass billboard imnpostor 1 information
		impostorBillboards[0].copyNVerts(billboardCoords, 4);
		impostorBillboards[0].copyNTexCoords(billboardTcs, 4);
		reflectionShader->uploadVectors3Uniform("billboardVerts_1", billboardCoords, 4);
		reflectionShader->uploadVectors2Uniform("billboardTcs_1", billboardTcs, 4);
		impostorBillboards[1].copyNVerts(billboardCoords, 4);
		impostorBillboards[1].copyNTexCoords(billboardTcs, 4);
		reflectionShader->uploadVectors3Uniform("billboardVerts_2", billboardCoords, 4);
		reflectionShader->uploadVectors2Uniform("billboardTcs_2", billboardTcs, 4);

		// create camera looking at first tMesh to be reflected off of teapot's surface
		PPC tempCamera(85.0f, 1280, 720); // need a wide field of view here
		tempCamera.positionAndOrient(
			tMeshArray[reflectorTMeshIndex]->getCenter(),
			tMeshArray[1]->getCenter(),
			V3(0.0f, 1.0f, 0.0f));
		// render to texture
		if (!createRenderTextureTarget(tempCamera, 1, renderedTexture[0])) {
			cout << "Error: Failed while aquiring render to texture 1" << endl;
			return;
		}

		// create camera looking at second tMesh to be reflected off of teapot's surface
		tempCamera.positionAndOrient(
			tMeshArray[reflectorTMeshIndex]->getCenter(),
			tMeshArray[2]->getCenter(),
			V3(0.0f, 1.0f, 0.0f));
		// render to texture
		if (!createRenderTextureTarget(tempCamera, 2, renderedTexture[1])) {
			cout << "Error: Failed while aquiring render to texture 2" << endl;
			return;
		}

		// even though we hardcoded  the vertices in the shader we still need
		// a VAO so openGL let us draw
		glGenVertexArrays(1, &skybox_vao);
		glBindVertexArray(skybox_vao);

		isGlewInit = true;
	}

	// clear framebuffer
	glClearColor(0.0f, 0.0f, 1.0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// enable depth test
	glEnable(GL_DEPTH_TEST);
	// enables face culling // TODO: Not sure if needed, should be on by default?
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK); // tells OpenGL to cull back faces (the sane default setting)

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

		glUseProgram(reflectionShader->getGLProgramHandle());
		reflectionShader->uploadMatrixUniform("proj_matrix", perspectiveMatrix);
		reflectionShader->uploadMatrixUniform("mv_matrix", modelviewMatrix);
		V3 cameraEye = camera->getEyePoint();
		reflectionShader->uploadVector3Uniform(
			"eyePosition", 
			cameraEye.getX(),
			cameraEye.getY(),
			cameraEye.getZ());
	}

	// draw skybox first
	glUseProgram(skyboxShader->getGLProgramHandle());
	skyboxShader->uploadMatrixUniform("view_matrix", modelviewMatrix);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envMap);
	// even though we hardcoded  the vertices in the shader we still need
	// a VAO so openGL let us draw
	glBindVertexArray(skybox_vao);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
	
	// draw reflector Tmesh through hardware using special shader 
	glUseProgram(reflectionShader->getGLProgramHandle());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture[0]); // use rendered to texture 1
	//glBindTexture(GL_TEXTURE_2D, 1); // upload billboard 1 texture manually for now
	glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, 2); // upload billboard 2 texture manually for now
	glBindTexture(GL_TEXTURE_2D, renderedTexture[1]);  // use rendered to texture 2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envMap); // bind the environment map here
	
	if (!tMeshArray[reflectorTMeshIndex]->getIsGLVertexArrayObjectCreated()) {
		tMeshArray[reflectorTMeshIndex]->createGLVertexArrayObject(); // enables hw support for this TMesh
	}
	tMeshArray[reflectorTMeshIndex]->hwGLVertexArrayObjectDraw();
	
	// render all non reflective triangle meshes through hardware
	glUseProgram(fixedPipelineProgramNoTexture->getGLProgramHandle());
	vector<TMesh *>::const_iterator it;
	
	for (it = tMeshArray.begin(); it != tMeshArray.end(); ++it) {

		if ((it - tMeshArray.begin()) != reflectorTMeshIndex) {
			TMesh *tMeshPtr = *it;
			glUseProgram(fixedPipelineProgramNoTexture->getGLProgramHandle());

			if (!tMeshPtr->getIsGLVertexArrayObjectCreated()) {
				tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh 
			}
			tMeshPtr->hwGLVertexArrayObjectDraw();
		}
	}

	// uncomment here to render all impostor billboards through hardware 
	// (only useful for debug purposes)
	/*
	glUseProgram(fixedPipelineProgram->getGLProgramHandle());
	glActiveTexture(GL_TEXTURE0);

	for (unsigned int i = 0; i < MAX_IMPOSTORS; i++) {

		GLuint glTexHandle;
		if (i == 0)
			glTexHandle = renderedTexture[0]; // use rendered to texture 1
			//glTexHandle = 1; // upload billboard 1 texture manually for now
		else
			glTexHandle = renderedTexture[1]; // use rendered to texture 2
			//glTexHandle = 2; // upload billboard 2 texture manually for now
		
		glBindTexture(GL_TEXTURE_2D, glTexHandle);

		TMesh *tMeshPtr = &impostorBillboards[i];
		if (!tMeshPtr->getIsGLVertexArrayObjectCreated()) {
			tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh 
		}
		tMeshPtr->hwGLVertexArrayObjectDraw();
	}*/
}
#if 0
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
		if (!initRenderTextureTarget())
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
#endif

void HWReflections::setReflectorTMesh(unsigned int index)
{
	// only allowed to be done once at init
	if (!isGlewInit) {
		reflectorTMeshIndex = index;
	}
}

void HWReflections::keyboardHandle(void)
{
	// do nothing with keyboard
}

void HWReflections::mouseLeftClickDragHandle(int event)
{
	static int prevMouseX = 0;
	static int prevMouseY = 0;
	int mouseCurrentX, mouseCurrentY;
	int deltaMouseX, deltaMouseY;

	mouseCurrentX = Fl::event_x();
	mouseCurrentY = Fl::event_y();

	switch (event)
	{
	case FL_PUSH:
		prevMouseX = mouseCurrentX;
		prevMouseY = mouseCurrentY;
		break;
	case FL_DRAG:
		deltaMouseX = -(mouseCurrentX - prevMouseX);
		deltaMouseY = -(mouseCurrentY - prevMouseY);
		prevMouseX = mouseCurrentX;
		prevMouseY = mouseCurrentY;
		scene->setMouseDelta(deltaMouseX, deltaMouseY);
		scene->currentSceneRedraw();
		// reset deltas
		scene->setMouseDelta(0, 0);
		break;
	default: // never used
		break;
	}
}


void HWReflections::mouseRightClickDragHandle(int event)
{
	static int prevMouseX = 0;
	int mouseCurrentX;
	int deltaMouseX;

	mouseCurrentX = Fl::event_x();

	switch (event)
	{
	case FL_PUSH:
		prevMouseX = mouseCurrentX;
		break;
	case FL_DRAG:
		deltaMouseX = -(mouseCurrentX - prevMouseX);
		prevMouseX = mouseCurrentX;
		scene->setMouseRoll(deltaMouseX);
		scene->currentSceneRedraw();
		// reset delta
		scene->setMouseRoll(0);
		break;
	default: // never used
		break;
	}
}

void HWReflections::registerCubeMap(CubeMap * const cubeMap)
{
	// only allowed to be done once at init
	if (!isGlewInit) {
		envMapData = cubeMap;
	}
}
