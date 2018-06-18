#ifndef __NoiseCalculationsHLSL__
#define __NoiseCalculationsHLSL__

#include "../Definitions/CppHLSLLocalisation.hlsl.h"


// Noise texture data will be bound to this texture array
Texture2DArray NoiseTextureData;

// All noise texture resources have the same constant depth to avoid passing additional data
static const unsigned int NoiseTextureDepth = 64U;


// Return a random noise value in the range [0 1]








#endif