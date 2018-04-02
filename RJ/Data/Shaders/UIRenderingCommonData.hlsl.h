#ifndef __UIRenderingCommonDataH__
#define __UIRenderingCommonDataH__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"

// Basic constant buffer holding minimum per-frame data required
CBUFFER UIRenderingFrameBuffer REGISTER(b4)
{
	RJ_ROW_MAJOR_MATRIX ViewProjection;		// (View * Proj) matrix
};



// String references to each buffer for application parameter binding
#define UIRenderingFrameBufferName BUFFER_NAME(UIRenderingFrameBuffer)





#endif