#include <iostream>
using std::cout;
using std::endl;
#include <cstdio>
#include <GL/glew.h> // opengl
#include "hw_framebuffer.h"

// static shader utilities to compile and link shader programs
static GLuint load(const char * filename, GLenum shader_type, bool check_errors = true);
static GLuint link_from_shaders(const GLuint * shaders, int shader_count, bool delete_shaders, bool check_errors = true);

void HWFrameBuffer::loadShaders(void)
{
	GLuint shaders[2];

	shaders[0] = load("glsl/fixedPipeline.vs.glsl", GL_VERTEX_SHADER);
	shaders[1] = load("glsl/fixedPipeline.fs.glsl", GL_FRAGMENT_SHADER);

	if (shaders[0] != 0 && shaders[1] != 0) {
		fixedPipelineProgram = link_from_shaders(shaders, 4, true);
	}
	else {
		cout << "error loading shaders..." << endl;
		// ok to commit suicide but got to be extra careful (communicate everybody using this object)
		//cout << "commit suicide now X( " << endl;
		//delete this;
	}
}

void HWFrameBuffer::loadTextures(void)
{
	vector<pair<Texture *, GLuint>>::iterator it;
	for (it = texturesInfo.begin(); it != texturesInfo.end(); ++it) {

		GLuint *glTexHandle = &(it->second);
		GLsizei width = it->first->getTexWidth();
		GLsizei height = it->first->getTexHeight();

		// Generate a name for the texture
		glGenTextures(1, glTexHandle);

		// Now bind it to the context using the GL_TEXTURE_2D binding point
		glBindTexture(GL_TEXTURE_2D, *glTexHandle);

		// Specify the amount of storage we want to use for the texture
		glTexStorage2D(
			GL_TEXTURE_2D,  // 2D texture
			8,				// 8 mipmap levels
			GL_RGBA8,		// 8-bit RGBA data
			width, height);  

		// Define some data to upload into the texture
		vector<unsigned char> &dataVector = it->first->getTexelsRef();
		unsigned char * data = &dataVector[0];

		// Assume the texture is already bound to the GL_TEXTURE_2D target
		glTexSubImage2D(GL_TEXTURE_2D,  // 2D texture
			0,				            // Level 0
			0, 0,						// Offset 0, 0
			width, height,				// width x height texels, replace entire image
			GL_RGBA,					// Four channel data
			GL_UNSIGNED_BYTE,			// Floating point data
			data);						// Pointer to data

		glGenerateMipmap(GL_TEXTURE_2D);

		// when not using mimpmaps
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// when using mipmaps
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// the texture wraps over at the edges (repeat)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// GL now has our data
	}
}

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
		if(isProgrammable)
			loadShaders();
		loadTextures();
		isGlewInit = true;
	}

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

	// bind texture
	// TODO: make it so that this is looked up in a hash table
	// that maps tmesh to texture binding
	glBindTexture(GL_TEXTURE_2D, texturesInfo[0].second);

	// bind shader program if applicable
	if(isProgrammable)
		glUseProgram((GLuint) fixedPipelineProgram);
	else
		glUseProgram(0);

	// render all triangle meshes with hardware
	vector<TMesh *>::const_iterator it;
	for (it = tMeshArray.begin(); it != tMeshArray.end(); ++it) {
		TMesh *tMeshPtr = *it;
		if (isProgrammable) {
			if (!tMeshPtr->getIsGLVertexArrayObjectCreated())
				tMeshPtr->createGLVertexArrayObject(); // enables hw support for this TMesh 
			tMeshPtr->hwGLVertexArrayObjectDraw();
		}
		else {
			tMeshPtr->hwGLFixedPiepelineDraw();
		}
	}
}

HWFrameBuffer::HWFrameBuffer(
	int u0, int v0, 
	unsigned int _w, unsigned int _h,
	bool isP):
	FrameBuffer(u0, v0, _w, _h),
	isProgrammable(isP),
	isGlewInit(false),
	fixedPipelineProgram(0)
{
}


HWFrameBuffer::~HWFrameBuffer()
{
	glDeleteProgram((GLuint) fixedPipelineProgram);
	// TODO: Delete all the textures
	//glDeleteTextures(2, tex_object);
}

void HWFrameBuffer::registerTMesh(TMesh * tMeshPtr)
{
	// only allowed to be done once at init
	if (!isGlewInit) {
		tMeshArray.push_back(tMeshPtr);
	}
}

void HWFrameBuffer::registerPPC(PPC * ppcPtr)
{
	// only allowed to be done once at init
	if (!isGlewInit) {
		camera = ppcPtr;
	}
}

void HWFrameBuffer::registerTexture(Texture * texture)
{
	// only allowed to be done once at init
	if (!isGlewInit) {
		texturesInfo.push_back(make_pair(texture, 0));
	}
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

static GLuint load(const char * filename, GLenum shader_type, bool check_errors)
{
	GLuint result = 0;
	FILE * fp;
	size_t filesize;
	char * data;
	errno_t err;
	err = fopen_s(&fp, filename, "rb");

	if (err == 0) {

		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		data = new char[filesize + 1];

		if (data) {

			fread(data, 1, filesize, fp);
			data[filesize] = '\0';
			fclose(fp);
			result = glCreateShader(shader_type);

			if (result) {

				glShaderSource(result, 1, &data, NULL);
				glCompileShader(result);
				delete[] data;

				if (check_errors)
				{
					GLint status = 0;
					glGetShaderiv(result, GL_COMPILE_STATUS, &status);

					if (!status)
					{
						char buffer[4096];
						glGetShaderInfoLog(result, 4096, NULL, buffer);
						cout << filename << ": " << endl << buffer << endl;
						glDeleteShader(result);
						result = 0;
					}
					else
						cout << filename << ": Compiled successfully!" << endl;
				}
			}
		}
	}
	return result;
}

static GLuint link_from_shaders(const GLuint * shaders, int shader_count, bool delete_shaders, bool check_errors)
{
	int i;
	GLuint program;
	program = glCreateProgram();

	for (i = 0; i < shader_count; i++)
	{
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);

	if (check_errors)
	{
		GLint status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);

		if (!status)
		{
			char buffer[4096];
			glGetProgramInfoLog(program, 4096, NULL, buffer);
			cout << buffer << endl;
			glDeleteProgram(program);
			program = 0;
		}
		else
			cout << "GLSL Program linked  successfully" << endl;
	}

	if (delete_shaders)
	{
		for (i = 0; i < shader_count; i++)
		{
			glDeleteShader(shaders[i]);
		}
	}

	return program;
}