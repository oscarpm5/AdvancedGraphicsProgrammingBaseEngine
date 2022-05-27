#include "engine.h"
#include "gBuffer.h"

GBuffer::GBuffer()
{
}

GBuffer::~GBuffer()
{
}

void GBuffer::GenerateGBuffer(glm::vec2 displaySize)
{

	colorAttachment0Handle = GenerateColTex2DHighPrecision(displaySize);
	colorAttachment1Handle = GenerateColTex2DHighPrecision(displaySize);
	colorAttachment2Handle = GenerateColTex2DHighPrecision(displaySize);
	colorAttachment3Handle = GenerateColTex2DHighPrecision(displaySize);

	depthAttachmentHandle = GenerateDepthTex2D(displaySize);

	frameBuffer.Generate();
	frameBuffer.Bind();
	//TODO make framebuffers generate attachments needed from a parameter
	frameBuffer.AddColorAttachment(GL_COLOR_ATTACHMENT0, colorAttachment0Handle, 0);//Albedo
	frameBuffer.AddColorAttachment(GL_COLOR_ATTACHMENT1, colorAttachment1Handle, 0); //Normal
	frameBuffer.AddColorAttachment(GL_COLOR_ATTACHMENT2, colorAttachment2Handle, 0); //Position
	frameBuffer.AddColorAttachment(GL_COLOR_ATTACHMENT3, colorAttachment3Handle, 0); //Radiance
	frameBuffer.AddDepthAttachment(depthAttachmentHandle);

	frameBuffer.CheckStatus();

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(ARRAY_COUNT(buffers), buffers);

	frameBuffer.Release();
}