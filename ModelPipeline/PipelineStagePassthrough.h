#pragma once

#include <iostream>
#include "../Definitions/ModelData.h"
#include "PipelineStage.h"

class PipelineStagePassthrough : public PipelineStage
{
public:

	inline std::string						GetName(void) const { return "PipelineStagePassthrough"; }

	std::unique_ptr<ModelData>				ExecuteTransform(std::unique_ptr<ModelData> model) const
	{
		TRANSFORM_INFO << "Passing model data without modification\n";
		return model;
	}


private:


};