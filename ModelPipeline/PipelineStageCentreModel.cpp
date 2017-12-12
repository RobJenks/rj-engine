#include "PipelineUtil.h"
#include "PipelineStageCentreModel.h"

std::unique_ptr<ModelData> PipelineStageCentreModel::Transform(std::unique_ptr<ModelData> model) const
{
	ModelData *m = model.get();
	if (!m) return model;

	// We need to take no action if the model is centred (or very, very close, to accomodate FP errors)
	const float epsilon = 1e-6f;
	if (fabs(m->CentrePoint.x) < epsilon && fabs(m->CentrePoint.y) < epsilon && fabs(m->CentrePoint.z) < epsilon)
	{
		TRANSFORM_INFO << "Model is already centred about origin, no modifications applied\n";
		return model;
	}

	// Apply a linear offset to all vertices to centre them about the origin
	TRANSFORM_INFO << "Model is centred about [" << m->CentrePoint.x << ", " << m->CentrePoint.y << ", " << m->CentrePoint.z << "], applying offset\n";
	XMFLOAT3 offset = XMFLOAT3(-(m->CentrePoint.x), -(m->CentrePoint.y), -(m->CentrePoint.z));
	for (unsigned int i = 0U; i < m->VertexCount; ++i)
	{
		m->VertexData[i].position = PipelineUtil::Float3Add(m->VertexData[i].position, offset);
	}

	// Also update derived figures; we could recalculate, but the results are provably as below
	m->MinBounds = PipelineUtil::Float3Add(m->MinBounds, offset);
	m->MaxBounds = PipelineUtil::Float3Add(m->MaxBounds, offset);
	m->CentrePoint = PipelineUtil::Float3Add(m->CentrePoint, offset);

	// Return success
	TRANSFORM_INFO << "Model centre offset applied\n";
	return model;
}