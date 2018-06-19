#ifndef __NoiseBuffersHlslH__
#define __NoiseBuffersHlslH__

#include "../Definitions/CppHLSLLocalisation.hlsl.h"


// Holds noise buffer data required for indexing into the noise texture resoruces
CBUFFER NoiseDataBuffer
{
	_uint32		TextureWidth;
	_uint32		TextureHeight;
	_uint32		TextureDepth;

	_uint32		_padding_noise;		// PADDING
	
	//------------------------------------------ ( 16 bytes )

	uint4		RandomNoiseSeed;	// Generated each frame for use in selecting noise data

	//------------------------------------------ ( 16 bytes )
};


// String references to each buffer for application parameter binding
#define NoiseDataBufferName BUFFER_NAME(NoiseDataBuffer)





#endif