#include "PipelineUtil.h"
#include "TransformPipeline.h"
#include "../Definitions/ModelSizeProperties.h"
#include "PipelineStageUnitScaleModel.h"


std::unique_ptr<ModelData> PipelineStageUnitScaleModel::ExecuteTransform(std::unique_ptr<ModelData> model) const
{
	ModelData *m = model.get();
	if (!m) return model;

	// Use pre-calculated metadata for the whole model if this is a multi-mesh model; otherwise get directly from model data
	ModelSizeProperties prop = m->SizeProperties;
	if (GetParent() && GetParent()->HasModelMetadata())
	{
		prop = GetParent()->GetModelMetadata();
	}

	// No action required if the model is already unit-scale
	const float epsilon = 1e-6f;
	if (PipelineUtil::Float3LessOrEqual(prop.ModelSize, XMFLOAT3(1.0f, 1.0f, 1.0f)) && 
		PipelineUtil::Float3AnyEqual(prop.ModelSize, 1.0f, epsilon))
	{
		TRANSFORM_INFO << "Model is already unit-scale, no modifications applied\n";
		return model;
	}

	// Determine transformation required based on current model size
	float max_component = max(max(prop.ModelSize.x, prop.ModelSize.y), prop.ModelSize.z);
	if (max_component == 0.0f)
	{
		TRANSFORM_ERROR << "Model size data is invalid; max size component is zero.  Cannot proceed\n";
		return model;
	}

	XMFLOAT3 required_scale = PipelineUtil::Float3Replicate(1.0f / max_component);
	TRANSFORM_INFO << "Model size is " << FLOAT3_STR(prop.ModelSize) << "; model requires transformation to unit scale\n";
	TRANSFORM_INFO << "Applying scale factor of " << FLOAT3_STR(required_scale) << " to model geometry\n";

	// Apply transformation
	for (unsigned int i = 0U; i < m->VertexCount; ++i)
	{
		m->VertexData[i].position = PipelineUtil::Float3Multiply(m->VertexData[i].position, required_scale);
	}

	// Update derived data
	m->RecalculateDerivedData();

	// Report completion
	TRANSFORM_INFO << "Transformation complete, unit scale applied; model = " << m->str() << "\n";
	return model;
}