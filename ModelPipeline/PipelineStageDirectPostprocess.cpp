#include "PipelineStageDirectPostprocess.h"
#include "AssimpIntegration.h"
#include "PipelineUtil.h"
#include <filesystem>
#include <sstream>

namespace fs = std::experimental::filesystem;



PipelineStageDirectPostprocess::PipelineStageDirectPostprocess(void)
	:
	m_operations(AssimpIntegration::DefaultOperations()), 
	m_modelpath("")
{
}

PipelineStageDirectPostprocess::PipelineStageDirectPostprocess(PostProcess operation_config, const std::string & model_path)
	:
	m_operations(operation_config), 
	m_modelpath(model_path)
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

	// Apply any custom transformation that has been specified
	if ((m_operations & (PostProcess)CustomPostProcess::CustomTransform) == (PostProcess)CustomPostProcess::CustomTransform)
	{
		XMMATRIX transform;
		TRANSFORM_INFO << "Applying custom transformation to model geometry\n";
		if (!ReadCustomTransformFile(transform))
		{
			TRANSFORM_ERROR << "Aborting custom transform step in model pipeline\n";
		}
		else
		{
			ApplyCustomTransform(m, transform);
			TRANSFORM_INFO << "Custom transformation applied to model geometry\n";
		}
	}

	return model;
}



bool PipelineStageDirectPostprocess::ReadCustomTransformFile(XMMATRIX & outTransform) const
{
	if (m_modelpath == "")
	{
		TRANSFORM_ERROR << "Custom transform model path not specified; cannot continue\n";
		return false;
	}

	std::string transform_path = (m_modelpath + ".transform");

	if (!fs::exists(fs::path(transform_path)))
	{
		TRANSFORM_ERROR << "Cannot process custom transform; model transform file \"" << transform_path << "\" does not exist\n";
		return false;
	}

	fs::path file(transform_path);
	std::string data = PipelineUtil::ReadFileToString(file);

	if (data == "")
	{
		TRANSFORM_ERROR << "Failed to read custom transform data from \"" << file.string() << "\"\n";
		return false;
	}

	std::vector<std::string> comp;
	PipelineUtil::SplitString(data, ',', true, comp);

	if (comp.size() != 16U)
	{
		TRANSFORM_INFO << "Read " << comp.size() << " transform components from file; transform is invalid and cannot proceed\n";
		return false;
	}

	float c[16];
	std::ostringstream ss;
	for (unsigned int i = 0U; i < 16U; ++i)
	{
		c[i] = (float)atof(comp[i].c_str());
		ss << (i != 0U ? ", " : "") << c[i];
	}

	outTransform = XMMatrixSet(c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13], c[14], c[15]);
	TRANSFORM_INFO << "Using custom geometry transformation of [ " << ss.str() << "]\n";

	return true;
}

void PipelineStageDirectPostprocess::ApplyCustomTransform(ModelData *model, const FXMMATRIX transform) const
{
	if (!model) return;

	for (unsigned int i = 0U; i < model->VertexCount; ++i)
	{
		// Apply transformation to vertex { position, normal, binormal, tangent } vectors
		TransformFloatVector(model->VertexData[i].position, transform);
		TransformFloatVector(model->VertexData[i].normal, transform);
		TransformFloatVector(model->VertexData[i].binormal, transform);
		TransformFloatVector(model->VertexData[i].tangent, transform);
	}
}

void PipelineStageDirectPostprocess::TransformFloatVector(XMFLOAT3 & vectorReference, const FXMMATRIX transform) const
{
	XMVECTOR v = XMLoadFloat3(&vectorReference);
	XMStoreFloat3(&vectorReference, XMVector3TransformCoord(v, transform));
}