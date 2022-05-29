#pragma once
#include <string>
#include <glad/glad.h>
#include "platform.h"

struct VertexShaderAttribute
{
	u8 location;
	u8 componentCount;
};

struct VertexShaderLayout
{
	std::vector<VertexShaderAttribute> attributes;
}; 

struct App;

class Program
{
public:
	GLuint             handle;
	std::string        filepath;
	std::string        programName;
	u64                lastWriteTimestamp; // What is this for?
	VertexShaderLayout vertexInputLayout;
public:
	Program();
	void Bind();
	void Release();

	GLuint LoadProgram(App* app, const char* filepath, const char* programName);

	GLuint CreateProgramFromSource(String programSource, const char* shaderName);
};