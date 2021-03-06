#include <iostream>
#include "PipelineUtil.h"
#include "AssimpIntegration.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
using namespace DirectX;


std::vector<std::unique_ptr<ModelData>> AssimpIntegration::ParseAssimpScene(const aiScene *scene, Assimp::Importer & importer, PostProcess operation_config, bool debug_info)
{
	if (!scene) MODEL_INST_ERROR("Cannot build model; null ai data provided");
	if (!importer.GetScene()) MODEL_INST_ERROR("Importer has no valid scene reference");
	if (scene != importer.GetScene()) MODEL_INST_ERROR("Importer scene reference does not match expected data");

	// Scene data may contain multiple meshes; process each mesh into a separate ModelData object
	if (!scene->HasMeshes() || scene->mNumMeshes < 1U) MODEL_INST_ERROR("No mesh data is present");
	MODEL_INST_DEBUG("Model contains " << scene->mNumMeshes << " mesh components");
	
	unsigned int meshcount = scene->mNumMeshes;
	std::vector<std::unique_ptr<ModelData>> result;
	for (unsigned int i = 0U; i < meshcount; ++i)
	{
		if (meshcount != 1U) MODEL_INST_INFO("Processing mesh component " << i << " of " << meshcount);

		aiMesh *mesh = scene->mMeshes[i];
		auto model = std::make_unique<ModelData>();

		// Verify mesh content and post-process if necessary
		if (mesh->mNumVertices == 0U || !mesh->mVertices) MODEL_INST_PER_MESH_ERROR("No vertex data is present");
		if (!mesh->mNormals) MODEL_INST_PER_MESH_ERROR("No normal data is present");
		if (!mesh->mTextureCoords) MODEL_INST_PER_MESH_ERROR("No UV data is present");

		// Tangent-space data can be calculated as a post-process if required
		if (!mesh->mTangents || !mesh->mBitangents)
		{
			MODEL_INST_INFO("Mesh does not contain tangent and/or binormal data");
			if (operation_config & aiPostProcessSteps::aiProcess_CalcTangentSpace)
			{
				scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
				if (mesh->mTangents && mesh->mBitangents)
				{
					MODEL_INST_INFO("Tangent-space data calculated");
				}
				else
				{
					MODEL_INST_INFO("WARNING: Calculation of tangent-space data appears to have failed, results may not be correct");
				}
			}
		}

		// Verify whether we have face data available; if not, we can only assume a sequential index buffer by default
		if (!mesh->HasFaces())
		{
			MODEL_INST_INFO("Warning: mesh does not contain face data; assuming sequential index buffer\n");
		}

		// Assign vertex header data
		ModelData *m = model.get();
		m->VertexCount = mesh->mNumVertices;
		m->IndexCount = mesh->mNumFaces * 3U;
		m->ModelMaterialIndex = mesh->mMaterialIndex;
		MODEL_INST_DEBUG("Mesh contains " << m->VertexCount << " vertices");
		MODEL_INST_DEBUG("Material index = " << m->ModelMaterialIndex);

		// Generate vertex data
		m->AllocateVertexData(m->VertexCount);
		size_t data_size = sizeof(ModelData::TVertex) * m->VertexCount;
		MODEL_INST_DEBUG("Allocating " << data_size << " bytes vertex data");

		MODEL_INST_INFO("Populating vertex data");
		for (unsigned int i = 0U; i < m->VertexCount; ++i)
		{
			// Read data into the vertex structure
			ModelData::TVertex & v = m->VertexData[i];
			v.position = GetFloat3(mesh->mVertices[i]);
			v.normal = GetFloat3(mesh->mNormals[i]);
			v.tangent = (mesh->mTangents ? GetFloat3(mesh->mTangents[i]) : XMFLOAT3(0.0f, 0.0f, 0.0f));
			v.binormal = (mesh->mBitangents ? GetFloat3(mesh->mBitangents[i]) : XMFLOAT3(0.0f, 0.0f, 0.0f));
		}

		// Update derived figures based on the new vertex data
		m->RecalculateDerivedData();

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

		// Verify whether we have face data available; if not, we can only assume a sequential index buffer by default
		if (!mesh->HasFaces())
		{
			m->IndexCount = m->VertexCount;
			MODEL_INST_INFO("Warning: mesh does not contain face data; assuming sequential index buffer (" << m->IndexCount << " indices)");
			m->AllocateIndexData(m->IndexCount);

			for (unsigned int i = 0U; i < m->IndexCount; ++i) m->IndexData[i] = i;
		}
		else
		{
			m->IndexCount = (mesh->mNumFaces * 3U);
			MODEL_INST_INFO("Mesh contains " << mesh->mNumFaces << " faces; transforming to " << m->IndexCount << " indices");
			m->AllocateIndexData(m->IndexCount);

			unsigned int ix = 0U;
			for (unsigned int i = 0U; i < mesh->mNumFaces; ++i)
			{
				if (mesh->mFaces[i].mNumIndices != 3U)
				{
					MODEL_INST_PER_MESH_ERROR("Mesh face " << i << " has " << mesh->mFaces[i].mNumIndices << " face indices.  Make sure mesh is correct triangulated");
				}

				m->IndexData[ix++] = mesh->mFaces[i].mIndices[0];
				m->IndexData[ix++] = mesh->mFaces[i].mIndices[1];
				m->IndexData[ix++] = mesh->mFaces[i].mIndices[2];
			}
		}

		result.emplace_back(std::move(model));
	}

	// Model successfully created; return an owning pointer and quit
	MODEL_INST_INFO("Model data instantiated successfully");
	return std::move(result);
}


PostProcess AssimpIntegration::DefaultOperations(void)
{
	return aiProcess_ValidateDataStructure;
}

DirectX::XMFLOAT2 AssimpIntegration::GetFloat2(const aiVector2D & vector)
{
	return XMFLOAT2(vector.x, vector.y);
}

DirectX::XMFLOAT3 AssimpIntegration::GetFloat3(const aiVector3D & vector)
{
	return XMFLOAT3(vector.x, vector.y, vector.z);
}
