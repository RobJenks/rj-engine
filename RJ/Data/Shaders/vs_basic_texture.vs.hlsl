#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "BasicTextureRenderingCommonData.hlsl.h"


// VS_Basic_Texture: Basic vertex texture which processes only position and texture data to clip space
BasicTextureSamplingVertexShaderOutput VS_Basic_Texture(Vertex_Inst_Standard input)
{
	BasicTextureSamplingVertexShaderOutput output;

	// Position is only required in clip space
	output.position = mul(float4(input.position, 1.0f), input.Transform);	// Model -> World space
	output.position = mul(output.position, ViewProjection);					// -> View -> Projection space

	// Other required data is simply passed through
	output.texCoord = input.tex;
	output.opacity = input.Highlight.x;		// Param[0] == opacity.  TODO: Rename from 'highlight' to 'params'

	return output;
}