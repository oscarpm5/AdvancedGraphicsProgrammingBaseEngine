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
	void PassUniformsToBrightestPixelsShader(glm::vec2 dimensions, GLuint inputTexture,float threshold);

private:
	void GenerateMipmapTexture(GLuint& handle, glm::vec2 dimensions);

public:

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


};