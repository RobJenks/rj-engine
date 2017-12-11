#pragma once

#include "../Definitions/ModelData.h"
#include "PipelineStage.h"

class PassthroughPipelineStage : public PipelineStage
{
public:

	inline std::string					GetName(void) const { return "PassthroughPipelineStage"; }

	std::unique_ptr<ModelData>				Transform(std::unique_ptr<ModelData> model) const
	{
		TRANSFORM_INFO << "Passing model data without modification\n";
		return model;
	}


private:


};