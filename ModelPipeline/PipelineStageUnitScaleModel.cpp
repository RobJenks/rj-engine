#include "PipelineUtil.h"
#include "PipelineStageUnitScaleModel.h"


std::unique_ptr<ModelData> PipelineStageUnitScaleModel::ExecuteTransform(std::unique_ptr<ModelData> model) const
{
	ModelData *m = model.get();
	if (!m) return model;

	// No action required if the model is already unit-scale
	const float epsilon = 1e-6f;
	if (PipelineUtil::Float3AnyEqual(m->ModelSize, 1.0f, epsilon))
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
	for (unsigned int i = 0U; i < m->VertexCount; ++i)
	{
		m->VertexData[i].position = PipelineUtil::Float3Multiply(m->VertexData[i].position, required_scale);
	}

	// Update derived data
	m->RecalculateDerivedData();

	// Report completion and verify data
	TRANSFORM_INFO << "Transformation complete; model = " << m->str() << "\n";
	if (!PipelineUtil::Float3AnyEqual(m->ModelSize, 1.0f, epsilon))
	{
		TRANSFORM_ERROR << "Model data does not appear to be correctly unit scale post-transformation; verify output\n";
	}

	return model;
}