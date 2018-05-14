#include <iostream>
#include "InputTransformerRjm.h"
#include "PipelineUtil.h"


std::vector<std::unique_ptr<ModelData>> InputTransformerRjm::ExecuteTransform(fs::path file) const
{
	if (!fs::exists(file)) return {};

	// Read file contents
	TRANSFORM_INFO << "Reading binary RJM data\n";
	ByteString data = PipelineUtil::ReadBinaryFile(file);
	
	if (data.empty())
	{
		TRANSFORM_ERROR << "Failed to read binary RJM data; data may be invalid\n";
		return {};
	}

	// Deserialise into the target model data structure
	TRANSFORM_INFO << "Attempting to deserialise loaded model data\n";
	auto geometry = ModelData::Deserialize(data);
	if (geometry.get() == NULL)
	{
		TRANSFORM_ERROR << "Failed to deserialise model data; make sure that this is a valid RJM file\n";
		return {};
	}

	// Return the deserialised model data
	TRANSFORM_INFO << "Successfully deserialised model data (" << geometry.get()->VertexCount << " vertices)\n";
	std::vector<std::unique_ptr<ModelData>> result;
	result.emplace_back(std::move(geometry));
	return std::move(result);
}

std::vector<std::unique_ptr<ModelData>> InputTransformerRjm::ExecuteTransform(const std::string & data) const
{
	fs::path file = PipelineUtil::SaveToNewTemporaryFile(data, "rjm");
	auto result = ExecuteTransform(file);
	PipelineUtil::DeleteTemporaryFile(file);

	return result;
}