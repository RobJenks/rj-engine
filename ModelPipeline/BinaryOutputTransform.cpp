#include <iostream>
#include "../Definitions/ByteString.h"
#include "TransformPipelineOutput.h"
#include "BinaryOutputTransform.h"

ByteString BinaryOutputTransform::Transform(std::unique_ptr<ModelData> model) const
{ 
	ModelData *m = model.get();
	if (!m) return ByteString();

	return m->Serialize();
}

void BinaryOutputTransform::Transform(std::unique_ptr<ModelData> model, fs::path output_file) const
{
	TRANSFORM_INFO << "Serializing model data\n";
	ByteString output = Transform(std::move(model));

	TRANSFORM_INFO << "Writing serialized data to file [" << output.size() << " bytes / " << (output.size() / 1024U) << "kb]\n";
	if (!WriteDataToFile<ByteString::value_type>(output_file, output.data(), output.size()))
	{
		TRANSFORM_ERROR << "Failed to write transformed output to file\n";
		return;
	}
}