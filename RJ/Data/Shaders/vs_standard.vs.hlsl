#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"


// VS_Standard: Standard vertex shader
VertexShaderStandardOutput VS_Standard(Vertex_Inst_Standard input)
{
	VertexShaderStandardOutput output;

	// Position transformation into view- and clip-space
	float4 input_pos = float4(input.position, 1.0f);
	float4x4 WorldView = mul(input.Transform, View);
	float4 pos_view = mul(input_pos, WorldView);

	output.positionVS = pos_view.xyz;				// View-space position
	output.position = mul(pos_view, Projection);	// Clip-space position

	// Also calculate vertex position in the prior frame, for screen-space velocity calculations
	float4x4 lastframe_transform = mul(input.LastTransform, PriorFrameViewProjection);
	output.lastframeposition = mul(input_pos, lastframe_transform);
	output.thisframeposition = output.position;

	// Transform auxilliary data
	output.tangentVS = mul(input.tangent, (float3x3)WorldView);
	output.binormalVS = mul(input.binormal, (float3x3)WorldView);
	output.normalVS = mul(input.normal, (float3x3)WorldView);

	output.texCoord = input.tex;

	return output;
}