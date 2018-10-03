#ifndef __CommonShaderBufferDefinitionsHLSL__
#define __CommonShaderBufferDefinitionsHLSL__

// Standard constant buffer definitions
#include "../Definitions/CppHLSLLocalisation.hlsl.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"


// Texture bindings to each register
TEXTURE2D AmbientTexture        RJ_REGISTER(t0);
TEXTURE2D EmissiveTexture       RJ_REGISTER(t1);
TEXTURE2D DiffuseTexture        RJ_REGISTER(t2);
TEXTURE2D SpecularTexture       RJ_REGISTER(t3);
TEXTURE2D SpecularPowerTexture  RJ_REGISTER(t4);
TEXTURE2D NormalTexture         RJ_REGISTER(t5);
TEXTURE2D BumpTexture           RJ_REGISTER(t6);
TEXTURE2D OpacityTexture        RJ_REGISTER(t7);

// Sampler states that may be bound to the shader execution
SAMPLERSTATE LinearRepeatSampler		RJ_REGISTER(s0);
SAMPLERSTATE LinearClampSampler			RJ_REGISTER(s1);
SAMPLERSTATE PointRepeatSampler			RJ_REGISTER(s2);
SAMPLERSTATE PointClampSampler			RJ_REGISTER(s3);
SAMPLERCOMPARISONSTATE PCFDepthSampler	RJ_REGISTER(s4);

#endif