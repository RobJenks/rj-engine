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

// Constant buffer used for debug pixel shader rendering of GBuffer data
CBUFFER DeferredRenderingParamBuffer REGISTER(b2)
{
	_bool debug_view_is_depth_texture;		// Indicates whether the debug view is a depth texture, otherwise will treat as three-component & alpha colour
	
	float3 _padding_dbg;					// CB size must be a multiple of 16 bytes
};


// String references to each buffer binding
#define LightIndexBufferName BUFFER_NAME(LightIndexBuffer)
#define DeferredRenderingParamBufferName BUFFER_NAME(DeferredRenderingParamBuffer)


#endif