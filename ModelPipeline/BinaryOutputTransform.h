#pragma once

#include "Model.h"
#include "TransformPipelineOutput.h"

class BinaryOutputTransform : public TransformPipelineOutput
{
public:

	inline std::string	GetName(void) const { return "BinaryOutputTransform"; }

	std::string			Transform(std::unique_ptr<Model> model) const;
	void				Transform(std::unique_ptr<Model> model, fs::path output_file) const;

private:


};