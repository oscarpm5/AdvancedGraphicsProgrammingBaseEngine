#include "engine.h"
#include "DeferredRendering.h"

DeferredRendering::DeferredRendering()
{
}

DeferredRendering::~DeferredRendering()
{
}

void DeferredRendering::Init(App* app)
{
	deferredGeometryProgramIdx = LoadProgram(app, "deferredRendering.glsl", "GEOMETRY_PASS");
	Program& deferredGeometryIdx = app->programs[deferredGeometryProgramIdx];
	deferredGeometry_uTexture = glGetUniformLocation(deferredGeometryIdx.handle, "uTexture");

	deferredLightProgramIdx = LoadProgram(app, "deferredRendering.glsl", "LIGHTING_PASS");
	Program& deferredLightingIdx = app->programs[deferredLightProgramIdx];
	deferredLighting_uAlbedo = glGetUniformLocation(deferredLightingIdx.handle, "uAlbedo");
	deferredLighting_uNormal = glGetUniformLocation(deferredLightingIdx.handle, "uNormal");
	deferredLighting_uPosition = glGetUniformLocation(deferredLightingIdx.handle, "uPosition");
	deferredLighting_uSSAO = glGetUniformLocation(deferredLightingIdx.handle, "uSSAO");
	deferredLighting_uSSAOActive = glGetUniformLocation(deferredLightingIdx.handle, "uSSAOActive");

	
	deferredLightMeshProgramIdx = LoadProgram(app, "deferredRendering.glsl", "LIGHT_MESH_PASS");
	Program& deferredLightMeshIdx = app->programs[deferredLightMeshProgramIdx];

	app->deferredRendering.GenerateGBuffer(app->displaySize);
}

void DeferredRendering::GenerateGBuffer(glm::vec2 displaySize)
{
	colorAttachment0Handle = GenerateColTex2DHighPrecision(displaySize);
	colorAttachment1Handle = GenerateColTex2DHighPrecision(displaySize);
	colorAttachment2Handle = GenerateColTex2DHighPrecision(displaySize);
	colorAttachment3Handle = GenerateColTex2DHighPrecision(displaySize);

	depthAttachmentHandle = GenerateDepthTex2D(displaySize);

	frameBuffer.Generate();
	frameBuffer.Bind();
	//TODO make framebuffers generate attachments needed from a parameter
	frameBuffer.AddColorAttachment(GL_COLOR_ATTACHMENT0, colorAttachment0Handle, 0);//Albedo
	frameBuffer.AddColorAttachment(GL_COLOR_ATTACHMENT1, colorAttachment1Handle, 0); //Normal
	frameBuffer.AddColorAttachment(GL_COLOR_ATTACHMENT2, colorAttachment2Handle, 0); //Position
	frameBuffer.AddColorAttachment(GL_COLOR_ATTACHMENT3, colorAttachment3Handle, 0); //Radiance
	frameBuffer.AddDepthAttachment(depthAttachmentHandle);

	frameBuffer.CheckStatus();

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(ARRAY_COUNT(buffers), buffers);

	frameBuffer.Release();
}