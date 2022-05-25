#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "platform.h"

class Camera;

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
	void GenerateSSAONoise(unsigned int noiseSamplesAxis);

	void PassUniformsToShader(GLuint gDepthTextureHandle,GLuint gNormTextureHandle,Camera& cam, App* app);

public:
	std::vector<glm::vec3> kernelSSAO;
	std::vector<glm::vec3> noiseSSAO;

	GLuint uniformNormalTexture;
	GLuint uniformDepthTexture;
	GLuint uniformRandomVecTexture;
	GLuint uniformKernel;
	GLuint uniformViewMat;
	GLuint uniformProjMat;
	GLuint uniformViewportSize;
	GLuint uniformNoiseScale;

	GLuint noiseTextureHandle;
	unsigned int noiseSizeAxis;

	Framebuffer fbSSAO;
};

