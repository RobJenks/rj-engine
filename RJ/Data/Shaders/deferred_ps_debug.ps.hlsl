#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "DeferredRenderingBuffers.hlsl"


// GBuffer texture target bindings
Texture2D DebugSourceTextureVS : register(t0);


// Pixel shader that generates the G-Buffer
[earlydepthstencil]
float4 PS_Deferred_Debug(VertexShaderStandardOutput IN) : SV_Target0
{
	if (is_depth_texture)
	{
		return DebugSourceTextureVS.Sample(LinearRepeatSampler, IN.texCoord).rrrr;
	}
	else
	{
		return DebugSourceTextureVS.Sample(LinearRepeatSampler, IN.texCoord);
	}
}