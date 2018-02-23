#pragma once

#include <vector>
#include <DirectXMath.h>
#include <filesystem>
#include "../Definitions/ByteString.h"
using namespace DirectX;
namespace fs = std::experimental::filesystem;

#define FLOAT2_STR(_float2)		"[" << _float2.x << ", " << _float2.y << "]"
#define FLOAT3_STR(_float3)		"[" << _float3.x << ", " << _float3.y << ", " << _float3.z << "]"


class PipelineUtil
{
public:

	static XMFLOAT3				Float3Add(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static XMFLOAT3				Float3Subtract(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static XMFLOAT3				Float3ScalarMultiply(const XMFLOAT3 & v, float scalar);

	static ByteString			ReadBinaryFile(fs::path file);

	static std::string			ReadFileToString(fs::path file);
	static void					WriteDataTofile(fs::path file, const std::string & data);
	static fs::path				NewTemporaryFile(void);
	static fs::path				NewTemporaryFileWithExistingFile(fs::path existing_file);
	static fs::path				SaveToNewTemporaryFile(const std::string & data);
	static void					DeleteTemporaryFile(fs::path file);

	template <typename T>
	static inline bool			ArrayContainsData(const T *_array, unsigned int array_size);
};


template <typename T>
inline bool PipelineUtil::ArrayContainsData(const T *_array, unsigned int array_size)
{
	const char *bytes = (const char*)_array;
	unsigned int bytelength = (sizeof(T) * array_size);

	for (unsigned int i = 0U; i < bytelength; ++i)
	{
		if (bytes[i] != 0) return true;
	}

	return false;
}