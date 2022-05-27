#pragma once
#include <vector>
#include <glad/glad.h>

class Framebuffer
{
public:
	Framebuffer();

	bool Generate();
	void Clear();

	bool CheckStatus();//returns true if framebuffer has not errors

	void Bind();
	void Release();
	void AddColorAttachment(GLenum colorAttachment, GLuint texture,GLuint lod);

public:
	GLuint handle;
	GLuint colorAttachment0Handle;
	GLuint colorAttachment1Handle;
	GLuint colorAttachment2Handle;
	GLuint colorAttachment3Handle;
	GLuint depthAttachmentHandle;
};
