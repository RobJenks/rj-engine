#include <string>
#include <iostream>
#include "AssimpIntegration.h"
#include "InputTransformerAssimp.h"
#include "PipelineUtil.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

InputTransformerAssimp::InputTransformerAssimp(void)
	:
	TransformPipelineInput()
{
	m_operations = DefaultOperations();
}

InputTransformerAssimp::InputTransformerAssimp(unsigned int operations)
	:
	TransformPipelineInput()
{
	m_operations = operations;
}

std::unique_ptr<ModelData> InputTransformerAssimp::Transform(const std::string & data) const
{
	// Save data to a temporary file, then process as normal and clean up the temporary file
	fs::path file = PipelineUtil::NewTemporaryFile();
	auto model_data = Transform(file);
	PipelineUtil::DeleteTemporaryFile(file);

	return model_data;
}

std::unique_ptr<ModelData> InputTransformerAssimp::Transform(fs::path file) const
{
	// Attempt to import the data into an Assimp scene
	TRANSFORM_INFO << "Attempting to import data via assimp\n";
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(fs::absolute(file).string(), m_operations);
	
	// Build a model based upon this scene data
	TRANSFORM_INFO << "Data loaded" << (scene == NULL ? " (WARNING: null scene data, likely import failure)" : "") << ", building model from ai scene data\n";
	auto model = AssimpIntegration::ParseAssimpScene(scene, importer, m_operations, true);

	// Return the new model data.  Assimp scene data is owned by the importer and will be properly
	// deallocated once the importer goes out of scope
	TRANSFORM_INFO << "Returning imported data" << (model == NULL ? " (WARNING: null model data, likely import failure)" : "") << "\n";
	return model;
}

unsigned int InputTransformerAssimp::DefaultOperations(void)
{
	return aiProcess_ValidateDataStructure;
}


