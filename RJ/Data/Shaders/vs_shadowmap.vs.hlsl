#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "shadowmap_resources.hlsl"


// VS_Standard: Standard vertex shader
ShadowMappingVertexShaderOutput VS_ShadowMap(Vertex_Inst_Standard input)
{
	ShadowMappingVertexShaderOutput output;

	float4 input_pos = float4(input.position, 1.0f);
	float4x4 WorldViewProj_Transform = mul(input.Transform, LightViewProjection);

	output.position = mul(input_pos, WorldViewProj_Transform);					// Clip-space position

	return output;
}