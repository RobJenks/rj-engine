#pragma once

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
};