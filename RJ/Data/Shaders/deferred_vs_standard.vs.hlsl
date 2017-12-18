#include "Common/CommonShaderPipelineStructures.hlsl"
#include "Common/CommonShaderBufferDefinitions.hlsl"
#include "../../../Definitions/VertexDefinitions.hlsl.h"


VertexShaderStandardOutput VS_Deferred(Vertex_Inst_Standard input)
{
	VertexShaderStandardOutput output;

	float4x4 WorldView = mul(input.Transform, View);
	float4 pos_view = mul(WorldView, float4(input.position, 1.0f));

	output.positionVS = pos_view.xyz;				// View-space position
	output.position = mul(Projection, pos_view);	// Clip-space position

	output.tangentVS = mul((float3x3)WorldView, input.tangent);
	output.binormalVS = mul((float3x3)WorldView, input.binormal);
	output.normalVS = mul((float3x3)WorldView, input.normal);

	output.texCoord = input.tex;

	return output;
}