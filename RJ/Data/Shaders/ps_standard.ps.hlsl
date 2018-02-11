#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"

// PS_Standard: Standard pixel shader
float4 PS_Standard(VertexShaderStandardOutput IN) : SV_TARGET
{
	return DiffuseTexture.Sample(LinearRepeatSampler, IN.texCoord);
}