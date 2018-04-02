#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"

// PS_Basic_Texture: Performs basic linear sampling from the supplied diffuse texture
float4 PS_Basic_Texture(BasicTextureSamplingVertexShaderOutput IN) : SV_TARGET
{
	return DiffuseTexture.Sample(LinearRepeatSampler, IN.texCoord);
}