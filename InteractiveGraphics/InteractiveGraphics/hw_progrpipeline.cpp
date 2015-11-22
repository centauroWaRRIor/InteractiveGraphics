#include "hw_progrpipeline.h"
#include "hw_shaderprogram.h"
#include <list>
using std::list;
#include <iostream>
using std::cout;
using std::endl;

void HWProgrPipeline::loadShaders(void)
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

HWProgrPipeline::HWProgrPipeline(
	int u0, int v0,
	unsigned int _w, unsigned int _h):
	HWFrameBuffer(u0, v0, _w, _h),
	fixedPipelineProgramNoTexture(nullptr),
	fixedPipelineProgram(nullptr)
{
}

HWProgrPipeline::~HWProgrPipeline()
{
	// delete all the programs
	delete fixedPipelineProgramNoTexture;
	delete fixedPipelineProgram;
}

void HWProgrPipeline::draw()
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
		loadShaders();
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
			glUseProgram(fixedPipelineProgram->getGLProgramHandle());

			if (!tMeshPtr->getIsGLVertexArrayObjectCreated()) {
				tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh 
			}
			tMeshPtr->hwGLVertexArrayObjectDraw();
		}
		else {

			glUseProgram(fixedPipelineProgramNoTexture->getGLProgramHandle());
			if (!tMeshPtr->getIsGLVertexArrayObjectCreated()) {
				tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh 
			}
			tMeshPtr->hwGLVertexArrayObjectDraw();
		}
	}
}

void HWProgrPipeline::keyboardHandle(void)
{

}

void HWProgrPipeline::mouseLeftClickDragHandle(int event)
{

}

void HWProgrPipeline::mouseRightClickDragHandle(int event)
{

}
