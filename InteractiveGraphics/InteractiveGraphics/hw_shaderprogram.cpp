#include "hw_shaderprogram.h"
#include <iostream>
using std::cout;
using std::endl;

GLuint ShaderProgram::load(const char * filename, GLenum shader_type, bool check_errors)
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

GLuint ShaderProgram::link_from_shaders(const GLuint * shaders, int shader_count, bool delete_shaders, bool check_errors)
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
			cout << "GLSL Program linked successfully" << endl;
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

ShaderProgram::ShaderProgram(const list<string>& shadersList) :
	glShaderProgramHandle(0),
	isInitSuccess(false)
{
	GLuint *glShadersHandle = new GLuint[shadersList.size()];
	unsigned int i = 0;

	list<string>::const_iterator listIt;
	isInitSuccess = true;
	for (listIt = shadersList.begin(); listIt != shadersList.end(); ++listIt) {
		if (string::npos != listIt->find(".vs.glsl")) {
			glShadersHandle[i] = load(listIt->c_str(), GL_VERTEX_SHADER);
		}
		else if (string::npos != listIt->find(".fs.glsl")) {
			glShadersHandle[i] = load(listIt->c_str(), GL_FRAGMENT_SHADER);
		}
		else {
			glShadersHandle[i] == 0;
		}

		// check for errors
		if (glShadersHandle[i] == 0) {
			isInitSuccess = false;
			cout << "error compiling shader " << *listIt << endl;
			break;
		}
		++i;
	}

	if (isInitSuccess) {
		glShaderProgramHandle = link_from_shaders(glShadersHandle, shadersList.size(), true);
		if (glShaderProgramHandle == 0) {
			isInitSuccess = false;
			cout << "error linking shaders..." << endl;
		}
	}
	else {
		cout << "error linking shaders..." << endl;
	}
}

ShaderProgram::~ShaderProgram()
{
	if(isInitSuccess)
		glDeleteProgram(glShaderProgramHandle);
}

GLuint ShaderProgram::getGLProgramHandle(void) const
{
	return glShaderProgramHandle;
}

void ShaderProgram::uploadMatrixUniform(const string & uniformName, const GLfloat * const matrix4x4)
{
	GLint uniformLocation = uniformsMap[uniformName];
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, matrix4x4);
}

void ShaderProgram::createUniform(const string & uniformName)
{
	if (isInitSuccess) {

		uniformsMap[uniformName] = glGetUniformLocation(glShaderProgramHandle, uniformName.c_str());
	}
}
