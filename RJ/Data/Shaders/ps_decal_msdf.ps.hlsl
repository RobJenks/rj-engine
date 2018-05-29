#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "SDFDecalRenderingCommonData.hlsl.h"
#include "hlsl_math.hlsl"

// PS_SDFDecal: Performs basic linear sampling from the supplied diffuse texture
float4 PS_MSDFDecal(SDFDecalRenderingVertexShaderOutput IN) : SV_TARGET
{
	// Sample distance directly from the diffuse texture alpha channel
	float3 tex = DiffuseTexture.Sample(LinearRepeatSampler, IN.texCoord).rgb;

	// Determine signed distance as median of three component channels
	float distance = median(tex.r, tex.g, tex.b) - 0.5;

	// Take the absolute value of the partial derivatives of this signed distance
	float weight = clamp(distance / fwidth(distance) + 0.5, 0.0, 1.0);

	// Linearly interpolate between decal colour and (transparent) background based on this weighting
	float4 _background = float4(baseColour.rgb, 0.0f);
	return lerp(_background, baseColour, weight);
}