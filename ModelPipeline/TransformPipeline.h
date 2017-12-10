#pragma once

#include <memory>
#include <string>
#include <vector>
#include <filesystem>
class TransformPipelineInput;
class TransformPipelineOutput;
class PipelineStage;
namespace fs = std::experimental::filesystem;

class TransformPipeline
{
public:

	TransformPipeline(
		std::unique_ptr<TransformPipelineInput> input_transform,
		std::unique_ptr<TransformPipelineOutput> output_transform,
		std::vector<std::unique_ptr<PipelineStage>> transforms
	);



	void Transform(fs::path file, fs::path output_file) const;
	void Transform(std::string input_data, fs::path output_file) const;
	std::string Transform(std::string input_data) const;
	std::string Transform(fs::path file) const;

	


private:

	std::unique_ptr<TransformPipelineInput>			m_input;
	std::vector<std::unique_ptr<PipelineStage>>		m_stages;
	std::unique_ptr<TransformPipelineOutput>		m_output;

};