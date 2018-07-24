#ifndef __DeferredRenderingBuffersHLSL__
#define __DeferredRenderingBuffersHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"


// Constant buffer holding current index into the light buffer (TODO: Can be combined with other cbuffer data?)
CBUFFER LightIndexBuffer REGISTER(b4)
{
	// The index of the light in the 'Lights' buffer
	_uint32 LightIndex;

	// CB size must be a multiple of 16 bytes
	float3 _padding;
};

// Constant buffer used for pixel shader rendering of GBuffer data
CBUFFER DeferredRenderingParamBuffer REGISTER(b3)
{
	// General data
	uint2 C_buffersize;							// Size of the primary render buffers (px)
	float2 C_texelsize;							// Texel size of the primary render buffers, 1.0/buffersize (i.e. pos*texelsize = [0.0 1.0])
	float C_frametime;							// Elapsed time for the frame (secs)

	// Velocity calculation data
	unsigned int C_k;								// Constant K in the velocity calculation, determining the screen-space radius considered
	float C_half_exposure;							// 0.5 * camera exposure
	float C_half_frame_exposure;					// 0.5 * the camera exposure weighted by frame time (0.5 * (exposure / frametime_secs))
	unsigned int C_motion_samples;					// Number of samples taken along the velocity vector when calculating motion blur
	unsigned int C_motion_max_sample_tap_distance;	// Max distance for reconstruction tap samples along the velocity vector when calculating motion blur

	// Padding
	float2 _padding_def;							// CB size must be a multiple of 16 bytes

	// Frustum jitter for the current and prior frame
	float4 C_Jitter;								// xy = current frame UV jitter, zw = prior frame UV jitter
};


// String references to each buffer binding
#define LightIndexBufferName BUFFER_NAME(LightIndexBuffer)
#define DeferredRenderingParamBufferName BUFFER_NAME(DeferredRenderingParamBuffer)


#endif