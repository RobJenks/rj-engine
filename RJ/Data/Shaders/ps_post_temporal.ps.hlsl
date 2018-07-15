#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "DeferredRenderingBuffers.hlsl"
#include "hlsl_common.hlsl"

// Compile-time debug constants
#define ENABLE_VELOCITY_DILATION			1


// Temporal anti-aliasing; temporal reprojection phase: entry point
float4 PS_Temporal(ScreenSpaceQuadVertexShaderOutput IN) : SV_Target0
{
	// Reverse UV jittering for reprojection
	float2 uv = IN.texCoord - C_jitter.xy;				// .xy is the current frame jitter (zw is prior frame)

	// Velocity dilation based on 3x3 neighbourhood
#	if ENABLE_VELOCITY_DILATION
		float3 closest = ClosestFragmentIn3x3Neighbourhood(uv);
		float2 ss_vel = GBuffer_VelocityTextureSS.Sample(closest.xy).xy;
		*** CONTINUE HERE 351 ***

#	else

#	endif

}
