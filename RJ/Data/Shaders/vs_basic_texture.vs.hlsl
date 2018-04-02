#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "UIRenderingCommonData.hlsl.h"


// VS_Basic_Texture: Basic vertex texture which processes only position and texture data to clip space
BasicTextureSamplingVertexShaderOutput VS_Basic_Texture(Vertex_Inst_Standard input)
{
	BasicTextureSamplingVertexShaderOutput output;

	// Position is only required in clip space
	output.position = mul(float4(input.position, 1.0f), input.Transform);	// Model -> World space
	output.position = mul(output.position, ViewProjection);					// -> View -> Projection space

	// Texture coordinates are simply passed through
	output.texCoord = input.tex;

	return output;
}