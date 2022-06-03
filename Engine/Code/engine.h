//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include "Program.h"
#include "ForwardRendering.h"
#include "DeferredRendering.h"
#include "SSAO.h"
#include "Bloom.h"
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

struct VertexShaderAttribute;

struct VertexShaderLayout;

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

class Material
{
public:
	std::string name;
	vec3 albedo;
	vec3 emissive;
	f32 smoothness;
	u32 albedoTextureIdx;
	u32 emissiveTextureIdx;
	u32 specularTextureIdx;
	u32 normalsTextureIdx;
	u32 bumpTextureIdx;

	Material();
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

class Camera
{
public:
	enum ProjectionMode
	{
		PERSPECTIVE,
		ORTHOGONAL
	};

	ProjectionMode projectionMode;
	float fov;
	float aspectRatio;
	float zNear;
	float zFar;
	glm::vec3 position;
	glm::vec3 target;
	glm::mat4 projection;
	glm::mat4 view;

public:
	Camera();
	Camera(float fov, float aspectRatio, float zNear, float zFar);
	void UpdateMatrices();
};

class Entity
{
public:
	String name;
	u32 id;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	glm::mat4 worldMatrix;
	u32 modelIndex;
	u32 localParamsOffset;
	u32 localParamsSize;

public:
	glm::mat4 UpdateWorldMatrix();
};

struct Buffer
{
	GLuint handle;
	GLenum type;
	u32 size;
	u32 head;
	void* data; // mapped data
};

enum LightType
{
	LightType_Directional,
	LightType_Point
};

struct Light
{
	LightType type;
	vec3 color;
	vec3 direction;
	vec3 position;
};

struct Framebuffer;

struct App
{
	SSAO ssaoEffect;
	Bloom bloomEffect;
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

	std::vector<Entity> entities;
	std::vector<Entity> lightEntities;

	// program indices
	u32 displayToScreenProgramIdx;


	
	// texture indices
	u32 diceTexIdx;
	u32 whiteTexIdx;
	u32 blackTexIdx;
	u32 normalTexIdx;
	u32 magentaTexIdx;

	// Embedded geometry (in-editor simple meshes such as
	// a screen filling quad, a cube, a sphere...)
	GLuint screenQuad; //Mesh
	GLuint sphereMesh; //Mesh

	// Location of the texture uniform in the textured quad shader
	GLuint programUniformTexture;
	GLuint programUniformIsDepth;
	// Location of the texture uniform in the textured mesh shader
	GLuint texturedMeshProgram_uTexture;

	

	//Patrick Model
	GLuint model;
	//Room Model
	GLuint modelRoom;

	//Models
	GLuint quadModel;
	GLuint sphereModel;

	GLint maxUniformBufferSize;
	GLint uniformBlockAlignment;

	Camera cam;

	std::vector<Light> lights;

	//buffers
	Buffer lBuffer;//local buffers
	Buffer cBuffer;//common buffers
	u32 globalParamsoffset;
	u32 globalParamsSize;

	ForwardRendering forwardRendering;
	DeferredRendering deferredRendering;

	//Texture Display mode
	u32 displayMode;

	//Debug
	bool renderLightMeshes;
};

Image LoadImage(const char* filename);

void FreeImage(Image image);

GLuint CreateTexture2DFromImage(Image image);

u32 LoadTexture2D(App* app, const char* filepath);

void Init(App* app);

float RandSph();

void Gui(App* app);

void Update(App* app);

void HandleCameraMove(App* app);

void Render(App* app);

void ForwardRender(App* app);

void DeferredRender(App* app);

GLuint GetDisplayTexture(App* app);

void GeometryPass(App* app);

void SSAOPass(App* app);

void SSAOBlurPass(App* app);

void BloomPass(App* app);
void BloomPassBrightestPixels(App* app, glm::vec2 dimensions);

void BloomPassBlur(App* app, Framebuffer* fbo, const glm::vec2& dimensions, GLenum colorAttachment, GLuint textureHandle, GLint lod, const glm::vec2& direction);

void BloomPassCombine(App* app, Framebuffer* fbo, GLenum colorAttachment, GLuint textureHandle, GLint maxLOD);

void LightPass(App* app);

void RenderLightMeshes(App* app);

void RenderTextureToScreen(App* app, GLuint textureHandle, bool isDepth);

void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

void CreateSphere(App* app);
void CreateQuad(App* app);

GLuint CreateModelFromMesh(App* app, GLuint meshIdx);

Mesh* CreateMesh(App* app, u32* index = nullptr);

glm::mat4 TransformScale(const glm::vec3& scaleFactors);
glm::mat4 TransformPositionScale(const glm::vec3& pos, const glm::vec3& scaleFactors);
glm::mat4 TransformPositionScaleRot(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scaleFactors);

u32 AddEntity(App* app, const char* name, u32 modelIndex);

Light* CreateDirectionalLight(App* app, vec3 color, vec3 direction);
Light* CreatePointLight(App* app, vec3 color, vec3 position);

GLuint GenerateColTex2D(vec2 displaySize);

GLuint GenerateColTex2DHighPrecision(vec2 displaySize);

GLuint GenerateDepthTex2D(vec2 displaySize);


u32 LoadProgram(App* app, const char* filepath, const char* programName);
