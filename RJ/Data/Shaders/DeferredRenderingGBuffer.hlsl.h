#ifndef __DeferredRenderingGBufferHLSL__
#define __DeferredRenderingGBufferHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"

// GBuffer register bindings
#define GBuffer_Register_Diffuse t0
#define GBuffer_Register_Specular t1
#define GBuffer_Register_Normal t2
#define GBuffer_Register_Velocity t3
#define GBuffer_Register_Depth t4

// GBuffer texture target bindings
TEXTURE2D GBuffer_DiffuseTextureVS REGISTER(GBuffer_Register_Diffuse);
TEXTURE2D GBuffer_SpecularTextureVS REGISTER(GBuffer_Register_Specular);
TEXTURE2D GBuffer_NormalTextureVS REGISTER(GBuffer_Register_Normal);
TEXTURE2D GBuffer_VelocityTextureSS REGISTER(GBuffer_Register_Velocity);
TEXTURE2D GBuffer_DepthTextureVS REGISTER(GBuffer_Register_Depth);

// Define common names for the GBuffer texture resources
#define GBufferDiffuseTextureName BUFFER_NAME(GBuffer_DiffuseTextureVS)
#define GBufferSpecularTextureName BUFFER_NAME(GBuffer_SpecularTextureVS)
#define GBufferNormalTextureName BUFFER_NAME(GBuffer_NormalTextureVS)
#define GBufferVelocityTextureName BUFFER_NAME(GBuffer_VelocityTextureSS)
#define GBufferDepthTextureName BUFFER_NAME(GBuffer_DepthTextureVS)


#endif