#pragma once
#include "platform.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "FrameBuffer.h"


struct App;

class Bloom
{
public:

	Bloom();
	void Init(App* app);
	void PassUniformsToBrightestPixelsShader(glm::vec2 dimensions, GLuint inputTexture);
	void PassUniformsToBlurShader(GLuint inputTexture, GLint inputLOD, const glm::vec2& direction);
	void PassUniformsToCombineShader(GLuint inputTexture, GLint maxLOD);

	void SetEffectActive(bool active);
	bool GetActive();

private:
	void GenerateMipmapTexture(GLuint& handle, glm::vec2 dimensions);
private:
	bool activeEffect = true;

public:

	float intensityLODs[5] = { 0.2,0.2,0.2,0.2,0.2 };
	float threshold=0.9f;
	//Program Indices
	u32 blitBrightestPixelsProgramIdx;
	u32 blurProgramIdx;
	u32 bloomProgramIdx;

	//Textures
	GLuint rtBright; //brightest Pixels & vertical blur
	GLuint rtBlurH; //Horizontal Blur

	//FBOs
	Framebuffer fboBloom1;
	Framebuffer fboBloom2;
	Framebuffer fboBloom3;
	Framebuffer fboBloom4;
	Framebuffer fboBloom5;

	//Uniforms------------------------------------
	//BrightPixels
	GLuint uniformColorTexture;
	GLuint uniformThreshold;
	//Blur
	GLuint uniformBlurColorTexture;
	GLuint uniformDirection;
	GLuint uniformLOD;
	//Bloom
	GLuint uniformBloomColorTexture;
	GLuint uniformMaxLOD;
	GLuint uniformLODIntensity;


};