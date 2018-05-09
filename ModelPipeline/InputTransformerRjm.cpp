#include <iostream>
#include "InputTransformerRjm.h"
#include "PipelineUtil.h"


std::unique_ptr<ModelData> InputTransformerRjm::Transform(fs::path file) const
{
	if (!fs::exists(file)) return NULL;

	// Read file contents
	TRANSFORM_INFO << "Reading binary RJM data\n";
	ByteString data = PipelineUtil::ReadBinaryFile(file);
	
	if (data.empty())
	{
		TRANSFORM_ERROR << "Failed to read binary RJM data; data may be invalid\n";
		return NULL;
	}

	// Deserialise into the target model data structure
	TRANSFORM_INFO << "Attempting to deserialise loaded model data\n";
	auto geometry = ModelData::Deserialize(data);
	if (geometry.get() == NULL)
	{
		TRANSFORM_ERROR << "Failed to deserialise model data; make sure that this is a valid RJM file\n";
		return NULL;
	}

	// Return the deserialised model data
	TRANSFORM_INFO << "Successfully deserialised model data (" << geometry.get()->VertexCount << " vertices)\n";
	return geometry;
}

std::unique_ptr<ModelData> InputTransformerRjm::Transform(const std::string & data) const
{
	fs::path file = PipelineUtil::SaveToNewTemporaryFile(data, "rjm");
	auto result = Transform(file);
	PipelineUtil::DeleteTemporaryFile(file);

	return result;
}