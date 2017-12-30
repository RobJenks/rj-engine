#ifndef __CppHLSLLocalisationH__
#define __CppHLSLLocalisationH__


// Enable localisation to both C++ & HLSL
#ifdef __cplusplus

#	include <DirectXMath.h>
#	include "../RJ/ALIGN16.h"
#	define cbuffer struct
#	define REGISTER(x) 
#	define ALIGNED16(T) : public ALIGN16<T>

	typedef DirectX::XMFLOAT2 float2;
	typedef DirectX::XMFLOAT3 float3;
	typedef DirectX::XMFLOAT4 float4;
	typedef DirectX::XMFLOAT3X3 float3x3;
	typedef DirectX::XMFLOAT4X4 float4x4;
	typedef uint32_t _uint32;

#else

#	define REGISTER(x) : register(x)
#	define ALIGNED16(T)

	typedef uint _uint32;

#endif




#endif