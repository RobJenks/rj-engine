#ifndef __ShadowMapCalculationsHLSL__
#define __ShadowMapCalculationsHLSL__

#include "shadowmap_resources.hlsl"

// Constant epsilon bias allowed between shadow map and projected camera distance calculation
static const float SHADOWMAP_PROJECTION_EPSILON = 0.01f;

// Degree of shadowing applied to occluded pixels
static const float SHADOW_SHADING_FACTOR = 0.5f;


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

// Compute the shadowing factor based on the currently-bound shadow map and the given UV-biased
// projected coordinates.  
float _ComputeShadowFactor(float2 shadowmap_uv, float camera_depth)
{
	float shadowmap_depth = ShadowMapTexture.Sample(PointClampSampler, shadowmap_uv).r;


	bool shadowed = ((camera_depth - shadowmap_depth) > SHADOWMAP_PROJECTION_EPSILON);

	return (shadowed ? SHADOW_SHADING_FACTOR : 1.0f);
}

float ComputeShadowFactor(float2 shadowmap_uv, float camera_depth)
{
	float shadow_pc = (camera_depth == 13.0f ?
		ShadowMapTexture.Sample(PointClampSampler, shadowmap_uv).r :
		ShadowMapTexture.SampleCmpLevelZero(PCFDepthSampler, shadowmap_uv, camera_depth)
	);

	return shadow_pc;
}





#endif