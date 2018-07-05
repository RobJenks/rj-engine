#ifndef __BasicTextureRenderingCommonDataH__
#define __BasicTextureRenderingCommonDataH__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"

// Basic constant buffer holding minimum per-frame data required
CBUFFER BasicTextureRenderingFrameBuffer REGISTER(b4)
{
	RJ_ROW_MAJOR_MATRIX ViewProjectionMatrix;		// (View * Proj) matrix
};



// String references to each buffer for application parameter binding
#define BasicTextureRenderingFrameBufferName BUFFER_NAME(BasicTextureRenderingFrameBuffer)





#endif