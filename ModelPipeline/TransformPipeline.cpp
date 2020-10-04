#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include "TransformPipeline.h"
#include "TransformPipelineInput.h"
#include "TransformPipelineOutput.h"
#include "PipelineStage.h"

namespace fs = std::filesystem;


TransformPipeline::TransformPipeline(
	std::unique_ptr<TransformPipelineInput> input_transform,
	std::unique_ptr<TransformPipelineOutput> output_transform,
	std::vector<std::unique_ptr<PipelineStage>> transforms)
	:
	m_input(std::move(input_transform)), 
	m_stages(std::move(transforms)), 
	m_output(std::move(output_transform)), 

	m_has_errors(false)
{
	std::cout << "Verifying transform pipeline\n";
	
	// Register input transform
	if (!m_input.get())
	{
		std::cerr << "Error: No input transformer detected\n";
		exit(1);
	}
	m_input.get()->RegisterParent(this);

	// Register each intermediate pipeline transform
	size_t stage_index = 0U;
	for (const auto & stage : m_stages)
	{
		if (!stage)
		{
			std::cerr << "Error: Pipeline stage " << stage_index << " is invalid\n";
			exit(1);
		}
		stage.get()->RegisterParent(this);
		++stage_index;
	}

	// Register output transform
	if (!m_output.get())
	{
		std::cerr << "Error: No output transformer detected\n";
		exit(1);
	}
	m_output.get()->RegisterParent(this);

	std::cout << "Transform pipeline initialised\n";
	std::cout << "Pipeline configuration: \"" << m_input.get()->GetName() << "\" -> ";
	for (const auto & stage : m_stages)
	{
		std::cout << "\"" << stage->GetName() << "\" -> ";
	}
	std::cout << "\"" << m_output.get()->GetName() << "\"\n\n";
}


void TransformPipeline::Transform(fs::path file, fs::path output_file) const
{
	ResetPipelineState();

	auto models = m_input.get()->Transform(file);
	if (m_input.get()->HasErrors()) ReportError();

	ExecuteTransform(std::move(models), output_file);
}

void TransformPipeline::Transform(std::string input_data, fs::path output_file) const
{
	ResetPipelineState();

	auto models = m_input.get()->Transform(input_data);
	if (m_input.get()->HasErrors()) ReportError();

	ExecuteTransform(std::move(models), output_file);
}

void TransformPipeline::ExecuteTransform(std::vector<std::unique_ptr<ModelData>> && input, fs::path output_file) const
{
	// Calculate a combined set of model metadata for the set of input models (since all originated from the 
	// same source, so all are components of the same overall model)
	std::vector<ModelSizeProperties> properties;
	for (const auto & item : input) { if (item.get()) properties.push_back(item.get()->SizeProperties); };
	StoreModelMetadata( ModelSizeProperties::Calculate(properties) );

	// Process each model in turn
	std::string original_filename = fs::absolute(output_file).string();
	for (size_t index = 0U; index < input.size(); ++index)
	{
		auto & model = input.at(index);

		// Process the mesh through each pipeline stage
		for (const auto & stage : m_stages)
		{
			model = stage->Transform(std::move(model));
			if (stage.get()->HasErrors()) ReportError();
		}

		// Construct a unique filename per mesh component, if the model contains >1
		if (input.size() != 1U)
		{
			std::ostringstream ss;
			ss << original_filename << "." << index;
			output_file = fs::path(ss.str());
		}

		// Output through the designated output transformer
		m_output.get()->Transform(std::move(model), output_file);
		if (m_output.get()->HasErrors()) ReportError();
	}
}

// In-memory transformation only supports single-mesh models.  Will process and return 
// the first mesh within a model if multiple are present
ByteString TransformPipeline::Transform(std::string input_data) const
{
	ResetPipelineState();

	auto models = m_input.get()->Transform(input_data);
	if (m_input.get()->HasErrors()) ReportError();

	if (models.empty()) { std::cerr << "Error: No model data loaded\n"; ReportError(); return ByteString(); }
	if (models.size() != 1U) { std::cout << "Warning: Model contains " << models.size() << "meshes; in-memory pipeline will process and return only the first mesh\n"; ReportError(); }

	return ExecuteTransformInMemory(std::move(models.at(0)));
}

// In-memory transformation only supports single-mesh models.  Will process and return 
// the first mesh within a model if multiple are present
ByteString TransformPipeline::Transform(fs::path file) const
{
	ResetPipelineState();

	auto models = m_input.get()->Transform(file);
	if (m_input.get()->HasErrors()) ReportError();

	if (models.empty()) { std::cerr << "Error: No model data loaded\n"; ReportError(); return ByteString(); }
	if (models.size() != 1U) { std::cout << "Warning: Model contains " << models.size() << "meshes; in-memory pipeline will process and return only the first mesh\n"; ReportError(); }

	return ExecuteTransformInMemory(std::move(models.at(0)));
}

ByteString TransformPipeline::ExecuteTransformInMemory(std::unique_ptr<ModelData> input) const
{
	for (const auto & stage : m_stages)
	{
		input = stage->Transform(std::move(input));
		if (stage.get()->HasErrors()) ReportError();
	}

	auto output = m_output.get()->Transform(std::move(input));
	if (m_output.get()->HasErrors()) ReportError();

	return output;
}

void TransformPipeline::StoreModelMetadata(const ModelSizeProperties & metadata) const
{
	m_metadata = metadata;
}

void TransformPipeline::ResetPipelineState(void) const 
{
	m_has_errors = false;
	m_metadata = ModelSizeProperties();
}
