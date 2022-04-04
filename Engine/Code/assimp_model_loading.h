#pragma once
#include "platform.h" //TODO dont include here just declare

struct Model
{
	u32 meshIdx;
	std::vector<u32> materialIdx;
};




void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory);

void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

u32 LoadModel(App* app, const char* filename);

