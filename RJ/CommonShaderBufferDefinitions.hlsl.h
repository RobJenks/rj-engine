#ifndef __CommonShaderBufferDefinitionsHLSL__
#define __CommonShaderBufferDefinitionsHLSL__

// Standard constant buffer definitions
#include "CommonShaderConstantBufferDefinitions.hlsl.h"


// Texture bindings to each register
Texture2D AmbientTexture        : register(t0);
Texture2D EmissiveTexture       : register(t1);
Texture2D DiffuseTexture        : register(t2);
Texture2D SpecularTexture       : register(t3);
Texture2D SpecularPowerTexture  : register(t4);
Texture2D NormalTexture         : register(t5);
Texture2D BumpTexture           : register(t6);
Texture2D OpacityTexture        : register(t7);

// Sampler states that may be bound to the shader execution
SamplerState LinearRepeatSampler		: register(s0);
SamplerState LinearClampSampler			: register(s1);
SamplerState PointRepeatSampler			: register(s2);
SamplerState PointClampSampler			: register(s3);
SamplerComparisonState PCFDepthSampler	: register(s4);

#endif