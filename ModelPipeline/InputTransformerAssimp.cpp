#include <string>
#include <iostream>
#include "InputTransformerAssimp.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>


std::unique_ptr<Model> InputTransformerAssimp::Transform(const std::string & data) const
{
	if (data.empty())
	{
		TRANSFORM_ERROR << "No data passed to transformer\n";
		return NULL;
	}

	constexpr size_t data_stride = sizeof(std::string::value_type);

	// Attempt to import the data into an Assimp scene
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFileFromMemory(data.data(), data.length() * data_stride,
		aiProcess_ValidateDataStructure);

	// Build a model based upon this scene data
	auto model = Model::FromAssimpScene(scene);

	// Return the new model data.  Assimp scene data is owned by the importer and will be properly
	// deallocated once the importer goes out of scope
	return model;
}
