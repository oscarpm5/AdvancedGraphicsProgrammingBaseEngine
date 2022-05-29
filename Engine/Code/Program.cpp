#include "engine.h"
#include "Program.h"

GLuint Program::CreateProgramFromSource(String programSource, const char* shaderName)
{
	GLchar  infoLogBuffer[1024] = {};
	GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
	GLsizei infoLogSize;
	GLint   success;

	char versionString[] = "#version 430\n";
	char shaderNameDefine[128];
	sprintf(shaderNameDefine, "#define %s\n", shaderName);
	char vertexShaderDefine[] = "#define VERTEX\n";
	char fragmentShaderDefine[] = "#define FRAGMENT\n";

	const GLchar* vertexShaderSource[] = {
		versionString,
		shaderNameDefine,
		vertexShaderDefine,
		programSource.str
	};
	const GLint vertexShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(vertexShaderDefine),
		(GLint)programSource.len
	};
	const GLchar* fragmentShaderSource[] = {
		versionString,
		shaderNameDefine,
		fragmentShaderDefine,
		programSource.str
	};
	const GLint fragmentShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(fragmentShaderDefine),
		(GLint)programSource.len
	};

	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
	glCompileShader(vshader);
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
	glCompileShader(fshader);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint programHandle = glCreateProgram();
	glAttachShader(programHandle, vshader);
	glAttachShader(programHandle, fshader);
	glLinkProgram(programHandle);
	glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	glUseProgram(0);

	glDetachShader(programHandle, vshader);
	glDetachShader(programHandle, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	return programHandle;
}

Program::Program()
{
}

void Program::Bind()
{
	glUseProgram(handle);
}

void Program::Release()
{
	glUseProgram(0);
}

u32 Program::LoadProgram(App* app, const char* filepath, const char* programName)
{
	String programSource = ReadTextFile(filepath);

	Program program = {};
	program.handle = CreateProgramFromSource(programSource, programName);
	filepath = filepath;
	programName = programName;
	lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);

	GLint attributeCount;

	glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

	for (u32 i = 0; i < attributeCount; ++i)
	{
		GLchar attribName[256];
		GLsizei attribNameLength;
		GLint attribSize;
		GLenum attribType;
		glGetActiveAttrib(handle, i, ARRAY_COUNT(attribName), &attribNameLength, &attribSize, &attribType, attribName);

		GLint attribLocation = glGetAttribLocation(handle, attribName);

		VertexShaderAttribute newInputAttrib;

		u8 attribRealSize = 0;

		switch (attribType)
		{
		case GL_FLOAT:
		{
			attribRealSize = 1;
		}
		break;
		case GL_FLOAT_VEC2:
		{
			attribRealSize = 2;
		}
		break;
		case GL_FLOAT_VEC3:
		{
			attribRealSize = 3;
		}
		break;
		case GL_FLOAT_VEC4:
		{
			attribRealSize = 4;
		}
		break;
		}

		assert(attribRealSize != 0);//AttribSize shouldn't be 0, if it is an input is missing in the input attribute switch

		newInputAttrib.componentCount = attribSize * attribRealSize;
		newInputAttrib.location = attribLocation;

		vertexInputLayout.attributes.push_back(newInputAttrib);
	}

	app->programs.push_back(program);

	return app->programs.size() - 1;
}
