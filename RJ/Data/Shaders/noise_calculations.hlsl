#ifndef __NoiseCalculationsHLSL__
#define __NoiseCalculationsHLSL__

#include "../Definitions/CppHLSLLocalisation.hlsl.h"
#include "noise_buffers.hlsl.h"


// Noise texture data will be bound to this texture array
Texture2DArray NoiseTextureData;


// Return a float3 containing random noise data, based upon the contents of the noise buffer and texture resources
// Input seed should vary over the area being rendered; for example, source UV coords for the texture being rendered
float3 RandomNoise(uint2 inputSeed)
{
	// (x & 0x3F) == (x & 63) == uint from 0 and 63 inclusive
	return NoiseTextureData.Load(uint4((inputSeed + RandomNoiseSeed.xy) & 0x3F, RandomNoiseSeed.w & 0x3F, 0)).rgb;
}







#endif