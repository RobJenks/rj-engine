#ifndef __ShadowMapCalculationsHLSL__
#define __ShadowMapCalculationsHLSL__

#include "shadowmap_resources.hlsl"


// Compute the shadowing factor based on the currently-bound shadow map and the given UV-biased
// projected coordinates.  
float ComputeShadowFactor(float4 shadowmap_uv_biased)
{
	bool shadowed = (ShadowMapTexture.Sample(PointClampSampler, shadowmap_uv_biased.xy).r < shadowmap_uv_biased.z);

	return (shadowed ? 0.5f : 1.0f);
}




#endif