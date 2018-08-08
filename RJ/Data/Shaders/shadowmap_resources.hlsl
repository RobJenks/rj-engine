#ifndef __ShadowMapResourcesHLSL__
#define __ShadowMapResourcesHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"


// Shadow map vertex shader output.  Minimal with geometric data only
struct ShadowMappingVertexShaderOutput
{
	float4 position					RJ_SEMANTIC(SV_POSITION);	// Clip-space position
};


// Constant buffer used for light-space shadow mapping
CBUFFER LightSpaceShadowMapDataBuffer REGISTER(b4)
{
	RJ_ROW_MAJOR_MATRIX				LightViewProjection;

	// Padding
	//float# _padding_sm;			// CB size must be a multiple of 16 bytes
};

// Shadow map depth texture resource; either bound as SM render target, or as lighting PS input
TEXTURE2D ShadowMapTexture REGISTER(t8);


// String references to each buffer for application parameter binding
#define LightSpaceShadowMapDataBufferName BUFFER_NAME(LightSpaceShadowMapDataBuffer)
#define ShadowMapTextureName BUFFER_NAME(ShadowMapTexture)


#endif