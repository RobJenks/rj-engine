#ifndef __NoiseCalculationsHLSL__
#define __NoiseCalculationsHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "noise_buffers.hlsl.h"


// Indicates whether noise data will be conformed to a triangular distribution, which is more accurate but (a) requires
// a sqrt per sample, and (b) may not look as good
static const bool NOISE_GENERATION_CONFORM_TO_TRIANGULAR_DIST = false;


// Return a float containing a single item of random noise data, based upon the contents of the noise buffer and texture resources
// Input seed should vary over the area being rendered; for example, source UV coords for the texture being rendered
float RandomNoise(uint2 inputSeed)
{
	// Make sure noise generation resources are available
	if (!NoiseResourceAvailable) return 0.0f;

	// (x & 0x3F) == (x & 63) == uint from 0 and 63 inclusive
	float noise = NoiseTextureData.Load(uint4((inputSeed + RandomNoiseSeed.xy) & (NOISE_TEXTURE_SIZE - 1), 
												RandomNoiseSeed.w & (NOISE_TEXTURE_DEPTH - 1), 0)).r;

	// Conform to a triangular distribution if required (more accurate; looks as good?)
	if (NOISE_GENERATION_CONFORM_TO_TRIANGULAR_DIST)
	{
		noise = mad(noise, 2.0f, -1.0f);
		noise = sign(noise)*(1.0f - sqrt(1.0f - abs(noise)));
	}

	// Return the noise value
	return noise;
}

// TODO: Fix issue with loading multi-channel noise resources (for HDR) and implement this
// Return a float2 containing random noise data, based upon the contents of the noise buffer and texture resources
// Input seed should vary over the area being rendered; for example, source UV coords for the texture being rendered
float2 RandomNoise2(uint2 inputSeed)
{
	return float2(0, 0);
}

// TODO: Fix issue with loading multi-channel noise resources (for HDR) and implement this
// Return a float3 containing random noise data, based upon the contents of the noise buffer and texture resources
// Input seed should vary over the area being rendered; for example, source UV coords for the texture being rendered
float3 RandomNoise3(uint2 inputSeed)
{
	return float3(0, 0, 0);
}



#endif