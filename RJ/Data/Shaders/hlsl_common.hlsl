#include "../../CommonShaderConstantBufferDefinitions.hlsl.h"

/*
	Contains general functions used across multiple HLSL shader implementations
*/

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