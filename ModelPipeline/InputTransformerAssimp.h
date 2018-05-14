#pragma once

#include "../Definitions/ModelData.h"
#include "TransformPipelineInput.h"


class InputTransformerAssimp : public TransformPipelineInput
{
public:

	InputTransformerAssimp(void);
	InputTransformerAssimp(unsigned int operations);

	inline std::string									GetName(void) const { return "InputTransformAssimp"; }

	virtual std::vector<std::unique_ptr<ModelData>>		ExecuteTransform(fs::path file) const;
	virtual std::vector<std::unique_ptr<ModelData>>		ExecuteTransform(const std::string & data) const;

	
private:

	unsigned int m_operations;

};