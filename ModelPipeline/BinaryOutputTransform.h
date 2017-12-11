#pragma once

#include "../Definitions/ModelData.h"
#include "TransformPipelineOutput.h"
#include "../Definitions/ByteString.h"

class BinaryOutputTransform : public TransformPipelineOutput
{
public:

	inline std::string	GetName(void) const { return "BinaryOutputTransform"; }

	ByteString			Transform(std::unique_ptr<ModelData> model) const;
	void				Transform(std::unique_ptr<ModelData> model, fs::path output_file) const;

private:


};