#ifndef __NoiseCalculationsHLSL__
#define __NoiseCalculationsHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "noise_buffers.hlsl.h"


// Return a float3 containing random noise data, based upon the contents of the noise buffer and texture resources
// Input seed should vary over the area being rendered; for example, source UV coords for the texture being rendered
float3 RandomNoise(uint2 inputSeed)
{
	// Make sure noise generation resources are available
	// TODO: Update buffer to hold only this flag, can discard the rest that are not used
	if (TextureWidth == 0U) return float3(0.0f, 0.0f, 0.0f);

	// (x & 0x3F) == (x & 63) == uint from 0 and 63 inclusive
	return NoiseTextureData.Load(uint4((inputSeed + RandomNoiseSeed.xy) & 0x3F, RandomNoiseSeed.w & 0x3F, 0)).rgb;
}







#endif