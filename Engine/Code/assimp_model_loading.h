#pragma once
#include "platform.h"
#include <glad/glad.h>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct App;
struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiNode;

struct VertexBufferAttribute;
struct VertexBufferLayout;
struct VertexShaderAttribute;
struct VertexShaderLayout;
struct Vao;
struct Model;
struct Submesh;
struct Mesh;
struct Material;

void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory);

void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

u32 LoadModel(App* app, const char* filename);


