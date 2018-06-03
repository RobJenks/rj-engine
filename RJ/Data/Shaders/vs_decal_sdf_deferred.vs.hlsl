#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "SDFDecalRenderingCommonData.hlsl.h"


// VS_SDFDecal_Deferred: Vertex transformations for deferred screen-space SDF decal rendering
SDFDecalRenderingDeferredVertexShaderOutput VS_SDFDecal_Deferred(Vertex_Inst_Standard input)
{
	SDFDecalRenderingDeferredVertexShaderOutput output;

	// Position is only required in clip space
	output.position = mul(float4(input.position, 1.0f), input.Transform);	// Model -> World space
	output.position = mul(output.position, ViewProjection);					// -> View -> Projection space

	// Texture coords are modified by the UV shift & scale calculated by the text renderer
	// UV = uv_shift + (UV * uv_scale)
	output.texCoord = input.Highlight.xy + (input.tex * input.Highlight.zw);

	return output;
}