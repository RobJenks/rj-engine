#pragma once

#include <vector>
#include <DirectXMath.h>
#include <filesystem>
#include "../Definitions/ByteString.h"
using namespace DirectX;
namespace fs = std::filesystem;

#define FLOAT2_STR(_float2)		"[" << _float2.x << ", " << _float2.y << "]"
#define FLOAT3_STR(_float3)		"[" << _float3.x << ", " << _float3.y << ", " << _float3.z << "]"


class PipelineUtil
{
public:

	static XMFLOAT3				Float3Add(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static XMFLOAT3				Float3Subtract(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static XMFLOAT3				Float3Multiply(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static XMFLOAT3				Float3ScalarMultiply(const XMFLOAT3 & v, float scalar);
	static XMFLOAT3				Float3Abs(const XMFLOAT3 & v);
	static XMFLOAT3				Float3Min(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static XMFLOAT3				Float3Max(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static XMFLOAT3				Float3Replicate(float v);
	static bool					Float3Equal(const XMFLOAT3 & v0, const XMFLOAT3 & v1, float epsilon = 0.0f);
	static bool					Float3AnyEqual(const XMFLOAT3 & v, float value, float epsilon = 0.0f);
	static bool					Float3AllEqual(const XMFLOAT3 & v, float value, float epsilon = 0.0f);
	static bool					Float3LessThan(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static bool					Float3LessOrEqual(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static bool					Float3GreaterThan(const XMFLOAT3 & v0, const XMFLOAT3 & v1);
	static bool					Float3GreaterOrEqual(const XMFLOAT3 & v0, const XMFLOAT3 & v1);

	static ByteString			ReadBinaryFile(fs::path file);

	static std::string			ReadFileToString(fs::path file);
	static void					WriteDataTofile(fs::path file, const std::string & data);
	static fs::path				NewTemporaryFile(const std::string & extensions = "");
	static fs::path				NewTemporaryFileWithExistingFile(fs::path existing_file, const std::string & extension = "");
	static fs::path				SaveToNewTemporaryFile(const std::string & data, const std::string & extension = "");
	static void					DeleteTemporaryFile(fs::path file);

	static void					SplitString(const std::string & input, char delimiter, bool skip_empty, std::vector<std::string> & outElements);
	static void					SplitStringQuoted(const std::string & input, std::vector<std::string> & outElements);
	static std::string			TrimString(const std::string & str);

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