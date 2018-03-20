#pragma once

#include <iostream>
#include "../Definitions/ModelData.h"
#include "PipelineStage.h"
#include "CustomPostProcess.h"



class PipelineStageDirectPostprocess : public PipelineStage
{
public:

	PipelineStageDirectPostprocess(void);
	PipelineStageDirectPostprocess(PostProcess operation_config);


	inline std::string						GetName(void) const { return "PipelineStageDirectPostprocess"; }

	std::unique_ptr<ModelData>				Transform(std::unique_ptr<ModelData> model) const;


private:

	PostProcess								m_operations;


};