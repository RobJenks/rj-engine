#ifndef __VelocityCalculationsH__
#define __VelocityCalculationsH__

#include "hlsl_common.hlsl"


// Maximum permitted screen-space pixel velocity
static const float MAX_SCREEN_SPACE_PIXEL_VELOCITY = 20.0f;
static const float2 MAX_SCREEN_SPACE_PIXEL_VELOCITY_VEC = float2(MAX_SCREEN_SPACE_PIXEL_VELOCITY, MAX_SCREEN_SPACE_PIXEL_VELOCITY);

// ****** TEMPORARY ******
static const float exposure = 1.0f;
static const float frame_delta = (1.0f / 60.0f);
static const float frame_delta_half_exposure = (0.5f * (exposure / frame_delta));
static const float K = 2.0f;


// Calculate the normalised screen-space velocity of the given pixel, based on supplied data
// for the current and prior frame.  Velocity data is normalised down within the 
// range [0 1] for screen-space velocity [0 MAX_SCREEN_SPACE_PIXEL_VELOCITY]
float2 CalculateScreenSpacePixelVelocity(float4 prior_pos, float4 current_pos)
{
	float2 vraw = ((current_pos.xy / current_pos.w) - (prior_pos.xy / prior_pos.w)) * frame_delta_half_exposure;
	float vmagnitude = length(vraw);

	float weight = max(0.5f, min(vmagnitude, K));
	weight /= (vmagnitude + 0.01f);

	vraw *= weight;
	return ApplyNormalisationScaleBias(vraw);
}



#endif