#include <iostream>
#include <fstream>
#include "TransformPipelineOutput.h"

// Default implementation to write data into an external file; can be used by output 
// pipeline implementations if there are no specific processing requirements.  Writes
// the specified number of bytes (i.e. sizeof(type) * count(type) data at the given 
// buffer pointer to file

bool TransformPipelineOutput::WriteBufferDataToFile(fs::path file, const void *data, size_t buffer_size) const
{
	if (!data)
	{
		TRANSFORM_ERROR << "Invalid data provided to default file output generator\n";
		return false;
	}

	TRANSFORM_INFO << "Opening output file\n";

	std::string output_file_name = fs::absolute(file).string();
	std::fstream output_file(output_file_name.c_str(), std::ios::out | std::ios::binary);
	if (!output_file.is_open())
	{
		TRANSFORM_ERROR << "Could not open output file \"" << output_file_name << "\" for writing\n";
		return false;
	}

	TRANSFORM_INFO << "Writing transformed data to output file \"" << output_file_name << "\"\n";
	output_file.write((const char *)data, buffer_size);
	output_file.close();

	TRANSFORM_INFO << "Output file written successfully\n";
	return true;
}
