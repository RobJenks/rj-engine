#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "SDFDecalRenderingCommonData.hlsl.h"


// VS_SDFDecal: Vertex transformations for direct SDF decal rendering
SDFDecalRenderingVertexShaderOutput VS_SDFDecal_Direct(Vertex_Inst_Standard input)
{
	SDFDecalRenderingVertexShaderOutput output;

	// Position is only required in clip space
	output.position = mul(float4(input.position, 1.0f), input.Transform);	// Model -> World space
	output.position = mul(output.position, ViewMatrix);						// -> View 
	output.position = mul(output.position, ProjMatrix);						// -> Projection space

	// Texture coords are modified by the UV shift & scale calculated by the text renderer
	// UV = uv_shift + (UV * uv_scale)
	output.texCoord = input.Highlight.xy + (input.tex * input.Highlight.zw);

	return output;
}