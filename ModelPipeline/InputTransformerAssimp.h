#pragma once

#include "Model.h"
#include "TransformPipelineInput.h"

class InputTransformerAssimp : public TransformPipelineInput
{
public:

	inline std::string			GetName(void) const { return "InputTransformAssimp"; }

	std::unique_ptr<Model>		Transform(const std::string & data) const;



private:



};