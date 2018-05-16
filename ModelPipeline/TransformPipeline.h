#pragma once

#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include "../Definitions/ByteString.h"
#include "../Definitions/ModelData.h"
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
	ByteString Transform(std::string input_data) const;
	ByteString Transform(fs::path file) const;

	CMPINLINE bool									HasErrors(void) const { return m_has_errors; }

	CMPINLINE const ModelSizeProperties &			GetModelMetadata(void) const { return m_metadata; }
	CMPINLINE bool									HasModelMetadata(void) const { return (m_metadata.HasData()); }

private:

	void ExecuteTransform(std::vector<std::unique_ptr<ModelData>> && input, fs::path output_file) const;
	ByteString ExecuteTransformInMemory(std::unique_ptr<ModelData> input) const;

	void ResetPipelineState(void) const;

	CMPINLINE void ReportError(void) const { m_has_errors = true; }

private:

	std::unique_ptr<TransformPipelineInput>			m_input;
	std::vector<std::unique_ptr<PipelineStage>>		m_stages;
	std::unique_ptr<TransformPipelineOutput>		m_output;

	mutable bool									m_has_errors;
	mutable ModelSizeProperties						m_metadata;

	void											StoreModelMetadata(const ModelSizeProperties & metadata) const;

};