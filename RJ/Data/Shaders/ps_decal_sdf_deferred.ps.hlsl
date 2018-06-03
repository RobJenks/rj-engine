#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "SDFDecalRenderingCommonData.hlsl.h"

// PS_SDFDecal_Direct: Renders a decal directly based on a source signed-distance field representation
float4 PS_SDFDecal_Deferred(SDFDecalRenderingVertexShaderOutput IN) : SV_TARGET
{
	// Sample distance directly from the diffuse texture alpha channel
	return DiffuseTexture.Sample(LinearRepeatSampler, IN.texCoord) + float4(0.0f, 0.0f, 0.0f, smoothingFactor * 0.01f);
}