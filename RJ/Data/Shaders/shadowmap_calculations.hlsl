#ifndef __ShadowMapCalculationsHLSL__
#define __ShadowMapCalculationsHLSL__

#include "shadowmap_resources.hlsl"
#include "shadowmap_pcf_kernels.hlsl"
#include "DeferredRenderingBuffers.hlsl"

// Macros controlling shadow-map features
#define SHADER_SHADOWMAP_PCF		PCF_BOX_3

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
#if SHADER_SHADOWMAP_PCF == PCF_BOX_3
	static const int TAPS = BOX3_TAPS;
	static const int2 offsets[BOX3_TAPS] = BOX3_OFFSETS;

#else
	static const int TAPS = 1;
	static const int2 offsets[1] = int2(0, 0);

#endif

	float shadow_pc = 0.0f;

	[unroll]
	for (int i = 0; i < TAPS; ++i)
	{
		shadow_pc += ShadowMapTexture.SampleCmpLevelZero(PCFDepthSampler, shadowmap_uv,
			(camera_depth - SHADOWMAP_PROJECTION_EPSILON), offsets[i]);
	}

	static const float WEIGHTING = (1.0f / (TAPS * (1.0f / SHADOW_SHADING_FACTOR)));	// E.g. 9 taps, shadingfactor of 0.5 -> WEIGHTING = 1/18
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