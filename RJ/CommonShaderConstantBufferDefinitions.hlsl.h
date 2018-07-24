#ifndef __CommonShaderConstantBufferDefinitionsH__
#define __CommonShaderConstantBufferDefinitionsH__

#include "../Definitions/CppHLSLLocalisation.hlsl.h"
#include "../Definitions/MaterialData.hlsl.h"

// Holds standard per-frame data such as view/projection transforms
CBUFFER FrameDataBuffer REGISTER(b0)
{
	RJ_ROW_MAJOR_MATRIX View;
	RJ_ROW_MAJOR_MATRIX Projection;
	RJ_ROW_MAJOR_MATRIX ViewProjection;
	RJ_ROW_MAJOR_MATRIX InvProjection;
	RJ_ROW_MAJOR_MATRIX PriorFrameViewProjection;

	RJ_ROW_MAJOR_MATRIX ProjectionUnjittered;
	RJ_ROW_MAJOR_MATRIX PriorFrameViewProjectionUnjittered;

	float2 ScreenDimensions;
	float2 padding;
};

// Holds material data for the current render pass.  Currently accepts only one material.  Could
// be extended to a (fixed) array and use a per-vertex material index.  However adds storage req to 
// both buffer and vertex structure, plus would require changing ModelBuffer impl.  Perhaps better
// to continue rendering all of a single material before moving to the next, even if a model 
// has >1 material
CBUFFER MaterialBuffer REGISTER(b2)
{
	MaterialData Mat;
};


// String references to each buffer for application parameter binding
#define FrameDataBufferName BUFFER_NAME(FrameDataBuffer)
#define MaterialBufferName BUFFER_NAME(MaterialBuffer)



#endif