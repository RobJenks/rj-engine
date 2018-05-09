#pragma once

#include "PipelineStage.h"
#include "AssimpIntegration.h"

class PipelineStageAssimpTransform : public PipelineStage
{
public:

	PipelineStageAssimpTransform(unsigned int operations = AssimpIntegration::DefaultOperations());

	inline std::string						GetName(void) const { return "PipelineStageAssimpTransform"; }

	std::unique_ptr<ModelData>				Transform(std::unique_ptr<ModelData> model) const;

private:

	unsigned int							m_operations;

};