//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//


//Vertices = 0
//Normals = 1
//TexCoords = 2

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
	GLchar  infoLogBuffer[1024] = {};
	GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
	GLsizei infoLogSize;
	GLint   success;

	char versionString[] = "#version 430\n";
	char shaderNameDefine[128];
	sprintf(shaderNameDefine, "#define %s\n", shaderName);
	char vertexShaderDefine[] = "#define VERTEX\n";
	char fragmentShaderDefine[] = "#define FRAGMENT\n";

	const GLchar* vertexShaderSource[] = {
		versionString,
		shaderNameDefine,
		vertexShaderDefine,
		programSource.str
	};
	const GLint vertexShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(vertexShaderDefine),
		(GLint)programSource.len
	};
	const GLchar* fragmentShaderSource[] = {
		versionString,
		shaderNameDefine,
		fragmentShaderDefine,
		programSource.str
	};
	const GLint fragmentShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(fragmentShaderDefine),
		(GLint)programSource.len
	};

	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
	glCompileShader(vshader);
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
	glCompileShader(fshader);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint programHandle = glCreateProgram();
	glAttachShader(programHandle, vshader);
	glAttachShader(programHandle, fshader);
	glLinkProgram(programHandle);
	glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	glUseProgram(0);

	glDetachShader(programHandle, vshader);
	glDetachShader(programHandle, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
	String programSource = ReadTextFile(filepath);

	Program program = {};
	program.handle = CreateProgramFromSource(programSource, programName);
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
	app->showGlInfoWindow = false;

	// TODO: Initialize your resources here!
	// - vertex buffers
	// - element/index buffers
	// - vaos
	// - programs (and retrieve uniform indices)
	// - textures



	//Geometry
	//VBO
	glGenBuffers(1, &app->embeddedVertices);
	glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//EBO
	glGenBuffers(1, &app->embeddedElements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//VAO
	glGenVertexArrays(1, &app->vao);
	glBindVertexArray(app->vao);
	glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);//embedd the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0);//Location 0 of the shader this is vertex pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)12);//Location 0 of the shader this is uv coords
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);//embedd the EBO
	glBindVertexArray(0);

	//Program
	app->texturedGeometryProgramIdx = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
	Program& texturedGeometryProgram = app->programs[app->texturedGeometryProgramIdx];
	app->programUniformTexture = glGetUniformLocation(texturedGeometryProgram.handle, "uTexture");

	app->texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", "SHOW_TEXTURED_MESH");
	Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
	app->texturedMeshProgram_uTexture = glGetUniformLocation(texturedMeshProgram.handle, "uTexture");
	//Texture
	app->diceTexIdx = LoadTexture2D(app, "dice.png");
	app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
	app->blackTexIdx = LoadTexture2D(app, "color_black.png");
	app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
	app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

	app->mode = Mode_TexturedQuad;

	app->model = LoadModel(app, "Patrick/Patrick.obj");


	CreateSphere(app);
	CreateQuad(app);
}

void Gui(App* app)
{
	ImGui::Begin("Info");
	ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
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

}

void Update(App* app)
{
	// You can handle app->input keyboard/mouse here

	//Toggle glInfoWindow
	if (app->input.keys[Key::K_I] == ButtonState::BUTTON_PRESS)
	{
		app->showGlInfoWindow = !app->showGlInfoWindow;
	}

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
			program.handle = CreateProgramFromSource(programSource, programName);
			program.lastWriteTimestamp = currentTimestamp;
		}
	}

}

void Render(App* app)
{
	// - clear the framebuffer
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);



	switch (app->mode)
	{
	case Mode_TexturedQuad:
	{
		//// - bind the program 
		//Program& texturedGeometryProgram = app->programs[app->texturedGeometryProgramIdx];
		//glUseProgram(texturedGeometryProgram.handle);
		//// - bind the vao
		//glBindVertexArray(app->vao);

		//// - set the blending state
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		////   (...and make its texture sample from unit 0)
		//glUniform1i(app->programUniformTexture, 0);
		//glActiveTexture(GL_TEXTURE0);
		//// - bind the texture into unit 0
		//GLuint textureHandle = app->textures[app->diceTexIdx].handle;
		//glBindTexture(GL_TEXTURE_2D, textureHandle);

		//// - glDrawElements() !!!
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		////unbind
		//glBindVertexArray(0);
		//glUseProgram(0);

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Program& texturedGeometryProgram = app->programs[app->texturedGeometryProgramIdx];
		glUseProgram(texturedGeometryProgram.handle);

		Mesh& mesh = app->meshes[app->screenQuad];

		for (u32 i = 0; i < mesh.submeshes.size(); ++i)
		{
			GLuint vao = FindVAO(mesh, i, texturedGeometryProgram);
			glBindVertexArray(vao);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, app->textures[app->diceTexIdx].handle);
			glUniform1i(app->programUniformTexture, 0);

			Submesh& submesh = mesh.submeshes[i];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}
		glBindVertexArray(0);
		glUseProgram(0);
	}
	break;
	case Mode_Count:
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
		glUseProgram(texturedMeshProgram.handle);

		Model& model = app->models[app->model];
		Mesh& mesh = app->meshes[model.meshIdx];

		for (u32 i = 0; i < mesh.submeshes.size(); ++i)
		{
			GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
			glBindVertexArray(vao);

			u32 submeshMaterialIdx = model.materialIdx[i];
			Material& submeshMaterial = app->materials[submeshMaterialIdx];

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
			glUniform1i(app->texturedMeshProgram_uTexture, 0);

			Submesh& submesh = mesh.submeshes[i];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}

		//TODO consider puting this code into a method that takes a mesh or a model and a program and draws things
		/*mesh = app->meshes[app->sphereMesh];
		for (u32 i = 0; i < mesh.submeshes.size(); ++i)
		{
			GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
			glBindVertexArray(vao);

			u32 submeshMaterialIdx = model.materialIdx[i];
			Material& submeshMaterial = app->materials[submeshMaterialIdx];

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, app->textures[app->diceTexIdx].handle);
			glUniform1i(app->texturedMeshProgram_uTexture, 0);

			Submesh& submesh = mesh.submeshes[i];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}*/

		glBindVertexArray(0);
		glUseProgram(0);
	}
	break;

	default:;
	}
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
			assert(attributeWasLinked); //The submesh should provide an attribute for each vertex inputs
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

/*
Submesh* CreateSubmesh(std::vector<vec3> verticesToProcess, std::vector<vec3> normalsToProcess, std::vector<u32> indicesToProess)
{
	std::vector<float> vertices;
	std::vector<u32> indices;

	bool hasTexCoords = false;
	bool hasTangentSpace = false;

	// process vertices
	for (unsigned int i = 0; i < verticesToProcess.size(); i++)
	{
		vertices.push_back(mesh->mVertices[i].x);
		vertices.push_back(mesh->mVertices[i].y);
		vertices.push_back(mesh->mVertices[i].z);
		vertices.push_back(mesh->mNormals[i].x);
		vertices.push_back(mesh->mNormals[i].y);
		vertices.push_back(mesh->mNormals[i].z);

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			hasTexCoords = true;
			vertices.push_back(mesh->mTextureCoords[0][i].x);
			vertices.push_back(mesh->mTextureCoords[0][i].y);
		}

		if (mesh->mTangents != nullptr && mesh->mBitangents)
		{
			hasTangentSpace = true;
			vertices.push_back(mesh->mTangents[i].x);
			vertices.push_back(mesh->mTangents[i].y);
			vertices.push_back(mesh->mTangents[i].z);

			// For some reason ASSIMP gives me the bitangents flipped.
			// Maybe it's my fault, but when I generate my own geometry
			// in other files (see the generation of standard assets)
			// and all the bitangents have the orientation I expect,
			// everything works ok.
			// I think that (even if the documentation says the opposite)
			// it returns a left-handed tangent space matrix.
			// SOLUTION: I invert the components of the bitangent here.
			vertices.push_back(-mesh->mBitangents[i].x);
			vertices.push_back(-mesh->mBitangents[i].y);
			vertices.push_back(-mesh->mBitangents[i].z);
		}
	}

	// process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// store the proper (previously proceessed) material for this mesh
	submeshMaterialIndices.push_back(baseMeshMaterialIndex + mesh->mMaterialIndex);

	// create the vertex format
	VertexBufferLayout vertexBufferLayout = {};
	vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 0, 3, 0 });
	vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 1, 3, 3 * sizeof(float) });
	vertexBufferLayout.stride = 6 * sizeof(float);
	if (hasTexCoords)
	{
		vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 2, 2, vertexBufferLayout.stride });
		vertexBufferLayout.stride += 2 * sizeof(float);
	}
	if (hasTangentSpace)
	{
		vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 3, 3, vertexBufferLayout.stride });
		vertexBufferLayout.stride += 3 * sizeof(float);

		vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 4, 3, vertexBufferLayout.stride });
		vertexBufferLayout.stride += 3 * sizeof(float);
	}

	// add the submesh into the mesh
	Submesh submesh = {};
	submesh.vertexBufferLayout = vertexBufferLayout;
	submesh.vertices.swap(vertices);
	submesh.indices.swap(indices);
	myMesh->submeshes.push_back(submesh);
}
*/
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


