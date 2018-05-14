#pragma once

#include "../Definitions/ModelData.h"
#include "TransformPipelineOutput.h"
#include "../Definitions/ByteString.h"

class BinaryOutputTransform : public TransformPipelineOutput
{
public:

	inline std::string	GetName(void) const { return "BinaryOutputTransform"; }

	ByteString			ExecuteTransform(std::unique_ptr<ModelData> model) const;
	void				ExecuteTransform(std::unique_ptr<ModelData> model, fs::path output_file) const;

private:


};