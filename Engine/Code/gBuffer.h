#pragma once
#include "FrameBuffer.h"
#include <glad/glad.h>


struct App;

class GBuffer
{
public:
	GBuffer();
	~GBuffer();

	void Init(App* app);

	void GenerateGBuffer(glm::vec2 displaySize);

public:

	Framebuffer frameBuffer;

	GLuint colorAttachment0Handle;
	GLuint colorAttachment1Handle;
	GLuint colorAttachment2Handle;
	GLuint colorAttachment3Handle;
	GLuint depthAttachmentHandle;

	//Programs
	u32 deferredGeometryProgramIdx;
	u32 deferredLightProgramIdx;
	u32 deferredLightMeshProgramIdx;



	//Location of the texture uniform in the deferred geometry shader
	GLuint deferredGeometry_uTexture;

	//Location of the texture uniforms in the deferred lighting shader
	GLuint deferredLighting_uAlbedo;
	GLuint deferredLighting_uNormal;
	GLuint deferredLighting_uPosition;

};