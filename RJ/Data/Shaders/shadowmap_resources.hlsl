#ifndef __ShadowMapResourcesHLSL__
#define __ShadowMapResourcesHLSL__


// Shadow map vertex shader output.  Minimal with geometric data only
struct ShadowMappingVertexShaderOutput
{
	float4 position					RJ_SEMANTIC(SV_POSITION);	// Clip-space position
};



#endif