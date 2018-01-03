#ifndef __CommonShaderConstantBufferDefinitionsH__
#define __CommonShaderConstantBufferDefinitionsH__

#include "CppHLSLLocalisation.hlsl.h"
#include "MaterialData.hlsl.h"

#define BUFFER_NAME(buffer) #buffer

// Holds standard per-frame data such as view/projection transforms
cbuffer FrameDataBuffer REGISTER(b0) ALIGNED16(FrameDataBuffer)
{
	float4x4 View;
	float4x4 Projection;
	float4x4 InvProjection;
	float2 ScreenDimensions;
};

// Holds material data for the current render pass.  Currently accepts only one material.  Could
// be extended to a (fixed) array and use a per-vertex material index.  However adds storage req to 
// both buffer and vertex structure, plus would require changing ModelBuffer impl.  Perhaps better
// to continue rendering all of a single material before moving to the next, even if a model 
// has >1 material
cbuffer MaterialBuffer REGISTER(b2) ALIGNED16(MaterialBuffer)
{
	MaterialData Mat;
};


// String references to each buffer for application parameter binding
#define FrameDataBufferName BUFFER_NAME(FrameDataBuffer)
#define MaterialBufferName BUFFER_NAME(MaterialBuffer)



#endif