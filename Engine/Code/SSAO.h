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
	u32 LoadSSAOBlurProgram(App* app);

	void GenerateSSAOKernel(unsigned int kernelSize);
	void GenerateSSAONoise(unsigned int noiseSamplesAxis);

	void PassUniformsToSSAOShader(GLuint gDepthTextureHandle,GLuint gNormTextureHandle,Camera& cam, App* app);
	void PassUniformsToSSAOBlurShader(GLuint textureToBlurHandle, u32 kernelHalfSize);


public:

	Framebuffer fbSSAO;
	
	//SSAO
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

	//SSAO Blur

	GLuint uniformBlurInputTexture;
	GLuint uniformBlurKernelHalfSize;

};

