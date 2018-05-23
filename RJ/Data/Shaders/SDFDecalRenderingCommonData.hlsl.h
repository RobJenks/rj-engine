#ifndef __SDFDecalRenderingCommonDataH__
#define __SDFDecalRenderingCommonDataH__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"

// Basic constant buffer holding minimum per-frame data required
CBUFFER DecalRenderingFrameBuffer REGISTER(b4)
{
	RJ_ROW_MAJOR_MATRIX ViewProjection;		// [TODO: TO BE CHANGED]
};



// String references to each buffer for application parameter binding
#define DecalRenderingFrameBufferName BUFFER_NAME(DecalRenderingFrameBuffer)





#endif