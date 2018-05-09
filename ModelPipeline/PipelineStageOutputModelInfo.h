#pragma once

#include "../Definitions/ModelData.h"
#include "PipelineStage.h"



class PipelineStageOutputModelInfo : public PipelineStage
{
public:

	inline std::string						GetName(void) const { return "PipelineStageOutputModelInfo"; }

	std::unique_ptr<ModelData>				Transform(std::unique_ptr<ModelData> model) const;



private:


};