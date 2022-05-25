
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

	fbSSAO = GenerateFrameBuffer(app);
}

u32 SSAO::LoadSSAOProgram(App* app)
{
	app->postProcessSSAOProgramIdx = LoadProgram(app, "postProcess.glsl", "SSAO");
	Program& postProcessSSAOProgram = app->programs[app->postProcessSSAOProgramIdx];
	uniformNormalTexture = glGetUniformLocation(postProcessSSAOProgram.handle, "uNormalTexture");
	uniformPositionTexture = glGetUniformLocation(postProcessSSAOProgram.handle, "uPosTexture");

	uniformKernel = glGetUniformLocation(postProcessSSAOProgram.handle, "uKernel");
	uniformViewMat = glGetUniformLocation(postProcessSSAOProgram.handle, "uViewMatrix");
	uniformProjMat = glGetUniformLocation(postProcessSSAOProgram.handle, "uProjMatrix"); 

	return app->postProcessSSAOProgramIdx;
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

void SSAO::PassUniformsToShader(GLuint gPosTextureHandle, GLuint gNormTextureHandle, Camera& cam)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gNormTextureHandle);
	glUniform1i(uniformNormalTexture, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gPosTextureHandle);
	glUniform1i(uniformPositionTexture, 0);

	glUniform3fv(uniformKernel, kernelSSAO.size(), glm::value_ptr(kernelSSAO[0]));
	glUniformMatrix4fv(uniformViewMat, 1, GL_FALSE, glm::value_ptr(cam.view));
	glUniformMatrix4fv(uniformProjMat, 1, GL_FALSE, glm::value_ptr(cam.projection));

}
