#include <iostream>
#include "TransformPipelineOutput.h"
#include "BinaryOutputTransform.h"

std::string BinaryOutputTransform::Transform(std::unique_ptr<Model> model) const
{ 
	return "abc"; 
}

void BinaryOutputTransform::Transform(std::unique_ptr<Model> model, fs::path output_file) const
{
	std::string output = Transform(std::move(model));

	if (!WriteDataToFile<char>(output_file, output.data(), output.size()))
	{
		TRANSFORM_ERROR << "Failed to write transformed output to file\n";
		return;
	}
}