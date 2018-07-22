// Compile-time debug constants
#define ENABLE_VELOCITY_DILATION			1		// Dilate velocity values based on the immediate 3x3 neighbourhood
#define USE_YCOCG							0		// Use YCoCg rather than RGB colour space
#define PERFORM_COLOUR_SPACE_CLIPPING		1		// Clip colour space values against the colour-space AABB, rather than clamping
#define FAST_APPROX_COLOUR_SPACE_CLIPPING	1		// Perform colour space clipping via fast method which only clips towards AABB centre (no noticeable quality impact)
#define MOTION_BLUR_BLEND					1		// Blend reprojection into motion blur at higher screen-space velocities
#define DEBUG_TEMPORAL_MOTION_BLEND			0		// Will show a debug view of temporal reprojection (green) vs motion (red) components, if set

// Includes
#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "DeferredRenderingBuffers.hlsl"
#include "hlsl_common.hlsl"
#include "temporal_aa_resources.hlsl"
#include "temporal_aa_calculations.hlsl"


// Constants
static const float Reproj_SubpixelThreshold = 0.5f;
static const float Reproj_GatherBase = 0.5f;
static const float Reproj_GatherSubpixelMotion = 0.1666f;
static const float MinimumHistoryLumaContribution = 0.2f;		// In the range [0.0 1.0]; min proportion contributed by history, regardless of relative luma
static const float VelocityThresholdFullTrust = 2.0f;
static const float VelocityThresholdNoTrust = 15.0f;


float4 TemporalReprojection(float2 ss_tc, float2 ss_vel, float vs_dist)
{
	// Sample the unjittered colour buffer texel, and the (already) unjittered history buffer texel
	float2 uv = ss_tc - C_Jitter.xy;
	float4 texel0 = SampleColour(TAAColourBufferInput, uv);						// Unjitter and sample
	float4 texel1 = SampleColour(TAAHistoryBufferInput, ss_tc - ss_vel);		// Sample back along the velocity vector.  Already unjittered
	
	// Perform 4-tap varying sample to calculate local minima-maxima in colour space
	float2 texel_vel = ss_vel / C_texelsize;
	float texel_vel_mag = length(texel_vel) * vs_dist;
	float k_subpixel_motion = saturate(Reproj_SubpixelThreshold / (texel_vel_mag + FLT_EPS));
	float k_minmax_support = (Reproj_GatherBase + Reproj_GatherSubpixelMotion * k_subpixel_motion);

	float2 ss_offset01 = (k_minmax_support * float2(-C_texelsize.x, C_texelsize.y));
	float2 ss_offset11 = (k_minmax_support * float2( C_texelsize.x, C_texelsize.y));

	float4 c00 = SampleColour(TAAColourBufferInput, uv - ss_offset11);
	float4 c10 = SampleColour(TAAColourBufferInput, uv - ss_offset01);
	float4 c01 = SampleColour(TAAColourBufferInput, uv + ss_offset01);
	float4 c11 = SampleColour(TAAColourBufferInput, uv + ss_offset11);

	float4 cmin = min(c00, min(c10, min(c01, c11)));
	float4 cmax = max(c00, max(c10, max(c01, c11)));

#	if (USE_YCOCG || PERFORM_COLOUR_SPACE_CLIPPING)
		float4 cavg = (c00 + c10 + c01 + c11) * 0.25f;
#	endif

	// Shrink chroma min/max if using YCoCg colour space
#	if USE_YCOCG
		float2 chroma_extent = 0.25 * 0.5 * (cmax.r - cmin.r);
		float2 chroma_centre = texel0.gb;
		cmin.yz = (chroma_centre - chroma_extent);
		cmax.yz = (chroma_centre + chroma_extent);
		cavg.yz = chroma_centre;
#	endif

	// Restrict to the colour-space neighbourhood of the current pixel
#	if PERFORM_COLOUR_SPACE_CLIPPING
		texel1 = ColourSpaceClipAABB(cmin.xyz, cmax.xyz, clamp(cavg, cmin, cmax), texel1);
#	else 
		texel1 = clamp(texel1, cmin, cmax);
#	endif

	// Calculate feedback weight (% influence from history) based on difference in unbiased luma
#	if USE_YCOCG
		float lum0 = texel0.r;
		float lum1 = texel1.r;
#	else
		float lum0 = Luminance(texel0.rgb);
		float lum1 = Luminance(texel1.rgb);
#	endif

	float unbiased_delta = abs(lum0 - lum1) / max(lum0, max(lum1, MinimumHistoryLumaContribution));
	float unbiased_weight = (1.0f - unbiased_delta);
	float unbiased_weight_sq = (unbiased_weight * unbiased_weight);
	float k_feedback = lerp(C_FeedbackMin, C_FeedbackMax, unbiased_weight_sq);

	// Blend between current and history based upon calculated feedback
	return lerp(texel0, texel1, k_feedback);
}



// Temporal anti-aliasing; temporal reprojection phase: entry point
TemporalAAPixelShaderOutput PS_Temporal(ScreenSpaceQuadVertexShaderOutput IN)
{
	TemporalAAPixelShaderOutput OUT;					// MRT output to { reprojection buffer, primary colour buffer }

	// Reverse UV jittering for reprojection
	float2 uv = IN.texCoord - C_Jitter.xy;				// .xy is the current frame jitter (zw is prior frame)

	// Velocity dilation based on 3x3 neighbourhood
#	if ENABLE_VELOCITY_DILATION
		float3 closest = ClosestFragmentIn3x3Neighbourhood(uv);
		float2 ss_vel = RevertNormalisationScaleBias(TAAVelocityBufferInput.Sample(LinearRepeatSampler, closest.xy).xy);
		float vs_dist = LinearEyeDepth(closest.z);
#	else
		float2 ss_vel = RevertNormalisationScaleBias(TAAVelocityBufferInput.Sample(LinearRepeatSampler, uv).xy);
		float raw_dist = TAADepthBufferInput.Sample(PointClampSampler, uv);
		float vs_dist = LinearEyeDepth(raw_dist);
#	endif

	// Perform temporal re-projection
	float4 colour_temporal = TemporalReprojection(IN.texCoord, ss_vel, vs_dist);
	float4 buffer_output = ResolveColour(colour_temporal);

	// Incorporate motion blur data if applicable
	// TODO: In future, if MotionBlur+TAA are both enabled, we can avoid performing the full & costly
	// motion blur gather phase.  Perform everything up to neighbour sampling and then run the gather
	// logic only for pixels in this shader where trust < max_threshold (if % is small enough to be worth it)
#	if MOTION_BLUR_BLEND
		const float VelocityThresholdTrustSpan = (VelocityThresholdNoTrust - VelocityThresholdFullTrust);
		float vel_mag = length(ss_vel * C_buffersize);
		float trust = 1.0f - (clamp(vel_mag - VelocityThresholdFullTrust, 0.0f, VelocityThresholdTrustSpan) / VelocityThresholdTrustSpan);	// [0.0 1.0]

		float4 colour_motion = TAAMotionBlurFinalInput.Sample(LinearRepeatSampler, uv);

#	if DEBUG_TEMPORAL_MOTION_BLEND
		colour_motion = (colour_motion * 0.5f) + float4(0.5f, 0, 0, 0.5f);
		colour_temporal = (colour_temporal * 0.5f) + float4(0, 0.5f, 0, 0.5f);
#	endif


		float4 screen_output = ResolveColour(lerp(colour_motion, colour_temporal, trust));
#	else
		float4 screen_output = ResolveColour(colour_temporal);
#	endif

	/* TODO: Add blue noise component */
	float4 noise = float4(0, 0, 0, 0);

	// Return the final calculated outputs
	OUT.ReprojectionBufferOutput = saturate(buffer_output + noise);
	OUT.ColourBufferOutput = saturate(screen_output + noise);
	return OUT;
}
