#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"

// PS_Basic_Texture: Performs basic linear sampling from the supplied diffuse texture
float4 PS_Basic_Texture(BasicTextureSamplingVertexShaderOutput IN) : SV_TARGET
{
	// Sample directly from the diffuse texture
	float4 diffuse = DiffuseTexture.Sample(LinearRepeatSampler, IN.texCoord);

	// Alpha will be (opacity_instance_param * opacity_texture * diffuse_opacity)
	float alpha = IN.opacity;
	if (Mat.HasOpacityTexture)
	{
		alpha *= OpacityTexture.Sample(LinearRepeatSampler, IN.texCoord).r;
	}

	// Return the diffuse colour with calculated opacity
	diffuse.w *= alpha;
	return diffuse;
}