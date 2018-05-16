#include <iostream>
#include "PipelineUtil.h"
#include "PipelineStageOutputModelInfo.h"

std::unique_ptr<ModelData> PipelineStageOutputModelInfo::ExecuteTransform(std::unique_ptr<ModelData> model) const
{
	ModelData *m = model.get();
	if (!m) return model;

	TRANSFORM_INFO << "Model vertex count = " << m->VertexCount << ", index count = " << m->IndexCount << "\n";
	TRANSFORM_INFO << "Model centre point = " << FLOAT3_STR(m->SizeProperties.CentrePoint) << "\n";
	TRANSFORM_INFO << "Model size = " << FLOAT3_STR(m->SizeProperties.ModelSize) << "\n";
	TRANSFORM_INFO << "Model bounds = " << FLOAT3_STR(m->SizeProperties.MinBounds) << " to " << FLOAT3_STR(m->SizeProperties.MaxBounds) << "\n";

	// No change to the model data itself; this is a simple read-only 'transformation'
	return model;
}