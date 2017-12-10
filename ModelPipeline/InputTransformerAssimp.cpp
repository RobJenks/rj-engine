#include <string>
#include <iostream>
#include "InputTransformerAssimp.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>


std::unique_ptr<Model> InputTransformerAssimp::Transform(const std::string & data) const
{
	// Not (currently) implemented; should just save to a tmp file and use the fs::path method, 
	// to preserve the file hints and ability of assimp to process file dependencies in the same 
	// directory (though this won't work in the case of a single string input)
	TRANSFORM_ERROR << "Transform from string is not implemented\n";
	return NULL;
}

std::unique_ptr<Model> InputTransformerAssimp::Transform(fs::path file) const
{
	// Attempt to import the data into an Assimp scene
	TRANSFORM_INFO << "Attempting to import data via assimp\n";
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(fs::absolute(file).string(), 
		aiProcess_ValidateDataStructure);

	// Build a model based upon this scene data
	TRANSFORM_INFO << "Data loaded" << (scene == NULL ? " (WARNING: null scene data, likely import failure)" : "") << ", building model from ai scene data\n";
	auto model = Model::FromAssimpScene(scene);

	// Return the new model data.  Assimp scene data is owned by the importer and will be properly
	// deallocated once the importer goes out of scope
	TRANSFORM_INFO << "Returning imported data" << (model == NULL ? " (WARNING: null model data, likely import failure)" : "") << "\n";
	return model;
}
