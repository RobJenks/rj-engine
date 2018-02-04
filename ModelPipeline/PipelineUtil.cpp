#include "PipelineUtil.h"
#include <fstream>
#include <sstream>
#include <ios>
#include <filesystem>
#include <DirectXMath.h>
#include "../Definitions/ByteString.h"
using namespace DirectX;



XMFLOAT3 PipelineUtil::Float3Add(const XMFLOAT3 & v0, const XMFLOAT3 & v1)
{
	return XMFLOAT3(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

XMFLOAT3 PipelineUtil::Float3Subtract(const XMFLOAT3 & v0, const XMFLOAT3 & v1)
{
	return XMFLOAT3(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

XMFLOAT3 PipelineUtil::Float3ScalarMultiply(const XMFLOAT3 & v, float scalar)
{
	return XMFLOAT3(v.x * scalar, v.y * scalar, v.z * scalar);
}

ByteString PipelineUtil::ReadBinaryFile(fs::path file)
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


std::string PipelineUtil::ReadFileToString(fs::path file)
{
	if (!fs::exists(file)) return "";

	std::ifstream input(fs::absolute(file));
	std::stringstream ss;

	while (input >> ss.rdbuf()) { /* Simply stream all data into the string */ }
	input.close();

	return ss.str();
}


void PipelineUtil::WriteDataTofile(fs::path file, const std::string & data)
{
	std::ofstream out(fs::absolute(file));
	out << data;
	out.close();
}

fs::path PipelineUtil::NewTemporaryFile(void)
{
	fs::path tmp = fs::temp_directory_path();
	
	for (unsigned int i = 0; i < 1000U; ++i)
	{
		std::string tmp_file_name = (fs::absolute(tmp).string() + "/rj_modelpipeline_tmp_" + std::to_string(i));
		fs::path tmp_file = fs::path(tmp_file_name);
		if (!fs::exists(tmp_file))
		{
			return tmp_file;
		}
	}

	return fs::path("<invalid>");
}

fs::path PipelineUtil::NewTemporaryFileWithExistingFile(fs::path existing_file)
{
	fs::path filename = existing_file.filename();
	fs::path dir = existing_file.parent_path();

	for (unsigned int i = 0; i < 1000U; ++i)
	{
		std::string new_file_name = (fs::absolute(dir).string() + "/" + filename.string() + "." + std::to_string(i));
		fs::path new_file = fs::path(new_file_name);
		if (!fs::exists(new_file))
		{
			return new_file;
		}
	}

	return fs::path("<invalid>");
}

fs::path PipelineUtil::SaveToNewTemporaryFile(const std::string & data)
{
	fs::path file = NewTemporaryFile();
	WriteDataTofile(file, data);
	return file;
}

void PipelineUtil::DeleteTemporaryFile(fs::path file)
{
	if (fs::exists(file))
	{
		fs::remove(file);
	}
}

