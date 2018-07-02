#ifndef __MotionBlurCalculationsHLSL__
#define __MotionBlurCalculationsHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "motion_blur_resources.hlsl"


// HLSL-only constants
static const float4 NO_VEL = float4(0.5f, 0.5f, 0.5f, 1.0f);	// Equates to a zero velocity vector in biased/scaled velocity space
static const float SOFT_Z_EXTENT = 0.10f;						// For cone sampling
static const float CYLINDER_CORNER_1 = 0.95f;					// Cylinder-sampling constant
static const float CYLINDER_CORNER_2 = 1.05f;					// Cylinder-sampling constant


// Determine size of the given texture
// TODO: REMOVE THIS.  Replace with cbuffer value passed into shader.  Split motion blur params into their own CB for this since smaller CBs = in registers = ideal
float2 DetermineTextureDimensions(Texture2D tex)
{
	uint w, h;
	tex.GetDimensions(w, h);
	return float2(w, h);
}

// All motion blur gather operations are in inverse depth space
float InvPointDepth(float2 tex)
{
	return -(MotionBlurDepthBufferInput.SampleLevel(PointClampSampler, tex, 0).r);
}

// Cone sampling
float Cone(float magDiff, float magV)
{
	return 1.0f - abs(magDiff) / magV;
}
// Cylinder sampling
float Cylinder(float magDiff, float magV)
{
	return 1.0 - smoothstep(CYLINDER_CORNER_1 * magV, CYLINDER_CORNER_2 * magV, abs(magDiff));
}
// Soft depth comparison
float SoftDepthComparison(float za, float zb)
{
	return clamp((1.0 - (za - zb) / SOFT_Z_EXTENT), 0.0, 1.0);
}




#endif