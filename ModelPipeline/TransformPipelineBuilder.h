#pragma once

#include <vector>
#include <memory>
#include "TransformPipeline.h"
#include "TransformPipelineInput.h"
#include "TransformPipelineOutput.h"
#include "PipelineStage.h"

class TransformPipelineBuilder
{
public:

	TransformPipelineBuilder(void);

	TransformPipelineBuilder &			WithInputTransformer(std::unique_ptr<TransformPipelineInput> transformer);
	TransformPipelineBuilder &			WithOutputTransformer(std::unique_ptr<TransformPipelineOutput> transformer);
	TransformPipelineBuilder &			WithPipelineStage(std::unique_ptr<PipelineStage> transformer);

	std::unique_ptr<TransformPipeline>	Build(void);

	~TransformPipelineBuilder(void);

private:

	std::unique_ptr<TransformPipelineInput>					m_input;
	std::unique_ptr<TransformPipelineOutput>				m_output;
	std::vector<std::unique_ptr<PipelineStage>>				m_stages;

};