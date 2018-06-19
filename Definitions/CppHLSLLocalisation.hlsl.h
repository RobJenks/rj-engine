#ifndef __CppHLSLLocalisationH__
#define __CppHLSLLocalisationH__


// Enable localisation to both C++ & HLSL
#if defined(__cplusplus) && !defined(RJ_COMPILING_HLSL)

#	include <windows.h>
#	include <DirectXMath.h>
#	include "../RJ/ALIGN16.h"
#	include "../RJ/DefaultingFloat4.h"

#	define CBUFFER struct
#	define TEXTURE2D struct
#	define REGISTER(x) 
#	define ALIGNED16(T) : public ALIGN16<T>

#	define RJ_ROW_MAJOR_MATRIX DirectX::XMFLOAT4X4
#	define RJ_SEMANTIC(sem) 

	typedef DirectX::XMFLOAT2 float2;
	typedef DirectX::XMFLOAT3 float3;
	typedef DirectX::XMFLOAT4 float4;
	typedef DirectX::XMFLOAT3X3 float3x3;
	typedef DirectX::XMFLOAT4X4 float4x4;
	typedef DirectX::XMUINT2 uint2;
	typedef DirectX::XMUINT3 uint3;
	typedef DirectX::XMUINT4 uint4;
	typedef uint32_t _uint32;
	typedef BOOL _bool;							// We must use BOOL/int for bool values when crossing CPU/GPU border since sizeof(bool) in HLSL == 1, sizeof(bool) in C++ == 4
	using RM_SortKey = _uint32;
	typedef DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT> Float4DefaultZeroW;

#else

#	define CBUFFER cbuffer
#	define TEXTURE2D Texture2D
#	define REGISTER(x) : register(x)
#	define ALIGNED16(T)

#	define RJ_ROW_MAJOR_MATRIX row_major float4x4
#	define RJ_SEMANTIC(sem) : sem

	typedef uint _uint32;
	typedef int _bool;							// We must use BOOL/int for bool values when crossing CPU/GPU border since sizeof(bool) in HLSL == 1, sizeof(bool) in C++ == 4
	typedef float4 Float4DefaultZeroW;
	typedef _uint32 RM_SortKey;					// HLSL uint == uint32_t


#endif

// Convert buffer type names to their string representation
#define BUFFER_NAME(buffer) #buffer

// Values common to both CPP and HLSL
static const _bool _true = 1;
static const _bool _false = 0;





#endif