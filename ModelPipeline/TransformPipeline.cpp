#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include "TransformPipeline.h"
#include "TransformPipelineInput.h"
#include "TransformPipelineOutput.h"
#include "PipelineStage.h"

namespace fs = std::experimental::filesystem;


TransformPipeline::TransformPipeline(
	std::unique_ptr<TransformPipelineInput> input_transform,
	std::unique_ptr<TransformPipelineOutput> output_transform,
	std::vector<std::unique_ptr<PipelineStage>> transforms)
	:
	m_input(std::move(input_transform)), 
	m_stages(std::move(transforms)), 
	m_output(std::move(output_transform))
{
	std::cout << "Verifying transform pipeline\n";
	
	if (!m_input.get())
	{
		std::cerr << "Error: No input transformer detected\n";
		exit(1);
	}

	size_t stage_index = 0U;
	for (const auto & stage : m_stages)
	{
		if (!stage)
		{
			std::cerr << "Error: Pipeline stage " << stage_index << " is invalid\n";
			exit(1);
		}
		++stage_index;
	}

	if (!m_output.get())
	{
		std::cerr << "Error: No output transformer detected\n";
		exit(1);
	}

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
	auto models = m_input.get()->Transform(file);

	ExecuteTransform(std::move(models), output_file);
}

void TransformPipeline::Transform(std::string input_data, fs::path output_file) const
{
	auto models = m_input.get()->Transform(input_data);

	ExecuteTransform(std::move(models), output_file);
}

void TransformPipeline::ExecuteTransform(std::vector<std::unique_ptr<ModelData>> && input, fs::path output_file) const
{
	for (size_t index = 0U; index < input.size(); ++index)
	{
		auto & model = input.at(index);

		// Process the mesh through each pipeline stage
		for (const auto & stage : m_stages)
		{
			model = stage->Transform(std::move(model));
		}

		// Construct a unique filename per mesh component, if the model contains >1
		if (input.size() != 1U)
		{
			std::ostringstream ss;
			ss << fs::absolute(output_file).string() << "." << index;
			output_file = fs::path(ss.str());
		}

		// Output through the designated output transformer
		m_output.get()->Transform(std::move(model), output_file);
	}
}

// In-memory transformation only supports single-mesh models.  Will process and return 
// the first mesh within a model if multiple are present
ByteString TransformPipeline::Transform(std::string input_data) const
{
	auto models = m_input.get()->Transform(input_data);

	if (models.empty()) { std::cerr << "Error: No model data loaded\n"; return ByteString(); }
	if (models.size() != 1U) { std::cout << "Warning: Model contains " << models.size() << "meshes; in-memory pipeline will process and return only the first mesh\n"; }

	return ExecuteTransformInMemory(std::move(models.at(0)));
}

// In-memory transformation only supports single-mesh models.  Will process and return 
// the first mesh within a model if multiple are present
ByteString TransformPipeline::Transform(fs::path file) const
{
	auto models = m_input.get()->Transform(file);

	if (models.empty()) { std::cerr << "Error: No model data loaded\n"; return ByteString(); }
	if (models.size() != 1U) { std::cout << "Warning: Model contains " << models.size() << "meshes; in-memory pipeline will process and return only the first mesh\n"; }

	return ExecuteTransformInMemory(std::move(models.at(0)));
}

ByteString TransformPipeline::ExecuteTransformInMemory(std::unique_ptr<ModelData> input) const
{
	for (const auto & stage : m_stages)
	{
		input = stage->Transform(std::move(input));
	}

	auto output = m_output.get()->Transform(std::move(input));
	return output;
}