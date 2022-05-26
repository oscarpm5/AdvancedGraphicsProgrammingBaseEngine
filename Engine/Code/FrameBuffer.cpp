#include "FrameBuffer.h"
#include "platform.h"

Framebuffer::Framebuffer()
{
}

bool Framebuffer::Generate()
{
	Clear();
	glGenFramebuffers(1, &handle);

	return CheckStatus();
}

void Framebuffer::Clear()
{
	if (handle != 0)
	{
		glDeleteBuffers(1, &handle);
	}
}

bool Framebuffer::CheckStatus()
{
	bool ret = true;
	Bind();
	GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (framebufferStatus)
		{
		case GL_FRAMEBUFFER_UNDEFINED: ELOG("GL_FRAMEBUFFER_UNDEFINED"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); break;
		case GL_FRAMEBUFFER_UNSUPPORTED: ELOG("GL_FRAMEBUFFER_UNSUPPORTED"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); break;
		default: ELOG("Unknown framebuffer status error");
		}
		ret = false;
	}
	Release();

	return ret;
}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void Framebuffer::Release()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
