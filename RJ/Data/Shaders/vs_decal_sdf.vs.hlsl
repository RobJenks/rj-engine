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

	// Texture coords are modified by the UV shift & scale calculated by the text renderer
	// UV = uv_shift + (UV * uv_scale)
	output.texCoord = input.Highlight.xy + (input.tex * input.Highlight.zw);

	// Other required data is simply passed through from vertex and instance parameters
	output.uv_shift = input.Highlight.xy;
	output.uv_scale = input.Highlight.zw;

	return output;
}