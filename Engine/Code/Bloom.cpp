#include "Bloom.h"
#include "engine.h"

Bloom::Bloom()
{
}

void Bloom::Init(App* app)
{
#define MIPMAP_BASE_LEVEL 0
#define MIPMAP_MAX_LEVEL 4

	blitBrightestPixelsProgramIdx = LoadProgram(app, "bloom.glsl", "BRIGHTEST_PIXELS");
	Program& blitBrightestPixelsProgram = app->programs[blitBrightestPixelsProgramIdx];
	uniformColorTexture = glGetUniformLocation(blitBrightestPixelsProgram.handle, "uColorTexture");
	uniformThreshold = glGetUniformLocation(blitBrightestPixelsProgram.handle, "uThreshold");

	blurProgramIdx = LoadProgram(app, "bloom.glsl", "BLUR");
	Program& blitBlurProgram = app->programs[blurProgramIdx];
	uniformBlurColorTexture = glGetUniformLocation(blitBlurProgram.handle, "uColorTexture");
	uniformDirection = glGetUniformLocation(blitBlurProgram.handle, "uDirection");
	uniformLOD = glGetUniformLocation(blitBlurProgram.handle, "uLOD");

	bloomProgramIdx = LoadProgram(app, "bloom.glsl", "BLOOM");
	Program& blitBloomProgram = app->programs[bloomProgramIdx];
	uniformBloomColorTexture = glGetUniformLocation(blitBloomProgram.handle, "uColorTexture");
	uniformMaxLOD = glGetUniformLocation(blitBloomProgram.handle, "uMaxLOD");
	uniformLODIntensity = glGetUniformLocation(blitBloomProgram.handle, "uLODIntensity");

	GenerateMipmapTexture(rtBright, app->displaySize);
	GenerateMipmapTexture(rtBlurH, app->displaySize);

	fboBloom1.Generate();
	fboBloom2.Generate();
	fboBloom3.Generate();
	fboBloom4.Generate();
	fboBloom5.Generate();

	fboBloom1.Bind();
	fboBloom1.AddColorAttachment(GL_COLOR_ATTACHMENT0, rtBright, 0);
	fboBloom1.AddColorAttachment(GL_COLOR_ATTACHMENT1, rtBlurH, 0);
	fboBloom1.CheckStatus();
	fboBloom1.Release();

	fboBloom2.Bind();
	fboBloom2.AddColorAttachment(GL_COLOR_ATTACHMENT0, rtBright, 1);
	fboBloom2.AddColorAttachment(GL_COLOR_ATTACHMENT1, rtBlurH, 1);
	fboBloom2.CheckStatus();
	fboBloom2.Release();

	fboBloom3.Bind();
	fboBloom3.AddColorAttachment(GL_COLOR_ATTACHMENT0, rtBright, 2);
	fboBloom3.AddColorAttachment(GL_COLOR_ATTACHMENT1, rtBlurH, 2);
	fboBloom3.CheckStatus();
	fboBloom3.Release();

	fboBloom4.Bind();
	fboBloom4.AddColorAttachment(GL_COLOR_ATTACHMENT0, rtBright, 3);
	fboBloom4.AddColorAttachment(GL_COLOR_ATTACHMENT1, rtBlurH, 3);
	fboBloom4.CheckStatus();
	fboBloom4.Release();

	fboBloom5.Bind();
	fboBloom5.AddColorAttachment(GL_COLOR_ATTACHMENT0, rtBright, 4);
	fboBloom5.AddColorAttachment(GL_COLOR_ATTACHMENT1, rtBlurH, 4);
	fboBloom5.CheckStatus();
	fboBloom5.Release();
}

void Bloom::PassUniformsToBrightestPixelsShader(glm::vec2 dimensions, GLuint inputTexture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glUniform1i(uniformColorTexture, 0);

	glUniform1f(uniformThreshold, threshold);
}

void Bloom::PassUniformsToBlurShader(GLuint inputTexture, GLint inputLOD, const glm::vec2& direction)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputTexture);
	glUniform1i(uniformBlurColorTexture, 0);
	glUniform1i(uniformLOD, inputLOD);
	glUniform2f(uniformDirection, direction.x, direction.y);
}

void Bloom::PassUniformsToCombineShader(GLuint inputTexture, GLint maxLOD)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputTexture);
	glUniform1i(uniformBloomColorTexture, 0);
	glUniform1i(uniformMaxLOD, maxLOD);
	glUniform1fv(uniformLODIntensity, 5, intensityLODs);
}

void Bloom::SetEffectActive(bool active)
{
	activeEffect = active;
}

bool Bloom::GetActive()
{
	return activeEffect;
}

void Bloom::GenerateMipmapTexture(GLuint& handle, glm::vec2 dimensions)
{
	//Generate textures with mipmap
	if (handle != 0)
		glDeleteTextures(1, &handle);

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, MIPMAP_BASE_LEVEL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MIPMAP_MAX_LEVEL);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, dimensions.x / 2.0, dimensions.y / 2.0, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, dimensions.x / 4.0, dimensions.y / 4.0, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA16F, dimensions.x / 8.0, dimensions.y / 8.0, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA16F, dimensions.x / 16.0, dimensions.y / 16.0, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA16F, dimensions.x / 32.0, dimensions.y / 32.0, 0, GL_RGBA, GL_FLOAT, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}