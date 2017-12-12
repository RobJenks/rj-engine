#include <fstream>
#include <ios>
#include "FileUtils.h"


ByteString FileUtils::ReadBinaryFile(fs::path file)
{
	if (!fs::exists(file)) return ByteString();

	std::ifstream input(fs::absolute(file).string(), std::ios::binary | std::ios::ate);	// ios::ate == seek to end on opening
	std::ifstream::pos_type file_size = input.tellg();

	ByteString result;
	result.reserve(file_size);

	input.seekg(0, std::ios::beg);				// Seek to beginning
	input.read(result.data(), file_size);

	return result;
}


