#pragma once
#include "FrameBuffer.h"
#include <glad/glad.h>

class GBuffer
{
public:
	GBuffer();
	~GBuffer();

	void GenerateGBuffer(glm::vec2 displaySize);

public:

	Framebuffer frameBuffer;

	GLuint colorAttachment0Handle;
	GLuint colorAttachment1Handle;
	GLuint colorAttachment2Handle;
	GLuint colorAttachment3Handle;
	GLuint depthAttachmentHandle;

};