#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "SDFDecalRenderingCommonData.hlsl.h"


// Calculate pixel depth value based upon view-space pixel position and the far plane
float CalculateDepth(float4 viewpos, float farclip)
{
	return -viewpos.z / farclip;
}



// VS_SDFDecal_Deferred: Vertex transformations for deferred screen-space SDF decal rendering
SDFDecalRenderingDeferredVertexShaderOutput VS_SDFDecal_Deferred(Vertex_Inst_Standard input)
{
	SDFDecalRenderingDeferredVertexShaderOutput output;

	float4x4 WorldView = mul(input.Transform, ViewMatrix);
	float4 viewpos = mul(float4(input.position, 1.0f), WorldView);

	// Return both view- and screen-space position data
	output.positionVS = viewpos; 
	output.position = mul(viewpos, ProjMatrix);
	output.depth = CalculateDepth(viewpos, FarClipDistance);

	// Texture coords are modified by the UV shift & scale calculated by the text renderer
	// UV = uv_shift + (UV * uv_scale)
	output.texCoord = input.Highlight.xy + (input.tex * input.Highlight.zw);

	return output;
}

