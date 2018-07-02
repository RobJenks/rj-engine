#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "DeferredRenderingBuffers.hlsl"
#include "hlsl_common.hlsl"
#include "motion_blur_calculations.hlsl"
#include "noise_calculations.hlsl"

// Debug define to show significance/insignificance
#define DEBUG_RENDER_VELOCITY_SIGNIFICANCE;
static const float4 DEBUG_RENDER_INSIGNIFICANT = float4(1, 0, 0, 1);
static const float4 DEBUG_RENDER_SIGNIFICANT = float4(0, 1, 0, 1);


// Constants
static const float EPS001 = 0.01f;

// TODO: Move into motion blur CB
static const unsigned int C_samples = 15U;
static const unsigned int C_max_sample_tap_distance = 6U;
static const float HALF_VELOCITY_CUTOFF = 0.25f;
static const float VARIANCE_THRESHOLD = 1.5f;
static const float WEIGHT_CORRECTION_FACTOR = 60.0f;



// Motion blur gather phase: entry point
float4 PS_MotionBlur_Gather(VertexShaderStandardOutput IN) : SV_Target0
{
	// TODO: Replace with CB data
	float2 texsize = DetermineTextureDimensions(MotionBlurColourBufferInput);

	// Sample from source (unprocessed colour buffer)
	float2 texcoord = IN.texCoord;
	float4 colour = MotionBlurColourBufferInput.SampleLevel(LinearClampSampler, texcoord, 0);

	// Read and decode the dominant half-velocity for this pixel's velocity neighbourhood
	float2 nh_vel = RevertNormalisationScaleBias(MotionBlurVelocityNeighbourhoodInput.Sample(PointClampSampler, texcoord).xy);
	float nh_vel_mag = length(nh_vel);

	// Weight vectors based upon exposure.  Early-exit if velocity is insignificant
	float weighted_nhv = (nh_vel_mag * C_half_exposure);
	bool nhv_pos = (weighted_nhv > EPS001);
	weighted_nhv = clamp(weighted_nhv, 0.1f, C_k);

	if (weighted_nhv < HALF_VELOCITY_CUTOFF)
	{
#		ifdef DEBUG_RENDER_VELOCITY_SIGNIFICANCE
			return DEBUG_RENDER_INSIGNIFICANT;
#		endif

		return colour;
	}

	// Weight half-velocity
	if (nhv_pos)
	{
		nh_vel *= (weighted_nhv / nh_vel_mag);
		nh_vel_mag = length(nh_vel);
	}

	// Sample & decode from the primary full-resolution velocity buffer at the same pixel location
	float2 vel = RevertNormalisationScaleBias(MotionBlurVelocityBufferInput.Sample(PointClampSampler, texcoord).xy);
	float vel_mag = length(vel);

	// Weight and clamp primary velocity data
	float weighted_v = (vel_mag * C_half_exposure);
	bool v_pos = (weighted_v > EPS001);
	weighted_v = clamp(weighted_v, 0.1f, C_k);

	if (v_pos)
	{
		vel *= (weighted_v / vel_mag);
		vel_mag = length(vel);
	}

	// Fall back to neighbourhood maximal velocity if point-velocity is insignificant
	float2 corrected_vel = (vel_mag < VARIANCE_THRESHOLD ? normalize(nh_vel) : normalize(vel));

	// Inverse depth at pixel location
	float depth = InvPointDepth(texcoord);

	// Random noise seed for tap sampling (bias to [-0.5 +0.5])
	float rnd = RandomNoise(texcoord) - 0.5f;

	// Update sample tap distance based on source texture
	float max_tap_dist = (C_max_sample_tap_distance / texsize.x);
	float2 half_texel = (HALF_VECTOR2 / texsize.x);

	// Weighting value with experimentally-derived constant (from pp)
	float weight = (C_samples / WEIGHT_CORRECTION_FACTOR / weighted_v);

	// Initialise to state of first texel, and determine current index so we can skip it during reconstruction sampling
	float3 aggregate = (colour.xyz * weight);
	unsigned int index = (C_samples - 1) / 2;

	// Aggregate for each reconstruction sample tap
	for (unsigned int i = 0; i < C_samples; ++i)
	{
		// Avoid double-sampling the current fragment
		if (i == index) continue;

		// Determine the distance between the current pixel and the next sample tap
		float lerp_amt = (float(i) + rnd + 1.0f) / (C_samples + 1.0f);
		float tapdist = lerp(-max_tap_dist, +max_tap_dist, lerp_amt);

		// Alternate between corrected and neighbourhood maximal velocity for better visual result
		float2 selected_vel = (((i & 1) == 1) ? corrected_vel : nh_vel);

		// Determine the texel location of this sample tap and sample from primary half-vel buffer
		float2 tapcoord = float2(texcoord + float2(selected_vel * tapdist * half_texel));
		float2 tap = RevertNormalisationScaleBias(MotionBlurVelocityBufferInput.SampleLevel(PointClampSampler, tapcoord, 0).xy);
		float tap_mag = length(tap);

		// Weight and clamp current tap velocity
		float weighted_tap = (tap_mag * C_half_exposure);
		float tap_pos = (weighted_tap > EPS001);
		weighted_tap = clamp(weighted_tap, 0.1f, C_k);
		
		if (tap_pos)
		{
			tap *= (weighted_tap / tap_mag);
			tap_mag = length(tap);
		}

		// Sample inverse depth at the tap point
		float tap_depth = InvPointDepth(tapcoord);

		// Compute alpha contribution value as (foreground contrib + background contrib + blurred fg+bg contrib)
		float tap_alpha = SoftDepthComparison(depth, tap_depth) * Cone(tapdist, weighted_tap)
						+ SoftDepthComparison(tap_depth, depth) * Cone(tapdist, weighted_v)
						+ Cylinder(tapdist, weighted_tap) * Cylinder(tapdist, weighted_v) * 2.0f;

		// Accumulate calculated tap value
		weight += tap_alpha;
		aggregate += (tap_alpha * MotionBlurColourBufferInput.SampleLevel(LinearClampSampler, tapcoord, 0).xyz);
	}

	// Debug signficance rendering
#	ifdef DEBUG_RENDER_VELOCITY_SIGNIFICANCE
		return DEBUG_RENDER_SIGNIFICANT + (float4(aggregate/weight, 0.0f) * 0.0001f);
#	endif

	// Return the final aggregate colour value
	return float4(aggregate / weight, 1.0f);
}