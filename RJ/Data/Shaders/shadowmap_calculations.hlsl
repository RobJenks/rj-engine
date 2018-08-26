#ifndef __ShadowMapCalculationsHLSL__
#define __ShadowMapCalculationsHLSL__

#include "shadowmap_resources.hlsl"
#include "shadowmap_pcf_kernels.hlsl"
#include "DeferredRenderingBuffers.hlsl"

// Macros controlling shadow-map features
#define SHADER_SHADOWMAP_PCF				PCF_BOX_3		/* Determines the PCF kernel that will be used */
#define SHADER_APPLY_PCF_KERNEL_WEIGHTS		1				/* Determines whether PCF kernel values will be weighted on a per-tap basis*/
#define SHADER_PCF_KERNEL_SCALE				2				/* Scale factor applied to all PCF kernel offsets (integral recommended since offset is int2 of texels)*/

// Constant epsilon bias allowed between shadow map and projected camera distance calculation
static const float SHADOWMAP_PROJECTION_EPSILON = 0.01f;

// Degree of shadowing applied to occluded pixels
static const float SHADOW_SHADING_FACTOR = 0.75f;


// Calculate the transformation of the given camera UV space coordinate into the current light shadow map space
// Transform: CameraUV -> CamProj -> CamView -> LightView -> LightProj -> Homogenize -> Bias -> ShadowUV
// Input: Camera (GBuffer) UV space			[0.0 1.0]
// Output: Shadow map UV space (x,y,depth)	[0.0 1.0]
float3 CalculateShadowMapUVProjection(float2 camera_uv, float depth)
{
	float4 camera_projected = float4(
		float2(camera_uv.x, 1.0f - camera_uv.y) * 2.0f - 1.0f,
		depth,
		1.0f
	);

	float4 light_projected = mul(camera_projected, CamToLightProjection);
	light_projected /= light_projected.w;

	float2 shadow_uv = (light_projected.xy * 0.5f) + 0.5f;
	shadow_uv.y = (1.0f - shadow_uv.y);
	
	return float3(shadow_uv, light_projected.z);
}


// PCF: Single-tap PCF kernel
float ShadowMapPCFSingle(float2 shadowmap_uv, float camera_depth)
{
	float shadow_pc = ShadowMapTexture.SampleCmpLevelZero(PCFDepthSampler, shadowmap_uv, (camera_depth - SHADOWMAP_PROJECTION_EPSILON));
	return (1.0f - SHADOW_SHADING_FACTOR) + (shadow_pc * SHADOW_SHADING_FACTOR);
}

// Percentage-closer filtering (PCF) with compile-time attached offsets, for use as shadow mapping kernel
float ShadowMapPCFOffsetKernel(float2 shadowmap_uv, float camera_depth)
{
	// Select PCF kernel
#if SHADER_SHADOWMAP_PCF == PCF_BOX_3
#	define KERNEL(x) PCF_KERNEL_BOX3_##x

#else
#	define KERNEL(x) PCF_KERNEL_NULL_##x

#endif

	// If configurable weighting per tap is enabled, shadow contribution must be divided through by weighting sum (instead of simply tap count)
#if SHADER_APPLY_PCF_KERNEL_WEIGHTS
	static const float KERNEL_DIVISOR = KERNEL(WEIGHT_SUM);
#else
	static const float KERNEL_DIVISOR = KERNEL(TAPS);
#endif
 

	// Sample based on the PCF kernel and accumulate the shadowing contribution
	float shadow_pc = 0.0f;

	[unroll]
	for (int i = 0; i < KERNEL(TAPS); ++i)
	{
		float pcfsample = ShadowMapTexture.SampleCmpLevelZero(PCFDepthSampler, shadowmap_uv,
			(camera_depth - SHADOWMAP_PROJECTION_EPSILON), KERNEL(OFFSETS[i]) * SHADER_PCF_KERNEL_SCALE);

#if SHADER_APPLY_PCF_KERNEL_WEIGHTS
		pcfsample *= KERNEL(WEIGHTS[i]);
#endif

		shadow_pc += pcfsample;
	}

	// Return the averaged contribution, accounting for kernel size and relative weights
	static const float WEIGHTING = (1.0f / (KERNEL_DIVISOR * (1.0f / SHADOW_SHADING_FACTOR)));	// E.g. 9 taps, shadingfactor of 0.5 -> WEIGHTING = 1/18
	return (1.0f - SHADOW_SHADING_FACTOR) + (shadow_pc * WEIGHTING);
}



// Compute the shadowing factor based on the currently-bound shadow map and the given UV-biased
// projected coordinates.  
float ComputeShadowFactor(float2 shadowmap_uv, float camera_depth)
{
	// PCF: Single-tap
#if SHADER_SHADOWMAP_PCF == PCF_SINGLE
	return ShadowMapPCFSingle(shadowmap_uv, camera_depth);

	// PCF: multi-tap offset filter kernel
#elif SHADER_SHADOWMAP_PCF == PCF_BOX_3
	return ShadowMapPCFOffsetKernel(shadowmap_uv, camera_depth);

#else

	// Single sample, no PCF
	float shadowmap_depth = ShadowMapTexture.Sample(PointClampSampler, shadowmap_uv).r;
	bool shadowed = ((camera_depth - shadowmap_depth) > SHADOWMAP_PROJECTION_EPSILON);
	return (shadowed ? (1.0F - SHADOW_SHADING_FACTOR) : 1.0f);

#endif

}






#endif