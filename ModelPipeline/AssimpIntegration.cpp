#include <iostream>
#include "PipelineUtil.h"
#include "AssimpIntegration.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
using namespace DirectX;


std::unique_ptr<ModelData> AssimpIntegration::ParseAssimpScene(const aiScene *scene, Assimp::Importer & importer, bool debug_info)
{
	if (!scene) MODEL_INST_ERROR("Cannot build model; null ai data provided");

	auto model = std::make_unique<ModelData>();

	// Scene data may contain multiple meshes; we only expect & take the first for now
	// TODO: multiple meshes will be present in the scene if more than one material is used; 
	// a mesh always has a single material.  We can therefore use this mesh[] array breakdown
	// to correctly handle the multiple material case and generate a new model per component
	if (!scene->HasMeshes() || scene->mNumMeshes < 1U) MODEL_INST_ERROR("No mesh data is present");
	MODEL_INST_DEBUG("Model contains " << scene->mNumMeshes << " meshes");
	aiMesh *mesh = scene->mMeshes[0];

	// Verify mesh content and post-process if necessary
	if (mesh->mNumVertices == 0U || !mesh->mVertices) MODEL_INST_ERROR("No vertex data is present");
	if (!mesh->mNormals) MODEL_INST_ERROR("No normal data is present");
	if (!mesh->mTextureCoords) MODEL_INST_ERROR("No UV data is present");

	if (!mesh->mTangents || !mesh->mBitangents)
	{
		MODEL_INST_INFO("Mesh does not contain tangent and/or binormal data; post-processing to generate data");
		importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
		MODEL_INST_INFO("Tangent-space data calculated");
	}

	// Assign header data
	ModelData *m = model.get();
	m->VertexCount = mesh->mNumVertices;
	m->ModelMaterialIndex = mesh->mMaterialIndex;
	MODEL_INST_DEBUG("Mesh contains " << m->VertexCount << " vertices");
	MODEL_INST_DEBUG("Material index = " << m->ModelMaterialIndex);

	// Generate vertex data
	m->AllocateVertexData(m->VertexCount);
	size_t data_size = sizeof(ModelData::TVertex) * m->VertexCount;
	MODEL_INST_DEBUG("Allocating " << data_size << " bytes vertex data");

	MODEL_INST_INFO("Populating vertex data");
	XMFLOAT3 min_bounds = XMFLOAT3(+1e9, +1e9, +1e9);
	XMFLOAT3 max_bounds = XMFLOAT3(-1e9, -1e9, -1e9);

	for (unsigned int i = 0U; i < m->VertexCount; ++i)
	{
		// Read data into the vertex structure
		ModelData::TVertex & v = m->VertexData[i];
		v.position = GetFloat3(mesh->mVertices[i]);
		v.normal = GetFloat3(mesh->mNormals[i]);
		v.tangent = GetFloat3(mesh->mTangents[i]);
		v.binormal = GetFloat3(mesh->mBitangents[i]);

		// Perform other per-vertex calculation
		if (v.position.x < min_bounds.x)		min_bounds.x = v.position.x;
		else if (v.position.x > max_bounds.x)	max_bounds.x = v.position.x;
		if (v.position.y < min_bounds.y)		min_bounds.y = v.position.y;
		else if (v.position.y > max_bounds.y)	max_bounds.y = v.position.y;
		if (v.position.z < min_bounds.z)		min_bounds.z = v.position.z;
		else if (v.position.z > max_bounds.z)	max_bounds.z = v.position.z;
	}

	m->MinBounds = min_bounds;
	m->MaxBounds = max_bounds;
	m->ModelSize = PipelineUtil::Float3Subtract(max_bounds, min_bounds);
	m->CentrePoint = PipelineUtil::Float3ScalarMultiply(PipelineUtil::Float3Add(min_bounds, max_bounds), 0.5f);

	// Load texture data separately since components may not be available
	// Can have up to [8] texcoords; just take[0] for now
	if (mesh->HasTextureCoords(0U))
	{
		MODEL_INST_INFO("Loading texture coordinate data");
		for (unsigned int i = 0U; i < m->VertexCount; ++i)
		{
			// Is 3D so pull only first two coords.  Z = for volume textures.  Ignore for now.
			m->VertexData[i].tex = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
	}
	else
	{
		MODEL_INST_INFO("No texture coordinate data available");
	}


	// Model successfully created; return an owning pointer and quit
	MODEL_INST_INFO("Model data instantiated successfully");
	return model;
}



DirectX::XMFLOAT2 AssimpIntegration::GetFloat2(aiVector2D & vector)
{
	return XMFLOAT2(vector.x, vector.y);
}

DirectX::XMFLOAT3 AssimpIntegration::GetFloat3(aiVector3D & vector)
{
	return XMFLOAT3(vector.x, vector.y, vector.z);
}
