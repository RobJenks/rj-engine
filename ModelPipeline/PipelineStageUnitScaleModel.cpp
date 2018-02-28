#include "PipelineUtil.h"
#include "PipelineStageUnitScaleModel.h"


std::unique_ptr<ModelData> PipelineStageUnitScaleModel::Transform(std::unique_ptr<ModelData> model) const
{
	ModelData *m = model.get();
	if (!m) return model;

	// No action required if the model is already unit-scale
	const float epsilon = 1e-6f;
	XMFLOAT3 unit_bounds_size = XMFLOAT3(1.0f, 1.0f, 1.0f);
	
	if (PipelineUtil::Float3Equal(m->ModelSize, unit_bounds_size, epsilon))
	{
		TRANSFORM_INFO << "Model is already unit-scale, no modifications applied\n";
		return model;
	}

	// Determine transformation required based on current model size
	float max_component = max(max(m->ModelSize.x, m->ModelSize.y), m->ModelSize.z);
	if (max_component == 0.0f)
	{
		TRANSFORM_ERROR << "Model size data is invalid; max size component is zero.  Cannot proceed\n";
		return model;
	}

	XMFLOAT3 required_scale = PipelineUtil::Float3Replicate(1.0f / max_component);
	TRANSFORM_INFO << "Model size is " << FLOAT3_STR(m->ModelSize) << "; model requires transformation to unit scale\n";
	TRANSFORM_INFO << "Applying scale factor of " << FLOAT3_STR(required_scale) << " to model geometry\n";

	// Apply transformation
	XMFLOAT3 min_bounds = XMFLOAT3(+1e6, +1e6, +1e6);
	XMFLOAT3 max_bounds = XMFLOAT3(-1e6, -1e6, -1e6);
	for (unsigned int i = 0U; i < m->VertexCount; ++i)
	{
		m->VertexData[i].position = PipelineUtil::Float3Multiply(m->VertexData[i].position, required_scale);
		min_bounds = PipelineUtil::Float3Min(min_bounds, m->VertexData[i].position);
		max_bounds = PipelineUtil::Float3Max(max_bounds, m->VertexData[i].position);
	}

	// Update derived data
	m->MinBounds = min_bounds;
	m->MaxBounds = max_bounds;
	m->ModelSize = PipelineUtil::Float3Subtract(max_bounds, min_bounds);

	// Report completion and verify data
	TRANSFORM_INFO << "Transformation complete; model bounds = " << FLOAT3_STR(m->MinBounds) << "-" << FLOAT3_STR(m->MaxBounds) << ", size = " << FLOAT3_STR(m->ModelSize) << "\n";
	if (!PipelineUtil::Float3Equal(m->ModelSize, unit_bounds_size, epsilon))
	{
		TRANSFORM_ERROR << "Model data does not appear to be correctly unit scale post-transformation; verify output\n";
	}

	return model;
}