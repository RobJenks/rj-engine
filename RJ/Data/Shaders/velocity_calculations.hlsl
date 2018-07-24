#ifndef __VelocityCalculationsH__
#define __VelocityCalculationsH__

#include "hlsl_common.hlsl"
#include "DeferredRenderingBuffers.hlsl"

#define REMOVE_VELOCITY_FRAMERATE_DEPENDENCE		0	// Removes framerate dependency for velocity calculation if set
#define IGNORE_INSIGNIFICANT_VELOCITY_VECTORS		0	// Ignores velocity vectors below a given significance threshold, if set

static const float FRAME_RATE_RECIP_60FPS = (1.0f / 60.0f);

static const float2 VEL_SIGNIFICANCE_THRESHOLD = float2(0.0001f, 0.0001f);
static const float2 VEL_NULL = float2(0.5f, 0.5f);		// Equivalent of ApplyNormalisationScaleBias(0.0f, 0.0f)

// Calculate the normalised screen-space velocity of the given pixel, based on supplied data
// for the current and prior frame.  Velocity data is normalised down within the 
// range [0 1] for screen-space velocity [0 MAX_SCREEN_SPACE_PIXEL_VELOCITY]
float2 CalculateScreenSpacePixelVelocity(float4 prior_pos, float4 current_pos)
{
	float2 vraw = ((current_pos.xy / current_pos.w) - (prior_pos.xy / prior_pos.w));

	// Ignore velocity vectors below the significance threshold, if parameter is set
#if IGNORE_INSIGNIFICANT_VELOCITY_VECTORS
	if (vraw.x < VEL_SIGNIFICANCE_THRESHOLD.x && vraw.y < VEL_SIGNIFICANCE_THRESHOLD.y) return VEL_NULL;
#endif

	// Remove dependency on framerate, if set
#if REMOVE_VELOCITY_FRAMERATE_DEPENDENCE
		vraw *= ((2.0f * C_half_exposure) / FRAME_RATE_RECIP_60FPS) * 0.5f;	// Equiv of C_half_frame_exposure calc but with 1/60 instead of 1/TimeFactor
#else
		vraw *= C_half_frame_exposure;
#endif

	float vmagnitude = length(vraw);

	float weight = max(0.5f, min(vmagnitude, C_k));
	weight /= (vmagnitude + 0.01f);

	vraw *= weight;
	return ApplyNormalisationScaleBias(vraw);
}



#endif