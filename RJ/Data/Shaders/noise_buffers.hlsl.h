#ifndef __NoiseBuffersHlslH__
#define __NoiseBuffersHlslH__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"


// Noise texture data will be bound to this texture array
TEXTURE2DARRAY NoiseTextureData REGISTER(t5);

// Noise data all has a consistent size/depth based on the generation process for simplicity
static const _uint32 NOISE_TEXTURE_SIZE = 64U;
static const _uint32 NOISE_TEXTURE_DEPTH = 64U;

// Holds noise buffer data required for indexing into the noise texture resoruces
CBUFFER NoiseDataBuffer
{
	_bool		NoiseResourceAvailable;
	uint3		_padding_noise;			// PADDING
	
	//------------------------------------------ ( 16 bytes )

	uint4		RandomNoiseSeed;		// Generated each frame for use in selecting noise data

	//------------------------------------------ ( 16 bytes )
};


// String references to each buffer for application parameter binding
#define NoiseTextureDataName BUFFER_NAME(NoiseTextureData)
#define NoiseDataBufferName BUFFER_NAME(NoiseDataBuffer)





#endif