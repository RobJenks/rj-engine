#ifndef __CommonShaderPipelineStructuresHlsl__
#define __CommonShaderPipelineStructuresHlsl__

#include "../Definitions/CppHLSLLocalisation.hlsl.h"


// Output of the standard vertex shader
struct VertexShaderStandardOutput
{
	float3 positionVS			RJ_SEMANTIC(TEXCOORD0);		// View space position
	float2 texCoord				RJ_SEMANTIC(TEXCOORD1);		// Texture coordinate
	float3 tangentVS			RJ_SEMANTIC(TANGENT);		// View space tangent
	float3 binormalVS			RJ_SEMANTIC(BINORMAL);		// View space binormal
	float3 normalVS 			RJ_SEMANTIC(NORMAL);		// View space normal
	float4 position				RJ_SEMANTIC(SV_POSITION);	// Clip space position
};

// Output of basic texture-sampling VS stage; returns only pos/texcoord basic data
struct BasicTextureSamplingVertexShaderOutput
{
	float4 position				RJ_SEMANTIC(SV_POSITION);	// Clip-space position
	float2 texCoord				RJ_SEMANTIC(TEXCOORD0);		// Texture coordinate
	float opacity				RJ_SEMANTIC(OPACITY);		// Opacity in the range [0.0 1.0]
};


// Output of the deferred geometry PS; mapping into the textures comprising the GBuffer
struct DeferredPixelShaderGeometryOutput
{
	float4 LightAccumulation    RJ_SEMANTIC(SV_Target0);   // Ambient + emissive (R8G8B8_ ) UNUSED (A8_UNORM)
	float4 Diffuse              RJ_SEMANTIC(SV_Target1);   // Diffuse Albedo (R8G8B8_UNORM) UNUSED (A8_UNORM)
	float4 Specular             RJ_SEMANTIC(SV_Target2);   // Specular Color (R8G8B8_UNORM) Specular Power(A8_UNORM)
	float4 NormalVS             RJ_SEMANTIC(SV_Target3);   // View space normal (R32G32B32_FLOAT) UNUSED(A32_FLOAT)
};




#endif