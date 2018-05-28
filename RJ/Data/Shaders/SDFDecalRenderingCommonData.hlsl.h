#ifndef __SDFDecalRenderingCommonDataH__
#define __SDFDecalRenderingCommonDataH__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"


// Output of SDF decal rendering VS stage
struct SDFDecalRenderingVertexShaderOutput
{
	float4 position				RJ_SEMANTIC(SV_POSITION);	// Clip-space position
	float2 texCoord				RJ_SEMANTIC(TEXCOORD0);		// Texture coordinate

	float2 uv_shift				RJ_SEMANTIC(UVSHIFT);		// Linear shift of UV coords within the texture being sampled
	float2 uv_scale				RJ_SEMANTIC(UVSCALE);		// Linear scale of UV coords within the texture being sampled
};


// Basic constant buffer holding minimum per-frame data required
CBUFFER DecalRenderingFrameBuffer REGISTER(b4)
{
	RJ_ROW_MAJOR_MATRIX ViewProjection;		// (View * Proj) matrix
};


// Basic constant buffer holding required decal rendering data
CBUFFER DecalRenderingDataBuffer REGISTER(b4)
{
	float4		baseColour;
	float4		outlineColour;
	float		outlineDistanceFactor;	// In the range 0.0 (no outline) to 1.0 (thick outline)

	float3		_padding;
};



// String references to each buffer for application parameter binding
#define DecalRenderingFrameBufferName BUFFER_NAME(DecalRenderingFrameBuffer)
#define DecalRenderingDataBufferName BUFFER_NAME(DecalRenderingDataBuffer)






#endif