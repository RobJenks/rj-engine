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
	static const float DEPTH_EXP_FACTOR = 6.0f;

	if (is_depth_texture)
	{
		return pow(DebugSourceTextureVS.Sample(LinearRepeatSampler, IN.texCoord).rrrr, DEPTH_EXP_FACTOR);
	}
	else
	{
		return DebugSourceTextureVS.Sample(LinearRepeatSampler, IN.texCoord);
	}
}