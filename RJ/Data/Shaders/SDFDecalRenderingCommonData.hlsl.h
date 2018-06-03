#ifndef __SDFDecalRenderingCommonDataH__
#define __SDFDecalRenderingCommonDataH__


#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"


// Output of SDF decal rendering VS stage (direct rendering)
struct SDFDecalRenderingVertexShaderOutput
{
	float4 position				RJ_SEMANTIC(SV_POSITION);	// Clip-space position
	float2 texCoord				RJ_SEMANTIC(TEXCOORD0);		// Texture coordinate
};

// Output of SDF decal rendering VS stage (deferred screenspace rendering)
struct SDFDecalRenderingDeferredVertexShaderOutput
{
	float4 position				RJ_SEMANTIC(SV_POSITION);	// Clip-space position
	float4 positionVS			RJ_SEMANTIC(TEXCOORD0);		// View-space position
	float2 texCoord				RJ_SEMANTIC(TEXCOORD1);		// Texture coordinate
	float depth					RJ_SEMANTIC(DEPTH);			// Calculated pixel depth
};

// Basic constant buffer holding minimum per-frame data required
CBUFFER DecalRenderingFrameBuffer REGISTER(b4)
{
	RJ_ROW_MAJOR_MATRIX ViewMatrix;
	RJ_ROW_MAJOR_MATRIX ProjMatrix;
	float FarClipDistance;

	float3 _padding_drfb;
};


// Basic constant buffer holding required decal rendering data
CBUFFER DecalRenderingDataBuffer REGISTER(b4)
{
	float4		baseColour;
	float4		outlineColour;
	float		outlineDistanceFactor;	// In the range 0.0 (no outline) to 1.0 (thick outline)
	float		smoothingFactor;		// Typical values range from 1/4 to 1/16

	float2		_padding_drdb;
};



// String references to each buffer for application parameter binding
#define DecalRenderingFrameBufferName BUFFER_NAME(DecalRenderingFrameBuffer)
#define DecalRenderingDataBufferName BUFFER_NAME(DecalRenderingDataBuffer)






#endif