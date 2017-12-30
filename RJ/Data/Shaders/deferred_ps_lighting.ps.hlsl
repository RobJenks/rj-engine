#include "Common/CommonShaderPipelineStructures.hlsl"
#include "Common/CommonShaderBufferDefinitions.hlsl"
#include "../../../Definitions/MaterialData.hlsl.h"


// GBuffer texture target bindings
Texture2D DiffuseTextureVS : register(t0);
Texture2D SpecularTextureVS : register(t1);
Texture2D NormalTextureVS : register(t2);
Texture2D DepthTextureVS : register(t3);


// Pixel shader that generates the G-Buffer
[earlydepthstencil]
xxx PS_Deferred_Lighting(xxx IN)
{
	return xxx;
}