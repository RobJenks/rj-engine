#include "../../CommonShaderPipelineStructures.hlsl.h"


// VS_Quad: Specialised full-screen rendering VS with minimal overhead
ScreenSpaceQuadVertexShaderOutput VS_Quad(in float2 P : POSITION)
{
	ScreenSpaceQuadVertexShaderOutput output;
	output.position = float4(P, 0.0f, 1.0f);
	output.texCoord = float2(0.5f, -0.5f) * P + 0.5f;

	return output;
}