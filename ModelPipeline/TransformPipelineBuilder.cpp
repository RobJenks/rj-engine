#include "TransformPipelineBuilder.h"


TransformPipelineBuilder::TransformPipelineBuilder(void)
{
}

TransformPipelineBuilder & TransformPipelineBuilder::WithInputTransformer(std::unique_ptr<TransformPipelineInput> transformer)
{
	m_input = std::move(transformer);
	return *this;
}

TransformPipelineBuilder & TransformPipelineBuilder::WithOutputTransformer(std::unique_ptr<TransformPipelineOutput> transformer)
{
	m_output = std::move(transformer);
	return *this;
}

TransformPipelineBuilder & TransformPipelineBuilder::WithPipelineStage(std::unique_ptr<PipelineStage> transformer)
{
	m_stages.push_back(std::move(transformer));
	return *this;
}

std::unique_ptr<TransformPipeline> TransformPipelineBuilder::Build(void)
{
	auto pipeline = std::make_unique<TransformPipeline>(std::move(m_input), std::move(m_output), std::move(m_stages));
	return pipeline;
}

TransformPipelineBuilder::~TransformPipelineBuilder(void)
{

}