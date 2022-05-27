#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "platform.h"
#include "FrameBuffer.h"

class Camera;

struct App;

class SSAO
{
public:
	SSAO();
	~SSAO();

	void Init(App* app, glm::vec2 displaySize);

	void GenerateSSAOBuffer(glm::vec2 displaySize);

	u32 LoadSSAOProgram(App* app);
	u32 LoadSSAOBlurProgram(App* app);

	void GenerateSSAOKernel(unsigned int kernelSize);
	void GenerateSSAONoise(unsigned int noiseSamplesAxis);

	void PassUniformsToSSAOShader(GLuint gDepthTextureHandle, GLuint gNormTextureHandle, Camera& cam, App* app);
	void PassUniformsToSSAOBlurShader( u32 kernelHalfSize);

public:

	Framebuffer frameBuffer;
	GLuint ssaoTextureHandle;
	GLuint ssaoBlurTextureHandle;

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
