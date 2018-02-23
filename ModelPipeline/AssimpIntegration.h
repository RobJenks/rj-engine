#pragma once

#include <memory>
#include <DirectXMath.h>
#include "../Definitions/ModelData.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
using namespace DirectX;


class AssimpIntegration
{
public:

	static std::unique_ptr<ModelData>		ParseAssimpScene(const aiScene *scene, Assimp::Importer & importer, unsigned int operation_config, bool debug_info);

	static unsigned int						DefaultOperations(void);


private:

	// Convenience functions to convert assimp data
	static XMFLOAT2							GetFloat2(const aiVector2D & vector);
	static XMFLOAT3							GetFloat3(const aiVector3D & vector);
	

#	define MODEL_INST_INFO(msg)				{ std::cout << "Info [Model instantiation]: " << msg << "\n"; }
#	define MODEL_INST_DEBUG(msg)			{ if (debug_info) { std::cout << "Debug [Model instantiation]: " << msg << "\n"; } }
#	define MODEL_INST_ERROR(msg)			{ std::cerr << "Error [Model instantiation]: " << msg << "\n"; return NULL; }

};