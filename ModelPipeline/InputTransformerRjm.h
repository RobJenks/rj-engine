#pragma once

#include "TransformPipelineInput.h"


class InputTransformerRjm : public TransformPipelineInput
{
public:

	inline std::string									GetName(void) const { return "InputTransformRjm"; }

	virtual std::vector<std::unique_ptr<ModelData>>		Transform(fs::path file) const;
	virtual std::vector<std::unique_ptr<ModelData>>		Transform(const std::string & data) const;

private:


};