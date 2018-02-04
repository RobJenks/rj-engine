#pragma once

#include "../Definitions/ModelData.h"
#include "TransformPipelineInput.h"


class InputTransformerAssimp : public TransformPipelineInput
{
public:

	inline std::string						GetName(void) const { return "InputTransformAssimp"; }

	virtual std::unique_ptr<ModelData>		Transform(fs::path file) const;
	virtual std::unique_ptr<ModelData>		Transform(const std::string & data) const;

	
private:



};