#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"


// VS_Standard: Standard vertex shader
VertexShaderStandardOutput VS_Standard(Vertex_Inst_Standard input)
{
	VertexShaderStandardOutput output;

	float4x4 WorldView = mul(input.Transform, View);
	float4 pos_view = mul(float4(input.position, 1.0f), WorldView);

	output.positionVS = pos_view.xyz;				// View-space position
	output.position = mul(pos_view, Projection);	// Clip-space position

	output.tangentVS = mul(input.tangent, (float3x3)WorldView);
	output.binormalVS = mul(input.binormal, (float3x3)WorldView);
	output.normalVS = mul(input.normal, (float3x3)WorldView);

	output.texCoord = input.tex;

	return output;
}