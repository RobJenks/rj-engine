#include <fstream>
#include <iostream>
#include <filesystem>
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

std::string TransformPipeline::Transform(std::string input_data) const
{
	std::unique_ptr<Model> model = m_input.get()->Transform(input_data);

	for (const auto & stage : m_stages)
	{
		model = stage->Transform(std::move(model));
	}

	std::string output = m_output.get()->Transform(std::move(model));
	return output;
}

std::string TransformPipeline::Transform(fs::path file) const
{
	std::unique_ptr<Model> model = m_input.get()->Transform(file);

	for (const auto & stage : m_stages)
	{
		model = stage->Transform(std::move(model));
	}

	std::string output = m_output.get()->Transform(std::move(model));
	return output;
}

void TransformPipeline::Transform(fs::path file, fs::path output_file) const
{
	std::unique_ptr<Model> model = m_input.get()->Transform(file);

	for (const auto & stage : m_stages)
	{
		model = stage->Transform(std::move(model));
	}

	m_output.get()->Transform(std::move(model), output_file);
}

void TransformPipeline::Transform(std::string input_data, fs::path output_file) const
{
	std::unique_ptr<Model> model = m_input.get()->Transform(input_data);

	for (const auto & stage : m_stages)
	{
		model = stage->Transform(std::move(model));
	}

	m_output.get()->Transform(std::move(model), output_file);
}
