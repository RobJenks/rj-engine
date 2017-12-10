#pragma once

#include <memory>
#include <DirectXMath.h>
#include <assimp\scene.h>
#include <assimp\Importer.hpp>
#include "../Definitions/VertexDefinitions.hlsl.h"
using namespace DirectX;

class Model
{
public:

	typedef Vertex_Standard TVertex;

	Model(void);
	~Model(void);

	static std::unique_ptr<Model>		FromAssimpScene(const aiScene *scene, Assimp::Importer & importer, bool debug_info = false);

private:

	// Header data
	unsigned int						ModelMaterialIndex;		// Note: not the same as MaterialId.  This is the index of materials loaded within this single model
	unsigned int						VertexCount;			// Determines total size of the VertexData array

	// Model data
	Vertex_Standard *					VertexData;				// Array of per-vertex data only; no instance data included



#	define MODEL_INST_INFO(msg)			{ std::cout << "Info [Model instantiation]: " << msg << "\n"; }
#	define MODEL_INST_DEBUG(msg)		{ if (debug_info) { std::cout << "Debug [Model instantiation]: " << msg << "\n"; } }
#	define MODEL_INST_ERROR(msg)		{ std::cerr << "Error [Model instantiation]: " << msg << "\n"; return NULL; }

	// Convenience functions to convert assimp data
	static XMFLOAT2						GetFloat2(aiVector2D & vector);
	static XMFLOAT3						GetFloat3(aiVector3D & vector);
	

};