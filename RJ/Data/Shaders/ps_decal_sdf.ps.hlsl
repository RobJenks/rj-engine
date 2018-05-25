#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "SDFDecalRenderingCommonData.hlsl.h"

// PS_SDFDecal: Performs basic linear sampling from the supplied diffuse texture
float4 PS_SDFDecal(SDFDecalRenderingVertexShaderOutput IN) : SV_TARGET
{
	// Sample directly from the diffuse texture
	float4 diffuse = DiffuseTexture.Sample(LinearRepeatSampler, IN.texCoord);

	// Return the diffuse colour
	return diffuse;
}