#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "DeferredRenderingBuffers.hlsl"
#include "hlsl_common.hlsl"
#include "motion_blur_calculations.hlsl"



// Motion blur tile generation: entry point
float4 PS_MotionBlur_Tilegen(ScreenSpaceQuadVertexShaderOutput IN) : SV_Target0
{
	// Default return value; zero motion unless we have specific data in the velocity buffer
	float2 result = float2(0.0f, 0.0f);

	// Determine sample increment across the velocity texture
	float2 texsize = DetermineTextureDimensions(MotionBlurVelocityBufferInput);
	float2 tcStart = IN.texCoord;
	float2 tcInc = float2(1.0f, 1.0f) / texsize;

	// Determing max velocity magnitude^2 within each tile
	float max_magnitude_sq = 0.0f;

	// Iterate across the velocity texture and process all pixels within the current tile
	for (unsigned int u = 0; u < C_k; ++u)
	{
		for (unsigned int v = 0; v < C_k; ++v)
		{
			// Read the encoded velocity vector at this texel and decode it back to the original velocity
			float2 texcoord = tcStart + (float2(u, v) * tcInc);
			float2 vel_encoded = MotionBlurVelocityBufferInput.SampleLevel(PointClampSampler, texcoord, 0).xy;
			float2 velocity = RevertNormalisationScaleBias(vel_encoded);

			// Identify the maximum velocity within this tile
			float mag_sq = dot(velocity, velocity);
			if (max_magnitude_sq < mag_sq)
			{
				result = velocity;
				max_magnitude_sq = mag_sq;
			}
		}
	}

	// Encode back into scaled/biased values for the target UNORM buffer
	return float4(ApplyNormalisationScaleBias(result), 0.0f, 1.0f);
}