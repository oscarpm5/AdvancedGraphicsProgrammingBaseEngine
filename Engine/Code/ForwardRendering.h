#pragma once

struct App;


class ForwardRendering
{
public:
	ForwardRendering();
	~ForwardRendering();

	void Init(App* app);
	void PassUniformsToShader(GLuint albedoTextureHandle);

public:

	//Programs
	u32 forwardRenderingProgramIdx;

	//uniforms
	GLuint uniformAlbedoTexture;

};
