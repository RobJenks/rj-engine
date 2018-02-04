#pragma once

#include <vector>
#include <DirectXMath.h>
#include <filesystem>
#include "../Definitions/ByteString.h"
using namespace DirectX;
namespace fs = std::experimental::filesystem;

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
};