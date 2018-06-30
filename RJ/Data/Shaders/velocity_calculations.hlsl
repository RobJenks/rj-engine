#ifndef __VelocityCalculationsH__
#define __VelocityCalculationsH__

#include "hlsl_common.hlsl"
#include "DeferredRenderingBuffers.hlsl"


// Calculate the normalised screen-space velocity of the given pixel, based on supplied data
// for the current and prior frame.  Velocity data is normalised down within the 
// range [0 1] for screen-space velocity [0 MAX_SCREEN_SPACE_PIXEL_VELOCITY]
float2 CalculateScreenSpacePixelVelocity(float4 prior_pos, float4 current_pos)
{
	float2 vraw = ((current_pos.xy / current_pos.w) - (prior_pos.xy / prior_pos.w)) * C_half_frame_exposure;
	float vmagnitude = length(vraw);

	float weight = max(0.5f, min(vmagnitude, C_k));
	weight /= (vmagnitude + 0.01f);

	vraw *= weight;
	return ApplyNormalisationScaleBias(vraw);
}



#endif