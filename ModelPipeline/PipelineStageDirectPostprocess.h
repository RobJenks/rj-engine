#pragma once

#include <iostream>
#include "../Definitions/ModelData.h"
#include "PipelineStage.h"
#include "CustomPostProcess.h"



class PipelineStageDirectPostprocess : public PipelineStage
{
public:

	PipelineStageDirectPostprocess(void);
	PipelineStageDirectPostprocess(PostProcess operation_config, const std::string & model_path);


	inline std::string						GetName(void) const { return "PipelineStageDirectPostprocess"; }

	std::unique_ptr<ModelData>				ExecuteTransform(std::unique_ptr<ModelData> model) const;


private:

	PostProcess								m_operations;
	std::string								m_modelpath;


	bool									ReadCustomTransformFile(XMMATRIX & outTransform) const;
	void									ApplyCustomTransform(ModelData *model, const FXMMATRIX transform) const;
	void									TransformFloatVector(XMFLOAT3 & vectorReference, const FXMMATRIX transform) const;


};