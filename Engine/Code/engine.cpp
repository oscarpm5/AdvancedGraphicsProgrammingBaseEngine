//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "buffer_management.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

#define BINDING(b) b

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
	String programSource = ReadTextFile(filepath);

	Program program = {};
	program.handle = program.CreateProgramFromSource(programSource, programName);
	program.filepath = filepath;
	program.programName = programName;
	program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);

	GLint attributeCount;

	glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

	for (u32 i = 0; i < attributeCount; ++i)
	{
		GLchar attribName[256];
		GLsizei attribNameLength;
		GLint attribSize;
		GLenum attribType;
		glGetActiveAttrib(program.handle, i, ARRAY_COUNT(attribName), &attribNameLength, &attribSize, &attribType, attribName);

		GLint attribLocation = glGetAttribLocation(program.handle, attribName);

		VertexShaderAttribute newInputAttrib;

		u8 attribRealSize = 0;

		switch (attribType)
		{
		case GL_FLOAT:
		{
			attribRealSize = 1;
		}
		break;
		case GL_FLOAT_VEC2:
		{
			attribRealSize = 2;
		}
		break;
		case GL_FLOAT_VEC3:
		{
			attribRealSize = 3;
		}
		break;
		case GL_FLOAT_VEC4:
		{
			attribRealSize = 4;
		}
		break;
		}

		assert(attribRealSize != 0);//AttribSize shouldn't be 0, if it is an input is missing in the input attribute switch

		newInputAttrib.componentCount = attribSize * attribRealSize;
		newInputAttrib.location = attribLocation;

		program.vertexInputLayout.attributes.push_back(newInputAttrib);
	}

	app->programs.push_back(program);

	return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
	Image img = {};
	stbi_set_flip_vertically_on_load(true);
	img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
	if (img.pixels)
	{
		img.stride = img.size.x * img.nchannels;
	}
	else
	{
		ELOG("Could not open file %s", filename);
	}
	return img;
}

void FreeImage(Image image)
{
	stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
	GLenum internalFormat = GL_RGB8;
	GLenum dataFormat = GL_RGB;
	GLenum dataType = GL_UNSIGNED_BYTE;

	switch (image.nchannels)
	{
	case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
	case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
	default: ELOG("LoadTexture2D() - Unsupported number of channels");
	}

	GLuint texHandle;
	glGenTextures(1, &texHandle);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
	for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
		if (app->textures[texIdx].filepath == filepath)
			return texIdx;

	Image image = LoadImage(filepath);

	if (image.pixels)
	{
		Texture tex = {};
		tex.handle = CreateTexture2DFromImage(image);
		tex.filepath = filepath;

		u32 texIdx = app->textures.size();
		app->textures.push_back(tex);

		FreeImage(image);
		return texIdx;
	}
	else
	{
		return UINT32_MAX;
	}
}

void Init(App* app)
{
	app->displayMode = 3;//TODO change this int into an enum?

	if (GLVersion.major > 4 || (GLVersion.major = 4 && GLVersion.minor >= 3))
	{
		glDebugMessageCallback(OnGlError, app);
	}
	//TODO consider putting opengl info variables retrieval into a method
	app->glInfo.version = MakeString((const char*)glGetString(GL_VERSION));
	app->glInfo.vendor = MakeString((const char*)glGetString(GL_VENDOR));
	app->glInfo.renderer = MakeString((const char*)glGetString(GL_RENDERER));
	app->glInfo.glslVersion = MakeString((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

	GLint nExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
	for (int i = 0; i < nExtensions; ++i)
	{
		app->glInfo.extensions.push_back(MakeString((const char*)glGetStringi(GL_EXTENSIONS, GLuint(i))));
	}
	app->showGlInfoWindow = true;

	// TODO: Initialize your resources here!
	// - vertex buffers
	// - element/index buffers
	// - vaos
	// - programs (and retrieve uniform indices)
	// - textures

	//Program
	app->displayToScreenProgramIdx = LoadProgram(app, "displayToScreen.glsl", "RECT_TO_SCREEN");
	Program& displayToScreenProgram = app->programs[app->displayToScreenProgramIdx];
	app->programUniformTexture = glGetUniformLocation(displayToScreenProgram.handle, "uTexture");
	app->programUniformIsDepth = glGetUniformLocation(displayToScreenProgram.handle, "isDepth");


	//Texture
	app->diceTexIdx = LoadTexture2D(app, "dice.png");
	app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
	app->blackTexIdx = LoadTexture2D(app, "color_black.png");
	app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
	app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

	app->model = LoadModel(app, "Patrick/Patrick.obj");
	app->modelRoom = LoadModel(app, "Room/Room #1.obj");

	float aspectRatio = (float)app->displaySize.x / (float)app->displaySize.y;
	app->cam = Camera(60.0f, aspectRatio, 0.1f, 1000.0f);
	app->cam.offsetDirection = glm::normalize(vec3(5.0f, 7.5f, 10.0f));
	app->cam.yaw = 45.0f;
	app->cam.pitch = -30.0f;
	app->cam.zoom = 15.0f;
	app->cam.target = vec3(0.0f, 0.0f, 0.0f);
	app->cam.CamUpdateRotation();
	app->cam.UpdateMatrices();

	CreateSphere(app);
	CreateQuad(app);

	app->quadModel = CreateModelFromMesh(app, app->screenQuad);
	app->sphereModel = CreateModelFromMesh(app, app->sphereMesh);

	//Only need to do this once
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);

	//For each buffer that needs to be created

	app->lBuffer = CreateConstantBuffer(app->maxUniformBufferSize);
	app->cBuffer = CreateConstantBuffer(app->maxUniformBufferSize);

	u32 currentEntity = AddEntity(app, "Patrick", app->model);//Add a patrick
	app->entities[currentEntity].position = vec3(-1.5f, 0.5f, 1.0f);
	app->entities[currentEntity].scale = vec3(0.6f);
	app->entities[currentEntity].rotation = vec3(0.0f, 180.0f, 0.0f);

	currentEntity = AddEntity(app, "Patrick", app->model);//Add a patrick
	app->entities[currentEntity].position = vec3(1.5f, 0.5f, 1.0f);
	app->entities[currentEntity].scale = vec3(0.6f);
	app->entities[currentEntity].rotation = vec3(0.0f, 180.0f, 0.0f);

	currentEntity = AddEntity(app, "Patrick", app->model);//Add a patrick
	app->entities[currentEntity].position = vec3(3.0f, 0.5f, -1.0f);
	app->entities[currentEntity].scale = vec3(0.6f);
	app->entities[currentEntity].rotation = vec3(0.0f, -90.0f, 0.0f);

	currentEntity = AddEntity(app, "Room", app->modelRoom);//Add a room model
	app->entities[currentEntity].position = vec3(0.0f, -2.0f, -0.0f);

	//Lights
	CreateDirectionalLight(app, vec3(1.0, 0.75, 0.5), vec3(1, -1, -1));
	CreateDirectionalLight(app, vec3(0.5, 0.5, 1.0), vec3(-1.0, -0.5, 0.5));

	for (int i = 0; i < 14; i++)
	{
		vec3 position = glm::normalize(vec3(RandSph(), RandSph(), RandSph())) * 6.0f * (1.0f - (1.0f - abs(RandSph())) * (1.0f - abs(RandSph())));
		vec3 color = vec3(abs(RandSph()), abs(RandSph()), abs(RandSph()));
		CreatePointLight(app, color * vec3(3.0), position); //TODO add more
	}

	app->renderLightMeshes = true;
	app->deferredMode = true;

	app->forwardRendering.Init(app);
	app->deferredRendering.Init(app);
	app->ssaoEffect.Init(app);
	app->bloomEffect.Init(app);
}

float RandSph()
{
	return (((float)rand() / RAND_MAX) - 0.5f) * 2.0f;
}

void Gui(App* app)
{
	//ImGui::ShowDemoWindow();

	ImGui::Begin("Info");
	ImGui::TextWrapped("FPS: %f", 1.0f / app->deltaTime);
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::TextWrapped("CONTROLS:");
	ImGui::Indent();
	ImGui::TextWrapped("WASDQE			-> Move");
	ImGui::TextWrapped("SPACE (HOLD)	-> Movement Multiplier");
	ImGui::TextWrapped("MOUSE SCROLL WHEEL	-> Zoom on target");
	ImGui::TextWrapped("MOUSE RIGHT BUTTON (HOLD)	-> Orbit (Move mouse)");
	ImGui::TextWrapped("F	-> Center camera to world origin");
	ImGui::Unindent();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::TextWrapped("If WASDQE controls do not work, check that the main viewport is the active window (click on it) instead of having the focus on the ImGui windows");
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::TextWrapped("If the ROOM model or Patrick model do not show properly, check that they are included in their own folder inside the Working Directory. (E.g. WorkingDir/Patrick/Patrick.obj).");

	ImGui::End();

	if (app->showGlInfoWindow)
	{
		ImGui::Begin("GL Info");
		ImGui::Text("Open GL Version: %s", app->glInfo.version.str);
		ImGui::Text("Open GL Renderer: %s", app->glInfo.renderer.str);
		ImGui::Text("Open GL Vendor: %s", app->glInfo.vendor.str);
		ImGui::Text("Open GL GLSL Version: %s", app->glInfo.glslVersion.str);
		ImGui::Text("Open GL Extensions: (%d)", app->glInfo.extensions.size());
		for (int i = 0; i < app->glInfo.extensions.size(); ++i)
		{
			ImGui::Indent();
			ImGui::Text("(%d) \t - %s", i, app->glInfo.extensions.at(i).str);
			ImGui::Unindent();
		}
		ImGui::End();
	}


	static int current_render_mode = 1;
	if (ImGui::Combo("Render Mode", &current_render_mode, "Forward\0Deferred\0\0"))
	{
		app->deferredMode = (bool)current_render_mode;
	}


	if (app->deferredMode)
	{
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Indent();
		ImGui::Checkbox("Display Light Debug Meshes", &app->renderLightMeshes);

		static int current_draw_mode = 3;
		if (ImGui::Combo("Display Render Target", &current_draw_mode, "Albedo\0Normals\0Position\0Final\0Depth\0SSAO\0SSAOBlured\0Bloom\0\0"))
		{
			app->displayMode = current_draw_mode;
		}
		ImGui::Unindent();
	}

	{
		ImGui::Begin("Inspector");
		glm::vec3 pos = app->cam.GetCamPos();
		ImGui::Text("Camera Position: %f(X), %f(Y),%f(Z)", pos.x, pos.y, pos.z);
		/*float newPos[3] = { pos.x,pos.y,pos.z };
		if (ImGui::DragFloat3("Cam Position", newPos))
		{
			glm::vec3 newPos = glm::vec3(newPos[0], newPos[1], newPos[2]);
			app->cam.target= newPos-app->cam.offsetDirection*app->cam.zoom;
			app->cam.UpdateMatrices();
		}*/

		ImGui::Separator();
		if (ImGui::Button("Add Patrick"))
		{
			AddEntity(app, "Patrick", app->model);
		}
		if (ImGui::Button("Add Room"))
		{
			AddEntity(app, "Room", app->modelRoom);
		}

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_Leaf;
		if (ImGui::CollapsingHeader("Entities", flags))
		{
			ImGui::Indent();
			static int selected = 0;
			for (int n = 0; n < app->entities.size(); ++n)
			{
				char buf[32];
				sprintf(buf, "%s##%d", app->entities[n].name.str, app->entities[n].id);
				if (ImGui::Selectable(buf, selected == n))
					selected = n;
			}
			ImGui::Unindent();

			char buf[128];
			sprintf(buf, "Transform: %s##%d", app->entities[selected].name.str, app->entities[selected].id);
			if (ImGui::CollapsingHeader(buf, flags))
			{
				bool hasToUpdateMatrix = false;
				float entityPos[3] = { app->entities[selected].position.x, app->entities[selected].position.y, app->entities[selected].position.z };
				if (ImGui::DragFloat3("Position", entityPos))
				{
					app->entities[selected].position.x = entityPos[0];
					app->entities[selected].position.y = entityPos[1];
					app->entities[selected].position.z = entityPos[2];
					hasToUpdateMatrix = true;
				}
				float entityRot[3] = { app->entities[selected].rotation.x, app->entities[selected].rotation.y, app->entities[selected].rotation.z };
				if (ImGui::DragFloat3("Rotation", entityRot))
				{
					app->entities[selected].rotation.x = entityRot[0];
					app->entities[selected].rotation.y = entityRot[1];
					app->entities[selected].rotation.z = entityRot[2];
					hasToUpdateMatrix = true;
				}
				float entityScale[3] = { app->entities[selected].scale.x, app->entities[selected].scale.y, app->entities[selected].scale.z };
				if (ImGui::DragFloat3("Scale", entityScale))
				{
					app->entities[selected].scale.x = entityScale[0];
					app->entities[selected].scale.y = entityScale[1];
					app->entities[selected].scale.z = entityScale[2];
					hasToUpdateMatrix = true;
				}

				if (hasToUpdateMatrix)
					app->entities[selected].UpdateWorldMatrix();
			}

			if (ImGui::Button("Delete Entity"))
			{
				app->entities.erase(app->entities.begin() + selected);
			}

			if (app->deferredMode)
			{

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				//SSAO
				bool active = app->ssaoEffect.GetActive();
				if (ImGui::Checkbox("SSAO", &active))
				{
					app->ssaoEffect.SetEffectActive(active);
				}
				if (active)
				{
					ImGui::Indent();
					ImGui::Checkbox("Blur", &app->ssaoEffect.blurActive);
					if (app->ssaoEffect.blurActive)
					{
						ImGui::DragInt("BlurAmmount", &app->ssaoEffect.blurSize, 1, 1, 30);
					}
					ImGui::Spacing();
					ImGui::DragFloat("KernelRadius", &app->ssaoEffect.radius, 0.1f, 0.0f, 100.0f);
					ImGui::DragFloat("SSAO Bias", &app->ssaoEffect.bias, 0.1f, -app->ssaoEffect.radius, app->ssaoEffect.radius);

					ImGui::Unindent();
				}
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				//BLOOM
				active = app->bloomEffect.GetActive();
				if (ImGui::Checkbox("Bloom", &active))
				{
					app->bloomEffect.SetEffectActive(active);
				}
				if (active)
				{
					ImGui::Indent();
					ImGui::DragFloat("Threshold", &app->bloomEffect.threshold, 0.05, 0.0);

					int arraySize = sizeof(app->bloomEffect.intensityLODs) / sizeof(app->bloomEffect.intensityLODs[0]);
					for (int i = 0; i < arraySize; ++i)
					{
						char buffer[50];
						sprintf(buffer, "LOD %i", i);
						ImGui::SliderFloat(&buffer[0], &app->bloomEffect.intensityLODs[i], 0.0f, 1.0f);
					}
					ImGui::Unindent();
				}
			}
		}

		ImGui::End();
	}
}

void Update(App* app)
{
	// You can handle app->input keyboard/mouse here

	//Toggle glInfoWindow
	if (app->input.keys[Key::K_I] == ButtonState::BUTTON_PRESS)
	{
		app->showGlInfoWindow = !app->showGlInfoWindow;
	}

	HandleCameraMove(app);

	// You can handle app->input keyboard/mouse here

	for (u64 i = 0; i < app->programs.size(); ++i)
	{
		Program& program = app->programs[i];
		u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());
		if (currentTimestamp > program.lastWriteTimestamp)
		{
			glDeleteProgram(program.handle);
			String programSource = ReadTextFile(program.filepath.c_str());
			const char* programName = program.programName.c_str();
			program.handle = program.CreateProgramFromSource(programSource, programName);
			program.lastWriteTimestamp = currentTimestamp;
		}
	}

	//Push data into the buffer (ordered according to the uniform block)

	//Global buffer
	MapBuffer(app->cBuffer, GL_WRITE_ONLY);

	app->globalParamsoffset = app->cBuffer.head;
	PushVec3(app->cBuffer, app->cam.GetCamPos());
	PushUInt(app->cBuffer, app->lights.size());

	for (u32 i = 0; i < app->lights.size(); ++i)
	{
		AlignHead(app->cBuffer, sizeof(glm::vec4));
		Light& light = app->lights[i];

		PushUInt(app->cBuffer, light.type);
		PushVec3(app->cBuffer, light.color);
		PushVec3(app->cBuffer, light.direction);
		PushVec3(app->cBuffer, light.position);
	}

	app->globalParamsSize = app->cBuffer.head - app->globalParamsoffset;

	UnmapBuffer(app->cBuffer);

	//Local buffer
	MapBuffer(app->lBuffer, GL_WRITE_ONLY);

	for (u32 i = 0; i < app->entities.size(); ++i) //Normal entities
	{
		AlignHead(app->lBuffer, app->uniformBlockAlignment);
		Entity& entity = app->entities[i];

		entity.UpdateWorldMatrix();

		entity.localParamsOffset = app->lBuffer.head;
		PushMat4(app->lBuffer, entity.worldMatrix);
		PushMat4(app->lBuffer, app->cam.projection * app->cam.view * entity.worldMatrix);
		entity.localParamsSize = app->lBuffer.head - entity.localParamsOffset;
	}

	for (u32 i = 0; i < app->lightEntities.size(); ++i) //Light Mesh entities
	{
		AlignHead(app->lBuffer, app->uniformBlockAlignment);
		Entity& entity = app->lightEntities[i];

		entity.localParamsOffset = app->lBuffer.head;
		PushMat4(app->lBuffer, entity.worldMatrix);
		PushMat4(app->lBuffer, app->cam.projection * app->cam.view * entity.worldMatrix);
		PushVec3(app->lBuffer, app->lights[i].color);
		entity.localParamsSize = app->lBuffer.head - entity.localParamsOffset;
	}

	UnmapBuffer(app->lBuffer);
}

void HandleCameraMove(App* app)
{
	float speed = 10.0f;
	float speedMult = 3.0f;

	
	float horizontalMove = 0.0f;
	float verticalMove = 0.0f;
	float forwardMove = 0.0f;

	vec3 forwardVec = glm::normalize(-app->cam.offsetDirection);
	vec3 rightVec = glm::normalize(glm::cross(forwardVec, vec3(0.0f, 1.0f, 0.0f)));
	vec3 vericalVec = glm::normalize(glm::cross(rightVec, forwardVec));
	
	if (app->input.keys[Key::K_SPACE] == ButtonState::BUTTON_PRESSED)
	{
		speed *= speedMult;
	}

	if (app->input.mouseButtons[MouseButton::RIGHT] == ButtonState::BUTTON_PRESSED)
	{
		
		//orbit
		glm::vec2 delta = app->input.mouseDelta;
		f32 rotSpeed = 12;
		glm::vec2 rot = delta * rotSpeed*app->deltaTime;
		app->cam.pitch += -rot.y;
		app->cam.yaw += -rot.x;	

		app->cam.CamUpdateRotation();
	}



	if (app->input.keys[Key::K_D] == ButtonState::BUTTON_PRESSED)
	{
		horizontalMove += 1;
	}
	if (app->input.keys[Key::K_A] == ButtonState::BUTTON_PRESSED)
	{
		horizontalMove += -1;
	}

	if (app->input.keys[Key::K_E] == ButtonState::BUTTON_PRESSED)
	{
		verticalMove += 1;
	}
	if (app->input.keys[Key::K_Q] == ButtonState::BUTTON_PRESSED)
	{
		verticalMove += -1;
	}

	if (app->input.keys[Key::K_W] == ButtonState::BUTTON_PRESSED)
	{
		forwardMove += 1;
	}
	if (app->input.keys[Key::K_S] == ButtonState::BUTTON_PRESSED)
	{
		forwardMove += -1;
	}



	if (app->input.mouseScrollDelta.y != 0.0f)
	{
		float zoomLvl = glm::max(1.0f, app->cam.zoom - app->input.mouseScrollDelta.y);
		app->cam.zoom = zoomLvl;
	}
	

	app->cam.target += forwardVec * forwardMove * speed * app->deltaTime;
	app->cam.target += vericalVec * verticalMove * speed * app->deltaTime;
	app->cam.target += rightVec * horizontalMove * speed * app->deltaTime;



	if (app->input.keys[Key::K_F] == ButtonState::BUTTON_PRESS)
	{
		app->cam.target = glm::vec3(0.0);
		app->cam.zoom = 15.0f;
	}



	app->cam.UpdateMatrices();
}

void Camera::CamUpdateRotation()
{
	pitch = glm::max(-89.9f, glm::min(89.9f, pitch));

	glm::mat4 rotCamP = glm::rotate(glm::radians(pitch), glm::vec3(1.0, 0.0, 0.0));
	glm::mat4 rotCamY = glm::rotate(glm::radians(yaw), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 combinedRot = rotCamY * rotCamP;
	//glm::quat rotQX =  glm::angleAxis(glm::radians(-rot.y), glm::vec3(1.0, 0.0, 0.0));
	//glm::quat rotQY = glm::angleAxis(glm::radians(-rot.x), glm::vec3(0.0, 1.0, 0.0));
	//glm::quat combinedRot = rotQX *rotQY;
	//app->cam.offsetDirection = combinedRot * app->cam.offsetDirection;
	offsetDirection = combinedRot * glm::vec4(0.0, 0.0, 1.0, 1.0);
}

void Render(App* app)
{
	if (app->deferredMode)
		DeferredRender(app);
	else
		ForwardRender(app);
}

void ForwardRender(App* app)
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - clear the framebuffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	Program& forwardRenderingProgram = app->programs[app->forwardRendering.forwardRenderingProgramIdx];
	glUseProgram(forwardRenderingProgram.handle);
	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cBuffer.handle, app->globalParamsoffset, app->globalParamsSize); //Only once as is used for each object


	for (int n = 0; n < app->entities.size(); ++n)
	{

		Model& model = app->models[app->entities[n].modelIndex];
		Mesh& mesh = app->meshes[model.meshIdx];

		for (u32 i = 0; i < mesh.submeshes.size(); ++i)
		{
			GLuint vao = FindVAO(mesh, i, forwardRenderingProgram);
			glBindVertexArray(vao);

			u32 submeshMaterialIdx = model.materialIdx[i];
			Material& submeshMaterial = app->materials[submeshMaterialIdx];

			if (submeshMaterial.albedoTextureIdx != UINT32_MAX)
			{
				app->forwardRendering.PassUniformsToShader(app->textures[submeshMaterial.albedoTextureIdx].handle);
			}
			else
			{
				app->forwardRendering.PassUniformsToShader(app->textures[app->whiteTexIdx].handle);
			}


			glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->lBuffer.handle, app->entities[n].localParamsOffset, app->entities[n].localParamsSize);


			Submesh& submesh = mesh.submeshes[i];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}

	}

	glBindVertexArray(0);
	glUseProgram(0);
}

void DeferredRender(App* app)
{
	GeometryPass(app);

	if (app->ssaoEffect.GetActive())
	{
		SSAOPass(app);
		SSAOBlurPass(app);
	}
	LightPass(app);
	if (app->bloomEffect.GetActive())
	{
		BloomPass(app);
	}


	if (app->renderLightMeshes == true)
	{
		RenderLightMeshes(app);
	}

	RenderTextureToScreen(app, GetDisplayTexture(app), app->displayMode == 4 ? true : false);
}

GLuint GetDisplayTexture(App* app)
{
	switch (app->displayMode)
	{
	case 0:
	{
		return app->deferredRendering.colorAttachment0Handle;
	}
	break;
	case 1:
	{
		return app->deferredRendering.colorAttachment1Handle;
	}
	break;
	case 2:
	{
		return app->deferredRendering.colorAttachment2Handle;
	}
	break;
	case 3:
	{
		return app->deferredRendering.colorAttachment3Handle;
	}
	break;
	case 4:
	{
		return app->deferredRendering.depthAttachmentHandle;
	}
	break;
	case 5:
	{
		return app->ssaoEffect.ssaoTextureHandle;
	}
	break;
	case 6:
	{
		return app->ssaoEffect.ssaoBlurTextureHandle;
	}
	break;
	case 7:
	{
		return app->bloomEffect.rtBright;
	}
	break;
	}

	ELOG("No display mode selected!");
	return app->deferredRendering.colorAttachment3Handle;
}

void GeometryPass(App* app)
{
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Render on this framebuffer render targets
	app->deferredRendering.frameBuffer.Bind();

	//Select on which render targets to draw
	GLuint drawbuffers[] = {
		GL_COLOR_ATTACHMENT0, //Albedo
		GL_COLOR_ATTACHMENT1, //Normals
		GL_COLOR_ATTACHMENT2 //Position
	};
	glDrawBuffers(ARRAY_COUNT(drawbuffers), drawbuffers);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - clear the framebuffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Program& deferredgeometryProgram = app->programs[app->deferredRendering.deferredGeometryProgramIdx];
	deferredgeometryProgram.Bind();

	for (int n = 0; n < app->entities.size(); ++n)
	{
		Model& model = app->models[app->entities[n].modelIndex];
		Mesh& mesh = app->meshes[model.meshIdx];

		for (u32 i = 0; i < mesh.submeshes.size(); ++i)
		{
			GLuint vao = FindVAO(mesh, i, deferredgeometryProgram);
			glBindVertexArray(vao);

			u32 submeshMaterialIdx = model.materialIdx[i];

			glUniform1i(app->deferredRendering.deferredGeometry_uTexture, 0);
			Material& submeshMaterial = app->materials[submeshMaterialIdx];
			glActiveTexture(GL_TEXTURE0);

			if (submeshMaterial.albedoTextureIdx != UINT32_MAX)
			{
				glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, app->textures[app->whiteTexIdx].handle);
			}

			glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->lBuffer.handle, app->entities[n].localParamsOffset, app->entities[n].localParamsSize);

			Submesh& submesh = mesh.submeshes[i];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}
	}

	glBindVertexArray(0);
	deferredgeometryProgram.Release();
	app->deferredRendering.frameBuffer.Release();
}

void SSAOPass(App* app)
{
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	app->ssaoEffect.frameBuffer.Bind();

	//Select on which render targets to draw
	GLuint drawbuffers[] = {
		GL_COLOR_ATTACHMENT0,
	};
	glDrawBuffers(ARRAY_COUNT(drawbuffers), drawbuffers);

	glViewport(0, 0, app->displaySize.x, app->displaySize.y);


	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//SSAO Pass

	Program& postProcessSSAOProgram = app->programs[app->ssaoEffect.ssaoProgramIdx];
	postProcessSSAOProgram.Bind();

	Mesh& mesh = app->meshes[app->screenQuad];

	GLuint vao = FindVAO(mesh, 0, postProcessSSAOProgram);
	glBindVertexArray(vao);

	app->ssaoEffect.PassUniformsToSSAOShader(app->deferredRendering.depthAttachmentHandle, app->deferredRendering.colorAttachment1Handle, app->cam, app);

	Submesh& submesh = mesh.submeshes[0];
	glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

	glBindVertexArray(0);
	postProcessSSAOProgram.Release();

	app->ssaoEffect.frameBuffer.Release();
}

void SSAOBlurPass(App* app)
{
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	app->ssaoEffect.frameBuffer.Bind();

	//Select on which render targets to draw
	GLuint drawbuffers[] = {
		GL_COLOR_ATTACHMENT1,
	};
	glDrawBuffers(ARRAY_COUNT(drawbuffers), drawbuffers);

	glViewport(0, 0, app->displaySize.x, app->displaySize.y);


	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//SSAO Blur Pass

	Program& postProcessSSAOBlurProgram = app->programs[app->ssaoEffect.blurProgramIdx];
	postProcessSSAOBlurProgram.Bind();

	Mesh& mesh = app->meshes[app->screenQuad];

	GLuint vao = FindVAO(mesh, 0, postProcessSSAOBlurProgram);
	glBindVertexArray(vao);

	app->ssaoEffect.PassUniformsToSSAOBlurShader();

	Submesh& submesh = mesh.submeshes[0];
	glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

	glBindVertexArray(0);
	postProcessSSAOBlurProgram.Release();

	app->ssaoEffect.frameBuffer.Release();
}

void BloomPass(App* app)
{
#define LOD(x) x

	const float w = (float)app->displaySize.x;
	const float h = (float)app->displaySize.y;

	const glm::vec2 horizontal = glm::vec2(1.0, 0.0);
	const glm::vec2 vertical = glm::vec2(0.0, 1.0);

	BloomPassBrightestPixels(app, glm::vec2(w / 2.0, h / 2.0));

	glBindTexture(GL_TEXTURE_2D, app->bloomEffect.rtBright);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Horizontal Blur
	BloomPassBlur(app, &app->bloomEffect.fboBloom1, glm::vec2(w / 2.0, h / 2.0), GL_COLOR_ATTACHMENT1, app->bloomEffect.rtBright, LOD(0), horizontal);
	BloomPassBlur(app, &app->bloomEffect.fboBloom2, glm::vec2(w / 4.0, h / 4.0), GL_COLOR_ATTACHMENT1, app->bloomEffect.rtBright, LOD(1), horizontal);
	BloomPassBlur(app, &app->bloomEffect.fboBloom3, glm::vec2(w / 8.0, h / 8.0), GL_COLOR_ATTACHMENT1, app->bloomEffect.rtBright, LOD(2), horizontal);
	BloomPassBlur(app, &app->bloomEffect.fboBloom4, glm::vec2(w / 16.0, h / 16.0), GL_COLOR_ATTACHMENT1, app->bloomEffect.rtBright, LOD(3), horizontal);
	BloomPassBlur(app, &app->bloomEffect.fboBloom5, glm::vec2(w / 32.0, h / 32.0), GL_COLOR_ATTACHMENT1, app->bloomEffect.rtBright, LOD(4), horizontal);

	//Vertical Blur
	BloomPassBlur(app, &app->bloomEffect.fboBloom1, glm::vec2(w / 2.0, h / 2.0), GL_COLOR_ATTACHMENT0, app->bloomEffect.rtBlurH, LOD(0), vertical);
	BloomPassBlur(app, &app->bloomEffect.fboBloom2, glm::vec2(w / 4.0, h / 4.0), GL_COLOR_ATTACHMENT0, app->bloomEffect.rtBlurH, LOD(1), vertical);
	BloomPassBlur(app, &app->bloomEffect.fboBloom3, glm::vec2(w / 8.0, h / 8.0), GL_COLOR_ATTACHMENT0, app->bloomEffect.rtBlurH, LOD(2), vertical);
	BloomPassBlur(app, &app->bloomEffect.fboBloom4, glm::vec2(w / 16.0, h / 16.0), GL_COLOR_ATTACHMENT0, app->bloomEffect.rtBlurH, LOD(3), vertical);
	BloomPassBlur(app, &app->bloomEffect.fboBloom5, glm::vec2(w / 32.0, h / 32.0), GL_COLOR_ATTACHMENT0, app->bloomEffect.rtBlurH, LOD(4), vertical);


	BloomPassCombine(app, &app->deferredRendering.frameBuffer, GL_COLOR_ATTACHMENT3, app->bloomEffect.rtBright, 4);



#undef LOD
}


void BloomPassBrightestPixels(App* app, glm::vec2 dimensions)
{
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	app->bloomEffect.fboBloom1.Bind();

	//Select on which render targets to draw
	GLuint drawbuffers[] = {
		GL_COLOR_ATTACHMENT0,
	};

	glDrawBuffers(ARRAY_COUNT(drawbuffers), drawbuffers);

	glViewport(0, 0, dimensions.x, dimensions.y);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Program& blitBrightestPixelsProgram = app->programs[app->bloomEffect.blitBrightestPixelsProgramIdx];
	blitBrightestPixelsProgram.Bind();

	Mesh& mesh = app->meshes[app->screenQuad];

	GLuint vao = FindVAO(mesh, 0, blitBrightestPixelsProgram);
	glBindVertexArray(vao);

	app->bloomEffect.PassUniformsToBrightestPixelsShader(dimensions, app->deferredRendering.colorAttachment3Handle);

	Submesh& submesh = mesh.submeshes[0];
	glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindVertexArray(0);
	blitBrightestPixelsProgram.Release();

	app->bloomEffect.fboBloom1.Release();



}

void BloomPassBlur(App* app, Framebuffer* fbo, const glm::vec2& dimensions, GLenum colorAttachment, GLuint textureHandle, GLint lod, const glm::vec2& direction)
{
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	fbo->Bind();
	glDrawBuffer(colorAttachment);
	glViewport(0, 0, dimensions.x, dimensions.y);

	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT);

	Program& blurProgram = app->programs[app->bloomEffect.blurProgramIdx];
	blurProgram.Bind();


	Mesh& mesh = app->meshes[app->screenQuad];

	GLuint vao = FindVAO(mesh, 0, blurProgram);
	glBindVertexArray(vao);

	app->bloomEffect.PassUniformsToBlurShader(textureHandle, lod, direction);

	Submesh& submesh = mesh.submeshes[0];
	glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

	glBindVertexArray(0);
	blurProgram.Release();
	fbo->Release();
}

void BloomPassCombine(App* app, Framebuffer* fbo, GLenum colorAttachment, GLuint textureHandle, GLint maxLOD)
{
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	fbo->Bind();
	glDrawBuffer(colorAttachment);
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT);

	Program& bloomProgram = app->programs[app->bloomEffect.bloomProgramIdx];
	bloomProgram.Bind();


	Mesh& mesh = app->meshes[app->screenQuad];

	GLuint vao = FindVAO(mesh, 0, bloomProgram);
	glBindVertexArray(vao);

	app->bloomEffect.PassUniformsToCombineShader(textureHandle, maxLOD);

	Submesh& submesh = mesh.submeshes[0];
	glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

	glBindVertexArray(0);
	bloomProgram.Release();
	fbo->Release();
}

void LightPass(App* app)
{
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);

	//Render on this framebuffer render targets
	app->deferredRendering.frameBuffer.Bind();

	//Select on which render targets to draw
	GLuint drawbuffers[] = {
		GL_COLOR_ATTACHMENT3, //radiance, illuminated scene
	};
	glDrawBuffers(ARRAY_COUNT(drawbuffers), drawbuffers);

	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Program& deferredlightProgram = app->programs[app->deferredRendering.deferredLightProgramIdx];
	deferredlightProgram.Bind();

	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cBuffer.handle, app->globalParamsoffset, app->globalParamsSize); //Only once as is used for each object

	//TODO push sphere lights as geometry

	//render quad
	Mesh& mesh = app->meshes[app->screenQuad];

	GLuint vao = FindVAO(mesh, 0, deferredlightProgram);
	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, app->deferredRendering.colorAttachment0Handle);
	glUniform1i(app->deferredRendering.deferredLighting_uAlbedo, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, app->deferredRendering.colorAttachment1Handle);
	glUniform1i(app->deferredRendering.deferredLighting_uNormal, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, app->deferredRendering.colorAttachment2Handle);
	glUniform1i(app->deferredRendering.deferredLighting_uPosition, 2);

	if (app->ssaoEffect.GetActive())
	{
		GLuint textureHandle = app->ssaoEffect.blurActive ? app->ssaoEffect.ssaoBlurTextureHandle : app->ssaoEffect.ssaoTextureHandle;
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureHandle);
		glUniform1i(app->deferredRendering.deferredLighting_uSSAO, 3);
	}

	glUniform1i(app->deferredRendering.deferredLighting_uSSAOActive, app->ssaoEffect.GetActive() ? 1 : 0);

	Submesh& submesh = mesh.submeshes[0];
	glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

	glBindVertexArray(0);
	deferredlightProgram.Release();

	app->deferredRendering.frameBuffer.Release();
}

void RenderLightMeshes(App* app)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	//Render on this framebuffer render targets
	app->deferredRendering.frameBuffer.Bind();

	//Select on which render targets to draw
	GLuint drawbuffers[] = {
		GL_COLOR_ATTACHMENT3 //Final
	};
	glDrawBuffers(ARRAY_COUNT(drawbuffers), drawbuffers);

	glViewport(0, 0, app->displaySize.x, app->displaySize.y);


	Program& deferredLightMeshProgram = app->programs[app->deferredRendering.deferredLightMeshProgramIdx];
	deferredLightMeshProgram.Bind();

	//Render lights

	for (int n = 0; n < app->lightEntities.size(); ++n)
	{
		if (app->lights[n].type == LightType::LightType_Directional)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
		}

		Model& model = app->models[app->lightEntities[n].modelIndex];
		Mesh& mesh = app->meshes[model.meshIdx];

		for (u32 i = 0; i < mesh.submeshes.size(); ++i)
		{
			GLuint vao = FindVAO(mesh, i, deferredLightMeshProgram);
			glBindVertexArray(vao);

			glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->lBuffer.handle, app->lightEntities[n].localParamsOffset, app->lightEntities[n].localParamsSize);

			Submesh& submesh = mesh.submeshes[i];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}
	}

	//TODO release Vao & programs?
	glBindVertexArray(0);
	deferredLightMeshProgram.Release();
	app->deferredRendering.frameBuffer.Release();
}

void RenderTextureToScreen(App* app, GLuint textureHandle, bool isDepth)
{
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, app->displaySize.x, app->displaySize.y);


	Program& displayToScreenProgram = app->programs[app->displayToScreenProgramIdx];
	displayToScreenProgram.Bind();

	Mesh& mesh = app->meshes[app->screenQuad];

	GLuint vao = FindVAO(mesh, 0, displayToScreenProgram);
	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glUniform1i(app->programUniformTexture, 0);

	glUniform1i(app->programUniformIsDepth, isDepth ? 1 : 0);

	Submesh& submesh = mesh.submeshes[0];
	glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);

	glBindVertexArray(0);
	displayToScreenProgram.Release();
}

void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	ELOG("OpenGL debug message: %s", message);
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:				ELOG(" - source: GL_DEBUG_SOURCE_API"); break; //Calls to the OpenGl API
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		ELOG(" - source: GL_DEBUG_SOURCE_WINDOW_SYSTEM"); break; //Calls to a window-system API
	case GL_DEBUG_SOURCE_SHADER_COMPILER:   ELOG(" - source: GL_DEBUG_SOURCE_SHADER_COMPILER"); break; //A compiler for a shading language
	case GL_DEBUG_SOURCE_THIRD_PARTY:		ELOG(" - source: GL_DEBUG_SOURCE_THIRD_PARTY"); break; //An application associated with OpenGL
	case GL_DEBUG_SOURCE_APPLICATION:		ELOG(" - source: GL_DEBUG_SOURCE_APPLICATION"); break; //Generated by the user of this application
	case GL_DEBUG_SOURCE_OTHER:				ELOG(" - source: GL_DEBUG_SOURCE_OTHER"); break; //Some source that isn't one of these
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:				ELOG(" - type: GL_DEBUG_TYPE_ERROR"); break; //An error, typically from the API
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	ELOG(" - type: GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"); break; //Some behaviour marked deprecated ...
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	ELOG(" - type: GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"); break; //Something has invoked undefined behaviour
	case GL_DEBUG_TYPE_PORTABILITY:			ELOG(" - type: GL_DEBUG_TYPE_PORTABILITY"); break; //Some functionallity the user relies upon ...
	case GL_DEBUG_TYPE_PERFORMANCE:			ELOG(" - type: GL_DEBUG_TYPE_PERFORMANCE"); break; //Code has triggered possible performance ...
	case GL_DEBUG_TYPE_MARKER:				ELOG(" - type: GL_DEBUG_TYPE_MARKER"); break; //Command stream annotation
	case GL_DEBUG_TYPE_PUSH_GROUP:			ELOG(" - type: GL_DEBUG_TYPE_PUSH_GROUP"); break; //Group pushing
	case GL_DEBUG_TYPE_POP_GROUP:			ELOG(" - type: GL_DEBUG_TYPE_POP_GROUP"); break; //foo
	case GL_DEBUG_TYPE_OTHER:				ELOG(" - type: GL_DEBUG_TYPE_OTHER"); break; //Some type that isn't one of these
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:				ELOG(" - type: GL_DEBUG_SEVERITY_HIGH"); break; //All OpenGL Errors, shader compilation/link ...
	case GL_DEBUG_SEVERITY_MEDIUM:				ELOG(" - type: GL_DEBUG_SEVERITY_MEDIUM"); break; //Major performance warnings, shader compilation ...
	case GL_DEBUG_SEVERITY_LOW:					ELOG(" - type: GL_DEBUG_SEVERITY_LOW"); break; //Redundant state change performance warning, ...
	case GL_DEBUG_SEVERITY_NOTIFICATION:		ELOG(" - type: GL_DEBUG_SEVERITY_NOTIFICATION"); break; //Anything that isn't an error or performance ...
	}
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
	Submesh& submesh = mesh.submeshes[submeshIndex];

	//Try finding a vao for this submesh/program
	for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
	{
		if (submesh.vaos[i].programHandle == program.handle)
		{
			return submesh.vaos[i].handle;
		}
	}
	GLuint vaoHandle = 0;

	//Create a new vao for this submesh/program
	{
		glGenVertexArrays(1, &vaoHandle);
		glBindVertexArray(vaoHandle);

		glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

		//We have to link all vertex inputs attributes to attributes in the vertex buffer
		for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
		{
			bool attributeWasLinked = false;
			for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
			{
				if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
				{
					const u32 index = submesh.vertexBufferLayout.attributes[j].location;
					const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
					const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;//Attribute offset + vertex offset
					const u32 stride = submesh.vertexBufferLayout.stride;
					glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
					glEnableVertexAttribArray(index);

					attributeWasLinked = true;
					break;
				}
			}
			if (!attributeWasLinked)
			{
				ELOG("An attribute couldn't be linked"); //The submesh should provide an attribute for each vertex inputs
			}
		}
		glBindVertexArray(0);
	}
	//Store it in the list of vaos for this submesh
	Vao vao = { vaoHandle,program.handle };
	submesh.vaos.push_back(vao);

	return vaoHandle;
}

void CreateQuad(App* app)
{
	std::vector<float> quadVertexVec;
	std::vector<u32> quadIndicesVec;

	const VertexV3V2 vertices[] = {
		{glm::vec3(-1,-1,0.0),glm::vec2(0.0,0.0)}, //Bottom-left
		{glm::vec3(1,-1,0.0),glm::vec2(1.0,0.0)}, //Bottom-right
		{glm::vec3(1, 1,0.0),glm::vec2(1.0,1.0)},//Top-right
		{glm::vec3(-1, 1,0.0),glm::vec2(0.0,1.0)}//Top-left
	};

	int verticesSize = sizeof(vertices) / sizeof(VertexV3V2);
	for (int i = 0; i < verticesSize; ++i)
	{
		quadVertexVec.push_back(vertices[i].pos.x);
		quadVertexVec.push_back(vertices[i].pos.y);
		quadVertexVec.push_back(vertices[i].pos.z);
		quadVertexVec.push_back(vertices[i].uv.x);
		quadVertexVec.push_back(vertices[i].uv.y);
	}

	const u16 indices[] = {
		0,1,2,
		0,2,3
	};

	int indicesSize = sizeof(indices) / sizeof(u16);

	for (int i = 0; i < indicesSize; ++i)
	{
		quadIndicesVec.push_back(indices[i]);
	}

	std::vector<VertexBufferAttribute> vertexFormat;
	vertexFormat.push_back(VertexBufferAttribute({ 0, 3, 0 }));
	vertexFormat.push_back(VertexBufferAttribute({ 1, 2,sizeof(vec3) }));

	Mesh* mesh = CreateMesh(app, &app->screenQuad);

	//mesh->name = "Sphere";
	mesh->AddSubmesh(vertexFormat, quadVertexVec, quadIndicesVec);
	mesh->GenerateMeshData(app);
}

GLuint CreateModelFromMesh(App* app, GLuint meshIdx)
{
	app->models.push_back(Model{});
	Model& model = app->models.back();
	model.meshIdx = meshIdx;
	return app->models.size() - 1u;
}

void CreateSphere(App* app)
{
#define H 32
#define V 16
	static const float pi = 3.1416f;

	Vertex sphere[H][V + 1];
	std::vector<float> sphereVertexVec;

	for (int h = 0; h < H; ++h) {
		for (int v = 0; v < V + 1; ++v)
		{
			float nh = float(h) / H;
			float nv = float(v) / V - 0.5f;
			float angleh = 2 * pi * nh;
			float anglev = -pi * nv;
			sphere[h][v].pos.x = sinf(angleh) * cosf(anglev);
			sphere[h][v].pos.y = -sinf(anglev);
			sphere[h][v].pos.z = cosf(angleh) * cosf(anglev);
			sphere[h][v].norm = sphere[h][v].pos;
			sphereVertexVec.push_back(sphere[h][v].pos.x);
			sphereVertexVec.push_back(sphere[h][v].pos.y);
			sphereVertexVec.push_back(sphere[h][v].pos.z);
			sphereVertexVec.push_back(sphere[h][v].norm.x);
			sphereVertexVec.push_back(sphere[h][v].norm.y);
			sphereVertexVec.push_back(sphere[h][v].norm.z);
		}
	}
	unsigned int sphereIndices[H][V][6];
	std::vector<u32> sphereIndicesVec;
	for (unsigned int h = 0; h < H; ++h) {
		for (unsigned int v = 0; v < V; ++v) {
			sphereIndices[h][v][0] = (h + 0) * (V + 1) + v;
			sphereIndices[h][v][1] = ((h + 1) % H) * (V + 1) + v;
			sphereIndices[h][v][2] = ((h + 1) % H) * (V + 1) + v + 1;
			sphereIndices[h][v][3] = (h + 0) * (V + 1) + v;
			sphereIndices[h][v][4] = ((h + 1) % H) * (V + 1) + v + 1;
			sphereIndices[h][v][5] = (h + 0) * (V + 1) + v + 1;

			sphereIndicesVec.push_back(sphereIndices[h][v][0]);
			sphereIndicesVec.push_back(sphereIndices[h][v][1]);
			sphereIndicesVec.push_back(sphereIndices[h][v][2]);
			sphereIndicesVec.push_back(sphereIndices[h][v][3]);
			sphereIndicesVec.push_back(sphereIndices[h][v][4]);
			sphereIndicesVec.push_back(sphereIndices[h][v][5]);
		}
	}

	std::vector<VertexBufferAttribute> vertexFormat;
	vertexFormat.push_back(VertexBufferAttribute({ 0, 3, 0 }));
	vertexFormat.push_back(VertexBufferAttribute({ 1, 3,sizeof(vec3) }));

	Mesh* mesh = CreateMesh(app, &app->sphereMesh);
	//mesh->name = "Sphere";
	mesh->AddSubmesh(vertexFormat, sphereVertexVec, sphereIndicesVec);
	mesh->GenerateMeshData(app);
}

Mesh* CreateMesh(App* app, u32* index)
{
	app->meshes.push_back(Mesh{});
	Mesh& mesh = app->meshes.back();

	if (index != nullptr)
		*index = ((u32)app->meshes.size() - 1u);

	return &app->meshes.back();
}

glm::mat4 TransformScale(const glm::vec3& scaleFactors)
{
	return glm::scale(scaleFactors);
}

glm::mat4 TransformPositionScale(const glm::vec3& pos, const glm::vec3& scaleFactors)
{
	glm::mat4 transform = glm::translate(pos);
	return glm::scale(transform, scaleFactors);
}

glm::mat4 TransformPositionScaleRot(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scaleFactors)
{
	glm::mat4 scale = glm::scale(glm::mat4(1.0), scaleFactors);
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0), glm::radians(rot.z), vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0), glm::radians(rot.y), vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0), glm::radians(rot.x), vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 transform = glm::translate(pos);

	return transform * rotateZ * rotateY * rotateX * scale;
}

u32 AddEntity(App* app, const char* name, u32 modelIndex)
{
	Entity toAdd;
	toAdd.name = MakeString(name);
	toAdd.modelIndex = modelIndex;
	toAdd.id = rand();

	toAdd.position = vec3(0.0f);
	toAdd.rotation = vec3(0.0f);
	toAdd.scale = vec3(1.0f);

	toAdd.UpdateWorldMatrix();

	app->entities.push_back(toAdd);
	return app->entities.size() - 1;
}

u32 AddPointLightEntity(App* app, vec3 position, vec3 scale)
{
	Entity toAdd;
	toAdd.name = MakeString("PointLight");
	toAdd.modelIndex = app->sphereModel;
	toAdd.id = rand();

	toAdd.position = position;
	toAdd.rotation = vec3(0.0f);
	toAdd.scale = scale;

	toAdd.UpdateWorldMatrix();

	app->lightEntities.push_back(toAdd);
	return app->lightEntities.size() - 1;
}

u32 AddDirectionalLightEntity(App* app, vec3 direction, vec3 scale, float offset)
{
	Entity toAdd;
	toAdd.name = MakeString("DirectionalLight");
	toAdd.modelIndex = app->quadModel;
	toAdd.id = rand();

	toAdd.position = (-direction * offset);
	toAdd.scale = scale;
	toAdd.rotation = glm::eulerAngles(glm::quatLookAt(direction, vec3(0.0, 1.0, 0.0)));
	toAdd.rotation = glm::degrees(toAdd.rotation);
	toAdd.UpdateWorldMatrix();

	app->lightEntities.push_back(toAdd);
	return app->lightEntities.size() - 1;
}

Light* CreateDirectionalLight(App* app, vec3 color, vec3 direction)
{
	Light l;
	l.color = color;
	l.direction = glm::normalize(direction);
	l.type = LightType::LightType_Directional;
	l.position = vec3(0.0, 0.0, 0.0);
	AddDirectionalLightEntity(app, l.direction, vec3(1.0f), 10.0f);

	app->lights.push_back(l);

	return &app->lights.back();
}

Light* CreatePointLight(App* app, vec3 color, vec3 position)
{
	Light l;
	l.color = color;
	l.direction = vec3(0.0, 0.0, 0.0);
	l.type = LightType::LightType_Point;
	l.position = position;

	AddPointLightEntity(app, position, vec3(0.1f));

	app->lights.push_back(l);

	return &app->lights.back();
}
//
//Framebuffer GenerateFrameBuffer(App* app)
//{
//	Framebuffer ret;
//
//	ret.colorAttachment0Handle = GenerateColTex2DHighPrecision(app->displaySize);
//	ret.colorAttachment1Handle = GenerateColTex2DHighPrecision(app->displaySize);
//	ret.colorAttachment2Handle = GenerateColTex2DHighPrecision(app->displaySize);
//	ret.colorAttachment3Handle = GenerateColTex2DHighPrecision(app->displaySize);
//
//	ret.depthAttachmentHandle = GenerateDepthTex2D(app->displaySize);
//
//	ret.handle;
//	glGenFramebuffers(1, &ret.handle);
//	glBindFramebuffer(GL_FRAMEBUFFER, ret.handle);
//	//TODO make framebuffers generate attachments needed from a parameter
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ret.colorAttachment0Handle, 0); //Albedo
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ret.colorAttachment1Handle, 0); //Normal
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, ret.colorAttachment2Handle, 0); //Position
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, ret.colorAttachment3Handle, 0); //Radiance
//
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ret.depthAttachmentHandle, 0);
//
//	GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
//	{
//		switch (framebufferStatus)
//		{
//		case GL_FRAMEBUFFER_UNDEFINED: ELOG("GL_FRAMEBUFFER_UNDEFINED"); break;
//		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
//		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
//		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); break;
//		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); break;
//		case GL_FRAMEBUFFER_UNSUPPORTED: ELOG("GL_FRAMEBUFFER_UNSUPPORTED"); break;
//		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); break;
//		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); break;
//		default: ELOG("Unknown framebuffer status error");
//		}
//	}
//
//	GLenum buffers[] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3 };
//	glDrawBuffers(ARRAY_COUNT(buffers), buffers);
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//	return ret;
//}

GLuint GenerateColTex2D(vec2 displaySize)
{
	GLuint ret;
	glGenTextures(1, &ret);
	glBindTexture(GL_TEXTURE_2D, ret);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, displaySize.x, displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	return ret;
}
GLuint GenerateColTex2DHighPrecision(vec2 displaySize)
{
	GLuint ret;
	glGenTextures(1, &ret);
	glBindTexture(GL_TEXTURE_2D, ret);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, displaySize.x, displaySize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	return ret;
}

GLuint GenerateDepthTex2D(vec2 displaySize)
{
	GLuint ret;
	glGenTextures(1, &ret);
	glBindTexture(GL_TEXTURE_2D, ret);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, displaySize.x, displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	return ret;
}

//Once we have the mesh with its submeshes, we can construct its data
void Mesh::GenerateMeshData(App* app)
{
	//first we delete buffers if they have been created before
	glDeleteBuffers(1, &vertexBufferHandle);
	glDeleteBuffers(1, &indexBufferHandle);

	u32 vertexBufferSize = 0;
	u32 indexBufferSize = 0;

	for (u32 i = 0; i < submeshes.size(); ++i)
	{
		vertexBufferSize += submeshes[i].vertices.size() * sizeof(float);
		indexBufferSize += submeshes[i].indices.size() * sizeof(u32);
	}

	glGenBuffers(1, &vertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, NULL, GL_STATIC_DRAW);

	u32 indicesOffset = 0;
	u32 verticesOffset = 0;

	for (u32 i = 0; i < submeshes.size(); ++i)
	{
		const void* verticesData = submeshes[i].vertices.data();
		const u32   verticesSize = submeshes[i].vertices.size() * sizeof(float);
		glBufferSubData(GL_ARRAY_BUFFER, verticesOffset, verticesSize, verticesData);
		submeshes[i].vertexOffset = verticesOffset;
		verticesOffset += verticesSize;

		const void* indicesData = submeshes[i].indices.data();
		const u32   indicesSize = submeshes[i].indices.size() * sizeof(u32);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indicesData);
		submeshes[i].indexOffset = indicesOffset;
		indicesOffset += indicesSize;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::AddSubmesh(std::vector<VertexBufferAttribute> format, std::vector<float> vertexData, std::vector<u32> indicesData)
{
	// create the vertex format
	VertexBufferLayout vertexBufferLayout = {};
	for (int i = 0; i < format.size(); i++)
	{
		vertexBufferLayout.attributes.push_back(format[i]);
		vertexBufferLayout.stride += format[i].componentCount * sizeof(float);
	}

	// add the submesh into the mesh
	Submesh submesh = {};
	submesh.vertexBufferLayout = vertexBufferLayout;
	submesh.vertices.swap(vertexData);
	submesh.indices.swap(indicesData);
	submeshes.push_back(submesh);
}

Camera::Camera()
{
	projectionMode = ProjectionMode::PERSPECTIVE;
	fov = glm::radians(60.0f);
	aspectRatio = 1920.0f / 1080.0f;
	zNear = 0.1f;
	zFar = 1000.0f;

	pitch = yaw = 0.0f;
	offsetDirection = vec3(1.0f, 1.0f, -1.0f);
	zoom = 15.0f;
	target = vec3(0.0f);
	UpdateMatrices();
}

Camera::Camera(float fov, float aspectRatio, float zNear, float zFar) :fov(fov), aspectRatio(aspectRatio), zNear(zNear), zFar(zFar)
{
	projectionMode = ProjectionMode::PERSPECTIVE;
	offsetDirection = vec3(1.0f, 1.0f, -1.0f);
	zoom = 15.0f;
	pitch = yaw = 0.0f;
	target = vec3(0.0f, 0.0f, 0.0f);
	UpdateMatrices();
}

void Camera::UpdateMatrices()
{
	view = glm::lookAt(GetCamPos(), target, vec3(0.0f, 1.0f, 0.0f));

	if (projectionMode == ProjectionMode::PERSPECTIVE)
	{
		projection = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);
		return;
	}

	assert("Orthogonal projection not avaliable yet!"); //TODO create orthogonal projection in the future?
}

glm::vec3 Camera::GetCamPos()
{
	return target + offsetDirection*zoom;
}

glm::mat4 Entity::UpdateWorldMatrix()
{
	worldMatrix = TransformPositionScaleRot(position, rotation, scale);
	return worldMatrix;
}

Material::Material()
{
	albedo = emissive = vec3(0.0);
	smoothness = 0;
	albedoTextureIdx = emissiveTextureIdx = specularTextureIdx = normalsTextureIdx = bumpTextureIdx = UINT32_MAX;
}