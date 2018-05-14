#include <iostream>
#include "../Definitions/ByteString.h"
#include "TransformPipelineOutput.h"
#include "BinaryOutputTransform.h"

ByteString BinaryOutputTransform::ExecuteTransform(std::unique_ptr<ModelData> model) const
{ 
	ModelData *m = model.get();
	if (!m) return ByteString();

	return m->Serialize();
}

void BinaryOutputTransform::ExecuteTransform(std::unique_ptr<ModelData> model, fs::path output_file) const
{
	if (!model.get())
	{
		TRANSFORM_ERROR << "No valid model data provided, terminating output stage\n";
		return;
	}

	TRANSFORM_INFO << "Serializing model data\n";
	ByteString output = ExecuteTransform(std::move(model));

	TRANSFORM_INFO << "Writing serialized data to file [" << output.size() << " bytes / " << (output.size() / 1024U) << "kb]\n";
	if (!WriteDataToFile<ByteString::value_type>(output_file, output.data(), output.size()))
	{
		TRANSFORM_ERROR << "Failed to write transformed output to file\n";
		return;
	}
}