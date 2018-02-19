#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "../../../Definitions/LightData.hlsl.h"

// Structured buffer ("LightBuffer") holding light data (in texture memory, not constant buffer memory)
// Only defined within HLSL context; not required for CPP
#if !defined(__cplusplus) 
	StructuredBuffer<LightData> Lights : register(t8);
#endif


// String references to each buffer binding
#define LightBufferName "LightBuffer"

