#pragma once

#include "Model.h"
#include "PipelineStage.h"

class PassthroughPipelineStage : public PipelineStage
{
public:

	inline std::string					GetName(void) const { return "PassthroughPipelineStage"; }

	std::unique_ptr<Model>				Transform(std::unique_ptr<Model> model) const
	{
		TRANSFORM_INFO << "Passing model data without modification\n";
		return model;
	}


private:


};