#pragma once
#include "hw_framebuffer.h" // opengl
#include <string>
using std::string;
#include <unordered_map>
using std::unordered_map;
#include <list>
using std::list;

class ShaderProgram
{
	GLuint glShaderProgramHandle;
	unordered_map<string, GLint> uniformsMap;
	bool isInitSuccess;

	// shader utilities to compile and link shader programs
	GLuint load(
		const char * filename, 
		GLenum shader_type, 
		bool check_errors = true);

	GLuint link_from_shaders(
		const GLuint * shaders, 
		int shader_count, 
		bool delete_shaders, 
		bool check_errors = true);

public:
	ShaderProgram(const list<string> &shadersList);
	~ShaderProgram();

	GLuint getGLProgramHandle(void) const;

	void uploadMatrixUniform(string uniformName, const GLfloat * const matrix4x4);
	void uploadVector3Uniform(string uniformName, GLfloat v1, GLfloat v2, GLfloat v3 );
	void uploadVectors3Uniform(string uniformName, const GLfloat * const vectors, GLsizei vectorsCount);
	void createUniform(string uniformName);
};

