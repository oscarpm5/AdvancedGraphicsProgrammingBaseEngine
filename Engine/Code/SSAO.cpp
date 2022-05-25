
#include "SSAO.h"
#include "engine.h"
#include<random>

float Lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

SSAO::SSAO()
{
	kernelSSAO.clear();
}

SSAO::~SSAO()
{
}

void SSAO::Init(App* app)
{
	LoadSSAOProgram(app);
	GenerateSSAOKernel(64);
	GenerateSSAONoise(64);

	LoadSSAOBlurProgram(app);

	fbSSAO = GenerateFrameBuffer(app);
}

u32 SSAO::LoadSSAOProgram(App* app)
{
	app->postProcessSSAOProgramIdx = LoadProgram(app, "postProcess.glsl", "SSAO");
	Program& postProcessSSAOProgram = app->programs[app->postProcessSSAOProgramIdx];
	uniformNormalTexture = glGetUniformLocation(postProcessSSAOProgram.handle, "uNormalTexture");
	uniformDepthTexture = glGetUniformLocation(postProcessSSAOProgram.handle, "uDepthTexture");
	uniformRandomVecTexture = glGetUniformLocation(postProcessSSAOProgram.handle, "uRandomVecTexture");

	uniformKernel = glGetUniformLocation(postProcessSSAOProgram.handle, "uKernel");
	uniformViewMat = glGetUniformLocation(postProcessSSAOProgram.handle, "uViewMatrix");
	uniformProjMat = glGetUniformLocation(postProcessSSAOProgram.handle, "uProjMatrix");

	uniformNoiseScale = glGetUniformLocation(postProcessSSAOProgram.handle, "uNoiseScale");
	uniformViewportSize = glGetUniformLocation(postProcessSSAOProgram.handle, "uViewportSize");
	

	return app->postProcessSSAOProgramIdx;
}

u32 SSAO::LoadSSAOBlurProgram(App* app)
{
	app->postProcessSSAOBlurProgramIdx = LoadProgram(app, "postProcess.glsl", "SIMPLE_BLUR");
	Program& postProcessSSAOBlurProgram = app->programs[app->postProcessSSAOBlurProgramIdx];
	uniformBlurInputTexture = glGetUniformLocation(postProcessSSAOBlurProgram.handle, "uBlurInputTexture");
	uniformBlurKernelHalfSize = glGetUniformLocation(postProcessSSAOBlurProgram.handle, "uKernelHalfSize");

	return app->postProcessSSAOBlurProgramIdx;
}

void SSAO::GenerateSSAOKernel(unsigned int kernelSize)
{
	kernelSSAO.clear();

	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	for (unsigned int i = 0; i < kernelSize; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator));

		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / (float)kernelSize;
		scale = Lerp(0.1f, 1.0f, scale * scale);//TODO mix might cause problems, consider creating a custom lerp function
		sample *= scale;
		kernelSSAO.push_back(sample);
	}
}

void SSAO::GenerateSSAONoise(unsigned int noiseSamplesAxis)
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	for (unsigned int i = 0; i < noiseSamplesAxis * noiseSamplesAxis; ++i)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		noiseSSAO.push_back(noise);
	}

	glGenTextures(1, &noiseTextureHandle);
	glBindTexture(GL_TEXTURE_2D, noiseTextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, (GLsizei) noiseSamplesAxis, (GLsizei)noiseSamplesAxis, 0, GL_RGB, GL_FLOAT, &noiseSSAO[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	noiseSizeAxis = noiseSamplesAxis;
	glBindTexture(GL_TEXTURE_2D, 0);
}

void SSAO::PassUniformsToSSAOShader(GLuint gDepthTextureHandle, GLuint gNormTextureHandle, Camera& cam, App* app)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gNormTextureHandle);
	glUniform1i(uniformNormalTexture, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gDepthTextureHandle);
	glUniform1i(uniformDepthTexture, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTextureHandle);
	glUniform1i(uniformRandomVecTexture, 2);

	glUniform3fv(uniformKernel, kernelSSAO.size(), glm::value_ptr(kernelSSAO[0]));
	glUniform2f(uniformNoiseScale, ((float)app->displaySize.x / (float)noiseSizeAxis), ((float)app->displaySize.y / (float)noiseSizeAxis));
	glUniform2f(uniformViewportSize, (float)app->displaySize.x, (float)app->displaySize.y);


	
	glUniformMatrix4fv(uniformViewMat, 1, GL_FALSE, glm::value_ptr(cam.view));
	glUniformMatrix4fv(uniformProjMat, 1, GL_FALSE, glm::value_ptr(cam.projection));

}

void SSAO::PassUniformsToSSAOBlurShader(GLuint textureToBlurHandle, u32 kernelHalfSize)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureToBlurHandle);
	glUniform1i(uniformBlurInputTexture, 0);

	glUniform1i(uniformBlurKernelHalfSize, kernelHalfSize);
}
