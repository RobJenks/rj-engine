#pragma once

#include <string>
#include <memory>
#include  <filesystem>
#include "TransformerComponent.h"
#include "Model.h"
namespace fs = std::experimental::filesystem;


class TransformPipelineOutput : public TransformerComponent
{
public:

	virtual std::string			Transform(std::unique_ptr<Model> model) const = 0;

	virtual void				Transform(std::unique_ptr<Model> model, fs::path output_file) const = 0;


protected:

	// Default implementation to write data into an external file; can be used by output 
	// pipeline implementations if there are no specific processing requirements.  Length
	// parameter is relative to data type T, i.e. the number of elements in array T[]
	template <typename T>
	bool						WriteDataToFile(fs::path file, const T *data, size_t length) const;

	// Default implementation to write data into an external file; can be used by output 
	// pipeline implementations if there are no specific processing requirements.  Writes
	// the specified number of bytes (i.e. sizeof(type) * count(type) data at the given 
	// buffer pointer to file
	bool						WriteBufferDataToFile(fs::path file, const void *data, size_t buffer_size) const;
};

// Default implementation to write data into an external file; can be used by output 
// pipeline implementations if there are no specific processing requirements.  Length
// parameter is relative to data type T, i.e. the number of elements in array T[]
template <typename T>
bool TransformPipelineOutput::WriteDataToFile(fs::path file, const T *data, size_t length) const
{
	return WriteBufferDataToFile(file, (const void *)data, sizeof(T) * length);
}