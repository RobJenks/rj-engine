#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "SDFDecalRenderingCommonData.hlsl.h"


// VS_SDFDecal: Basic vertex texture which largely passes through instance data
SDFDecalRenderingVertexShaderOutput VS_SDFDecal(Vertex_Inst_Standard input)
{
	SDFDecalRenderingVertexShaderOutput output;

	// Position is only required in clip space
	output.position = mul(float4(input.position, 1.0f), input.Transform);	// Model -> World space
	output.position = mul(output.position, ViewProjection);					// -> View -> Projection space

	// Other required data is simply passed through from vertex and instance parameters
	output.texCoord = input.tex;
	output.uv_shift = input.Highlight.xy;
	output.uv_scale = input.Highlight.zw;

	return output;
}