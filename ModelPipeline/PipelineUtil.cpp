#include "PipelineUtil.h"
#include <fstream>
#include <sstream>
#include <ios>
#include <iomanip>
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

XMFLOAT3 PipelineUtil::Float3Multiply(const XMFLOAT3 & v0, const XMFLOAT3 & v1)
{
	return XMFLOAT3(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z);
}

XMFLOAT3 PipelineUtil::Float3ScalarMultiply(const XMFLOAT3 & v, float scalar)
{
	return XMFLOAT3(v.x * scalar, v.y * scalar, v.z * scalar);
}

XMFLOAT3 PipelineUtil::Float3Abs(const XMFLOAT3 & v)
{
	return XMFLOAT3(fabs(v.x), fabs(v.y), fabs(v.z));
}

XMFLOAT3 PipelineUtil::Float3Min(const XMFLOAT3 & v0, const XMFLOAT3 & v1)
{
	return XMFLOAT3(std::min(v0.x, v1.x), std::min(v0.y, v1.y), std::min(v0.z, v1.z));
}

XMFLOAT3 PipelineUtil::Float3Max(const XMFLOAT3 & v0, const XMFLOAT3 & v1)
{
	return XMFLOAT3(std::max(v0.x, v1.x), std::max(v0.y, v1.y), std::max(v0.z, v1.z));
}

XMFLOAT3 PipelineUtil::Float3Replicate(float v)
{
	return XMFLOAT3(v, v, v);
}

bool PipelineUtil::Float3Equal(const XMFLOAT3 & v0, const XMFLOAT3 & v1, float epsilon)
{
	XMFLOAT3 abs_diff = Float3Abs(Float3Subtract(v0, v1));
	return (abs_diff.x < epsilon && abs_diff.y < epsilon && abs_diff.z < epsilon);
}

bool PipelineUtil::Float3AnyEqual(const XMFLOAT3 & v, float value, float epsilon)
{
	XMFLOAT3 abs_diff = XMFLOAT3(std::fabsf(v.x - value), std::fabsf(v.y - value), std::fabsf(v.z - value));
	return (abs_diff.x < epsilon || abs_diff.y < epsilon || abs_diff.z < epsilon);
}

bool PipelineUtil::Float3AllEqual(const XMFLOAT3 & v, float value, float epsilon)
{
	return PipelineUtil::Float3Equal(v, XMFLOAT3(value, value, value), epsilon);
}

ByteString PipelineUtil::ReadBinaryFile(fs::path file)
{
	if (!fs::exists(file)) return ByteString();
	std::ifstream in(fs::absolute(file).string(), std::ios::binary);
	if (in.fail()) return ByteString();

	auto const start_pos = in.tellg();
	in.ignore((std::numeric_limits<std::streamsize>::max)());

	auto const char_count = in.gcount();
	in.seekg(start_pos);

	ByteString b(char_count);
	in.read(&b[0], b.size());
	return b;
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

fs::path PipelineUtil::NewTemporaryFile(const std::string & extension)
{
	fs::path tmp = fs::temp_directory_path();
	
	for (unsigned int i = 0; i < 1000U; ++i)
	{
		std::string tmp_file_name = (fs::absolute(tmp).string() + "/rj_modelpipeline_tmp_" + std::to_string(i) + (extension.empty() ? "" : (std::string(".") + extension)));
		fs::path tmp_file = fs::path(tmp_file_name);
		if (!fs::exists(tmp_file))
		{
			return tmp_file;
		}
	}

	return fs::path("<invalid>");
}

fs::path PipelineUtil::NewTemporaryFileWithExistingFile(fs::path existing_file, const std::string & extension)
{
	fs::path filename = existing_file.filename();
	fs::path dir = existing_file.parent_path();

	for (unsigned int i = 0; i < 1000U; ++i)
	{
		std::string new_file_name = (fs::absolute(dir).string() + "/" + filename.string() + "." + std::to_string(i) + (extension.empty() ? "" : (std::string(".") + extension)));
		fs::path new_file = fs::path(new_file_name);
		if (!fs::exists(new_file))
		{
			return new_file;
		}
	}

	return fs::path("<invalid>");
}

fs::path PipelineUtil::SaveToNewTemporaryFile(const std::string & data, const std::string & extension)
{
	fs::path file = NewTemporaryFile(extension);
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

// Splits a string based upon the supplied delimeter, optionally skipping empty items
void PipelineUtil::SplitString(const std::string & input, char delimiter, bool skip_empty, std::vector<std::string> & outElements)
{
	// Return if there is no data to process
	if (input.empty()) return;

	// Use a string stream to process each element in turn
	std::string item;
	std::stringstream ss(input);
	while (std::getline(ss, item, delimiter)) {
		if (!skip_empty || !item.empty())
			outElements.push_back(item);
	}
}


// Splits a string around spaces, observing the presence of quote marks
void PipelineUtil::SplitStringQuoted(const std::string & input, std::vector<std::string> & outElements)
{
	std::string item;
	std::istringstream ss(input);

	while (ss >> std::quoted(item, '\"', (char)0))		// Escape=0 seems to prevent this choking on \\ in path strings
	{
		outElements.push_back(item);
	}
}

std::string PipelineUtil::TrimString(const std::string & str)
{
	auto start = str.find_first_not_of(' ');
	auto end = str.find_last_not_of(' ');
	return str.substr(start, end - start + 1U);
}