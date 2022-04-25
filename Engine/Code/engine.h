//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include "assimp_model_loading.h"
#include <glad/glad.h>


struct Vertex
{
	vec3 pos;
	vec3 norm;
	vec2 uv;
};


struct VertexBufferAttribute
{
	u8 location;
	u8 componentCount;
	u8 offset;
};

struct VertexBufferLayout
{
	std::vector<VertexBufferAttribute> attributes;
	u8 stride;
};

struct VertexShaderAttribute
{
	u8 location;
	u8 componentCount;
};

struct VertexShaderLayout
{
	std::vector<VertexShaderAttribute> attributes;
};

struct Vao
{
	GLuint handle;
	GLuint programHandle;
};


struct Model
{
	u32 meshIdx;
	std::vector<u32> materialIdx;
};

struct Submesh
{
	VertexBufferLayout vertexBufferLayout;
	std::vector<float> vertices;
	std::vector<u32> indices;
	u32 vertexOffset;
	u32 indexOffset;
	std::vector<Vao> vaos;
};

class Mesh
{
public:
	std::vector<Submesh> submeshes;
	GLuint vertexBufferHandle;
	GLuint indexBufferHandle;

	void AddSubmesh(std::vector<VertexBufferAttribute> format, std::vector<float> vertexData, std::vector<u32> indicesData);
	void GenerateMeshData(App* app);
};

struct Material
{
	std::string name;
	vec3 albedo;
	vec3 emissive;
	f32 smoothness;
	u32 albedoTextureIdx;
	u32 emissiveTextureIdx;
	u32 specularTextureIdx;
	u32 normalsTextureIdx;
	u32 bumpTextureIdx;

};




struct Image
{
	void* pixels;
	ivec2 size;
	i32   nchannels;
	i32   stride;
};

struct Texture
{
	GLuint      handle;
	std::string filepath;
};

struct Program
{
	GLuint             handle;
	std::string        filepath;
	std::string        programName;
	u64                lastWriteTimestamp; // What is this for?
	VertexShaderLayout vertexInputLayout;

};

enum Mode
{
	Mode_TexturedQuad,
	Mode_Count
};
struct VertexV3V2
{
	glm::vec3 pos;
	glm::vec2 uv;
};

const VertexV3V2 vertices[] = {
	{glm::vec3(-0.5,-0.5,0.0),glm::vec2(0.0,0.0)}, //Bottom-left
	{glm::vec3(0.5,-0.5,0.0),glm::vec2(1.0,0.0)}, //Bottom-right
	{glm::vec3(0.5, 0.5,0.0),glm::vec2(1.0,1.0)},//Top-right
	{glm::vec3(-0.5, 0.5,0.0),glm::vec2(0.0,1.0)}//Top-left
};

const u16 indices[] = {
	0,1,2,
	0,2,3
};

struct App
{
	// Loop
	f32  deltaTime;
	bool isRunning;

	// Input
	Input input;

	//information about opengl
	OpenGLInfo glInfo;
	bool showGlInfoWindow;

	// Graphics
	char gpuName[64];
	char openGlVersion[64];

	ivec2 displaySize;

	std::vector<Texture>  textures;
	std::vector<Material>  materials;
	std::vector<Mesh>  meshes;
	std::vector<Model>  models;
	std::vector<Program>  programs;

	// program indices
	u32 texturedGeometryProgramIdx;
	u32 texturedMeshProgramIdx;

	// texture indices
	u32 diceTexIdx;
	u32 whiteTexIdx;
	u32 blackTexIdx;
	u32 normalTexIdx;
	u32 magentaTexIdx;

	// Mode
	Mode mode;

	// Embedded geometry (in-editor simple meshes such as
	// a screen filling quad, a cube, a sphere...)
	GLuint embeddedVertices;
	GLuint embeddedElements;

	// Location of the texture uniform in the textured quad shader
	GLuint programUniformTexture;

	// Location of the texture uniform in the textured mesh shader
	GLuint texturedMeshProgram_uTexture;

	// VAO object to link our screen filling quad with our textured quad shader
	GLuint vao;

	//Patrick Model
	GLuint model;

	GLuint sphereMesh;
};

GLuint CreateProgramFromSource(String programSource, const char* shaderName);

u32 LoadProgram(App* app, const char* filepath, const char* programName);

Image LoadImage(const char* filename);

void FreeImage(Image image);

GLuint CreateTexture2DFromImage(Image image);

u32 LoadTexture2D(App* app, const char* filepath);

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

void CreateSphere(App* app);


Mesh* CreateMesh(App * app, u32* index = nullptr);

Submesh* CreateSubmesh(std::vector<vec3> verticesToProcess, std::vector<vec3> normalsToProcess, std::vector<u32> indicesToProess);

void AddSubmeshToMesh(Submesh* submesh);

