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

	void Init(App* app);
	
	void PassUniformsToSSAOShader(GLuint gDepthTextureHandle, GLuint gNormTextureHandle, Camera& cam, App* app);
	void PassUniformsToSSAOBlurShader();

	void SetEffectActive(bool active);
	bool GetActive();

private:
	void GenerateSSAOBuffer(glm::vec2 displaySize);
	u32 LoadSSAOProgram(App* app);
	u32 LoadSSAOBlurProgram(App* app);

	void GenerateSSAOKernel(unsigned int kernelSize);
	void GenerateSSAONoise(unsigned int noiseSamplesAxis);

private:
	bool activeEffect = true;
public:

	bool blurActive = true;


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
	GLuint uniformRadius;
	GLuint uniformBias;


	GLuint noiseTextureHandle;
	unsigned int noiseSizeAxis;
	int blurSize = 2;
	float radius=1.0f;
	float bias=0.0f;
	//SSAO Blur

	GLuint uniformBlurInputTexture;
	GLuint uniformBlurKernelHalfSize;

	//Program Idx
	u32 ssaoProgramIdx;
	u32 blurProgramIdx;

};
