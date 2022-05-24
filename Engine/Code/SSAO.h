#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "platform.h"

struct Framebuffer
{
	GLuint handle;
	GLuint colorAttachment0Handle;
	GLuint colorAttachment1Handle;
	GLuint colorAttachment2Handle;
	GLuint colorAttachment3Handle;
	GLuint depthAttachmentHandle;
}; 

struct App;

class SSAO
{
public:
	SSAO();
	~SSAO();

	void Init(App* app);

	u32 LoadSSAOProgram(App* app);

	void GenerateSSAOKernel(unsigned int kernelSize);

public:
	std::vector<glm::vec3> kernelSSAO;

	GLuint uniformDepthTexture;
	GLuint uniformNormalTexture;
	GLuint uniformPositionTexture;
	GLuint uniformKernel;
	GLuint uniformViewMat;
	GLuint uniformProjMat;

	Framebuffer fbSSAO;
};

