#ifndef __material_definitionH__
#define __material_definitionH__


// Enable common usage across C++ and HLSL by making preprocessor adjustments
#ifdef __cplusplus
	#include <DirectXMath.h>
	using float2 = DirectX::XMFLOAT2;
	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMFLOAT4;
#endif


// Custom type for material ID
typedef unsigned int				MATERIAL_ID;


// Standard material definition
struct MaterialData
{
	MATERIAL_ID						ID;
	float3							_padding_matheader;

	float4							Ambient;
	float4							Diffuse;
	float4							Specular; // w = SpecPower
	float4							Reflect;

	// Size = 68, Size+pad = 80, 80 % 16 = 0
};


#endif


