#include <string>
#include <iostream>
#include "AssimpIntegration.h"
#include "InputTransformerAssimp.h"
#include "ModelPipelineConstants.h"
#include "PipelineUtil.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

InputTransformerAssimp::InputTransformerAssimp(void)
	:
	TransformPipelineInput()
{
	m_operations = AssimpIntegration::DefaultOperations();
}

InputTransformerAssimp::InputTransformerAssimp(unsigned int operations)
	:
	TransformPipelineInput()
{
	m_operations = operations;
}

std::vector<std::unique_ptr<ModelData>> InputTransformerAssimp::ExecuteTransform(const std::string & data) const
{
	// Save data to a temporary file, then process as normal and clean up the temporary file
	fs::path file = PipelineUtil::NewTemporaryFile("obj");
	PipelineUtil::WriteDataTofile(file, data);
	auto model_data = ExecuteTransform(file);
	PipelineUtil::DeleteTemporaryFile(file);

	return model_data;
}

std::vector<std::unique_ptr<ModelData>> InputTransformerAssimp::ExecuteTransform(fs::path file) const
{
	// Attempt to import the data into an Assimp scene
	TRANSFORM_INFO << "Attempting to import data via assimp\n";
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(fs::absolute(file).string(), m_operations);
	
	// Log any error that may have been reported
	if (!scene)
	{
		TRANSFORM_ERROR << "Transformation error: " << importer.GetErrorString() << "\n";
	}

	// Build a model based upon this scene data
	TRANSFORM_INFO << "Data loaded" << (scene == NULL ? " (WARNING: null scene data, likely import failure)" : "") << ", building model from ai scene data\n";
	auto models = AssimpIntegration::ParseAssimpScene(scene, importer, m_operations, (ModelPipelineConstants::LogLevel != ModelPipelineConstants::LoggingType::Normal));

	// Return the new model data.  Assimp scene data is owned by the importer and will be properly
	// deallocated once the importer goes out of scope
	TRANSFORM_INFO << "Returning imported data" << (models.empty() ? " (WARNING: null model data, likely import failure)" : "") << "\n";
	return models;
}



