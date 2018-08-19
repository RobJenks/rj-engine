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

// Reconstruct world-space position from the given texcoord [0 1] and depth value
// TODO: NOT TESTED
float4 ReconstructWorld(float2 texCoord, float depth)
{
	float4 projected = float4(
		float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, 
		depth,
		1.0f
	);
	
	return mul(projected, CameraInverseViewProjection);
	// TODO: divide through by W first?
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

// Convert RGB to YCoCg colour space
// https://software.intel.com/en-us/node/503873
float3 RGB_YCoCg(float3 c)
{
	// Y = R/4 + G/2 + B/4
	// Co = R/2 - B/2
	// Cg = -R/4 + G/2 - B/4
	return float3(
		c.x / 4.0 + c.y / 2.0 + c.z / 4.0,
		c.x / 2.0 - c.z / 2.0,
		-c.x / 4.0 + c.y / 2.0 - c.z / 4.0
	);
}

// Convert YCoCg to RGB colour space
// https://software.intel.com/en-us/node/503873
float3 YCoCg_RGB(float3 c)
{
	// R = Y + Co - Cg
	// G = Y + Cg
	// B = Y - Co - Cg
	return saturate(float3(
		c.x + c.y - c.z,
		c.x + c.z,
		c.x - c.y - c.z
	));
}

// Return luminance for the given RGB
// TODO: Requires LINEAR rgb input, not gamma-corrected input, in whch case the result 
// should be exact (https://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color)
float Luminance(float3 rgb)
{
	return (0.2126*rgb.r + 0.7152*rgb.g + 0.0722*rgb.b);
}

// Return perceived luminance for the given RGB
float LuminancePerceived(float3 rgb)
{
	return (0.299*rgb.r + 0.587*rgb.g + 0.114*rgb.b);
}

// Return more a precise luminance for the given RGB, but at greater cost (incl 1x sqrt) 
float3 LuminancePrecise(float3 rgb)
{
	return sqrt((0.299 * rgb.r * rgb.r) + (0.587 * rgb.g * rgb.g) + (0.114 * rgb.b * rgb.b));
}