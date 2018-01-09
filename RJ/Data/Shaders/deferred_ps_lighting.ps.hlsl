#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/MaterialData.hlsl.h"


// GBuffer texture target bindings
Texture2D DiffuseTextureVS : register(t0);
Texture2D SpecularTextureVS : register(t1);
Texture2D NormalTextureVS : register(t2);
Texture2D DepthTextureVS : register(t3);


// Pixel shader that generates the G-Buffer
[earlydepthstencil]
float4 PS_Deferred_Lighting(VertexShaderStandardOutput IN) : SV_TARGET
{
	return float4(0,0,0,0);
}