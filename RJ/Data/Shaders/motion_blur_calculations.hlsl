#ifndef __MotionBlurCalculationsHLSL__
#define __MotionBlurCalculationsHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "motion_blur_resources.hlsl"


// HLSL-only constants
static const float4 NO_VEL = float4(0.5f, 0.5f, 0.5f, 1.0f);	// Equates to a zero velocity vector in biased/scaled velocity space


// Determine size of the given texture
// TODO: REMOVE THIS.  Replace with cbuffer value passed into shader.  Split motion blur params into their own CB for this since smaller CBs = in registers = ideal
float2 DetermineTextureDimensions(Texture2D tex)
{
	uint w, h;
	tex.GetDimensions(w, h);
	return float2(w, h);
}




#endif