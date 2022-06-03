#include "engine.h"
#include "ForwardRendering.h"


ForwardRendering::ForwardRendering()
{
}

ForwardRendering::~ForwardRendering()
{
}

void ForwardRendering::Init(App* app)
{
	forwardRenderingProgramIdx = LoadProgram(app, "forwardRendering.glsl", "FORWARD_PASS");
	Program& forwardRenderingProgram = app->programs[forwardRenderingProgramIdx];
	uniformAlbedoTexture = glGetUniformLocation(forwardRenderingProgram.handle, "uTexture");

}

void ForwardRendering::PassUniformsToShader(GLuint albedoTextureHandle)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, albedoTextureHandle);
	glUniform1i(uniformAlbedoTexture, 0);
}
