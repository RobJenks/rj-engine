#include "../../../../Definitions/MaterialData.hlsl.h"

// Holds standard per-frame data such as view/projection transforms
cbuffer FrameDataBuffer : register(b0)
{
	float4x4 View;
	float4x4 Projection;
	float4x4 InvProjection;
	float2 ScreenDimensions;
}

// Holds material data for the current render pass.  Currently accepts only one material.  Could
// be extended to a (fixed) array and use a per-vertex material index.  However adds storage req to 
// both buffer and vertex structure, plus would require changing ModelBuffer impl.  Perhaps better
// to continue rendering all of a single material before moving to the next, even if a model 
// has >1 material
cbuffer MaterialBuffer : register(b2)
{
	MaterialData Mat;
};

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
sampler LinearRepeatSampler     : register(s0);
sampler LinearClampSampler      : register(s1);

