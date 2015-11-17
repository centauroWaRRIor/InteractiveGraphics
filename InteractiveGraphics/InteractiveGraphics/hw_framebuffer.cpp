#include <GL/glew.h>
#include <iostream>
using std::cout;
using std::endl;
#include <cstdio>
#include "hw_framebuffer.h"

// static shader utilities to compile and link shader programs
static GLuint load(const char * filename, GLenum shader_type = GL_FRAGMENT_SHADER, bool check_errors = true);
static GLuint link_from_shaders(const GLuint * shaders, int shader_count, bool delete_shaders, bool check_errors = true);

void HWFrameBuffer::loadShaders(void)
{
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
						cout << "filename: " << endl << buffer << endl;
						glDeleteShader(result);
					}
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
			return 0;
		}
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