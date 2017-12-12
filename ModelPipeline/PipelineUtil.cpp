#include "PipelineUtil.h"
#include <fstream>
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

