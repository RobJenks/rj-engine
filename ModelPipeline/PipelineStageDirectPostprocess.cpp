#include "PipelineStageDirectPostprocess.h"
#include "AssimpIntegration.h"


PipelineStageDirectPostprocess::PipelineStageDirectPostprocess(void)
	:
	m_operations(AssimpIntegration::DefaultOperations())
{
}

PipelineStageDirectPostprocess::PipelineStageDirectPostprocess(PostProcess operation_config)
	:
	m_operations(operation_config)
{
}

std::unique_ptr<ModelData> PipelineStageDirectPostprocess::Transform(std::unique_ptr<ModelData> model) const
{
	ModelData *m = model.get();
	if (!m) return model;

	// Invert U and/or V of UV mapping.  Loop twice for 2N operations instead of 2N comparisons within a single loop
	if ((m_operations & (PostProcess)CustomPostProcess::InvertU) == (PostProcess)CustomPostProcess::InvertU)
	{
		TRANSFORM_INFO << "Inverting U coordinates of model UV mappings\n";
		for (unsigned int i = 0U; i < m->VertexCount; ++i)
		{
			m->VertexData[i].tex.x = (1.0f - m->VertexData[i].tex.x);
		}
	}

	if ((m_operations & (PostProcess)CustomPostProcess::InvertV) == (PostProcess)CustomPostProcess::InvertV)
	{
		TRANSFORM_INFO << "Inverting V coordinates of model UV mappings\n";
		for (unsigned int i = 0U; i < m->VertexCount; ++i)
		{
			m->VertexData[i].tex.y = (1.0f - m->VertexData[i].tex.y);
		}
	}


	return model;
}