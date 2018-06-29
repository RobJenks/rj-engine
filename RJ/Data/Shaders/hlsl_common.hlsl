#include "../../CommonShaderConstantBufferDefinitions.hlsl.h"

/*
	Contains general functions used across multiple HLSL shader implementations
*/

// Constants
static const float2 NULL_VECTOR2 = float2(0.0f, 0.0f);
static const float3 NULL_VECTOR3 = float3(0.0f, 0.0f, 0.0f);
static const float4 NULL_VECTOR4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
static const float2 ONE_VECTOR2 = float2(1.0f, 1.0f);
static const float3 ONE_VECTOR3 = float3(1.0f, 1.0f, 1.0f);
static const float4 ONE_VECTOR4 = float4(1.0f, 1.0f, 1.0f, 1.0f);
static const float2 TWO_VECTOR2 = float2(2.0f, 2.0f);
static const float3 TWO_VECTOR3 = float3(2.0f, 2.0f, 2.0f);
static const float4 TWO_VECTOR4 = float4(2.0f, 2.0f, 2.0f, 2.0f);
static const float2 HALF_VECTOR2 = float2(0.5f, 0.5f);
static const float3 HALF_VECTOR3 = float3(0.5f, 0.5f, 0.5f);
static const float4 HALF_VECTOR4 = float4(0.5f, 0.5f, 0.5f, 0.5f);


// Convert clip space coordinates to view space
float4 ClipToView(float4 clip)
{
	// View space position
	float4 view = mul(clip, InvProjection);

	// Perspective projection
	view = view / view.w;
	return view;
}


// Convert screen space coordinates to view space
float4 ScreenToView(float4 screen)
{
	// Convert to normalized texture coordinates
	float2 texCoord = screen.xy / ScreenDimensions;

	// Convert to clip space
	float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);

	// Convert from clip to view space and return
	return ClipToView(clip);
}

// Scale and bias a [-1 +1] input to the range [0 +1]
float2 ApplyNormalisationScaleBias(float2 input)
{
	return ((input + ONE_VECTOR2) * HALF_VECTOR2);
}

// Perform inverse scale and bias to revert normalised [0 +1] back to [-1 +1]
float2 RevertNormalisationScaleBias(float2 input)
{
	return ((input * TWO_VECTOR2) - ONE_VECTOR2);
}