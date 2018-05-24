#ifndef __SDFDecalRenderingCommonDataH__
#define __SDFDecalRenderingCommonDataH__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"

// Basic constant buffer holding required decal rendering data
CBUFFER DecalRenderingDataBuffer REGISTER(b4)
{
	float4 tmp;
};



// String references to each buffer for application parameter binding
#define DecalRenderingDataBufferName BUFFER_NAME(DecalRenderingDataBuffer)





#endif