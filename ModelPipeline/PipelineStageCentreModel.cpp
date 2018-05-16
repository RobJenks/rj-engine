#include "PipelineUtil.h"
#include "PipelineStageCentreModel.h"
#include "../Definitions/ModelSizeProperties.h"
#include "TransformPipeline.h"

std::unique_ptr<ModelData> PipelineStageCentreModel::ExecuteTransform(std::unique_ptr<ModelData> model) const
{
	ModelData *m = model.get();
	if (!m) return model;

	// Use pre-calculated metadata for the whole model if this is a multi-mesh model; otherwise get directly from model data
	ModelSizeProperties prop = m->SizeProperties;
	if (GetParent() && GetParent()->HasModelMetadata())
	{
		prop = GetParent()->GetModelMetadata();
	}

	// We need to take no action if the model is centred (or very, very close, to accomodate FP errors)
	const float epsilon = 1e-6f;
	if (PipelineUtil::Float3Equal(prop.CentrePoint, XMFLOAT3(0.0f, 0.0f, 0.0f), epsilon))
	{
		TRANSFORM_INFO << "Model is already centred about origin, no modifications applied\n";
		return model;
	}

	// Apply a linear offset to all vertices to centre them about the origin
	TRANSFORM_INFO << "Model is centred about " << FLOAT3_STR(prop.CentrePoint) << ", applying offset\n";
	XMFLOAT3 offset = XMFLOAT3(-(prop.CentrePoint.x), -(prop.CentrePoint.y), -(prop.CentrePoint.z));
	for (unsigned int i = 0U; i < m->VertexCount; ++i)
	{
		m->VertexData[i].position = PipelineUtil::Float3Add(m->VertexData[i].position, offset);
	}

	// Update derived figures based on the new vertex data
	m->RecalculateDerivedData();

	// Return success
	TRANSFORM_INFO << "Transformation complete, centre offset applied; model = " << m->str() << "\n";
	return model;
}

