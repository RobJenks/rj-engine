#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "SDFDecalRenderingCommonData.hlsl.h"

// PS_SDFDecal: Performs basic linear sampling from the supplied diffuse texture
float4 PS_SDFDecal(SDFDecalRenderingVertexShaderOutput IN) : SV_TARGET
{
	// Sample distance directly from the diffuse texture alpha channel
	float distance = DiffuseTexture.Sample(LinearRepeatSampler, IN.texCoord).r;

	// Optional outline around decal edges
	const float smoothing = 1.0f / 4.0f;
	float outlineEffect = smoothstep(0.5f - smoothing, 0.5f + smoothing, distance);
	float4 color = lerp(outlineColour, baseColour, outlineEffect);

	// Convert outline factor from [0.0 1.0] to [0.5 0.0], for lowest->highest
	// Conversion = f(x) = ((1.0f - x) * 0.5f)
	float calculatedOutlineFactor = (1.0f - clamp(outlineDistanceFactor, 0.0f, 1.0f)) * 0.5f;

	// Alpha falloff with distance; outline factor determines width of surrounding outline (if any)
	float alpha = smoothstep(calculatedOutlineFactor - smoothing, calculatedOutlineFactor + smoothing, distance);
	return float4(color.rgb, color.a * alpha);
}