#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "DeferredRenderingBuffers.hlsl"
#include "hlsl_common.hlsl"
#include "motion_blur_calculations.hlsl"



// Motion blur neighbourhood determination: entry point
float4 PS_MotionBlur_Neighbourhood(VertexShaderStandardOutput IN) : SV_Target0
{
	// Default return value; zero motion unless we have specific data in the velocity buffer
	float2 result = float2(0.0f, 0.0f);

	// Determine sample increment across the velocity texture
	float2 texsize = DetermineTextureDimensions(MotionBlurVelocityTileBufferInput);
	float2 tcStart = IN.texCoord;
	float2 tcInc = float2(1.0f, 1.0f) / texsize;

	// Shader will determine max velocity within neighbourhood of relevant tiles
	float max_magnitude_sq = 0.0f;

	// Iterate across local neighbourhood
	for (int u = -1; u <= 1; ++u)
	{
		for (int v = -1; v <= 1; ++v)
		{
			// Read the encoded (max) velocity vector for the given tile and decode it back to the original velocity
			float2 texcoord = tcStart + (float2(u, v) * tcInc);
			float2 vel_encoded = MotionBlurVelocityTileBufferInput.SampleLevel(PointClampSampler, texcoord, 0).xy;
			float2 velocity = RevertNormalisationScaleBias(vel_encoded);

			// Test whether this tile exceeds the current max velocity vector
			float mag_sq = dot(velocity, velocity);
			if (max_magnitude_sq < mag_sq)
			{
				// Test whether velocity is following the direction of max displacement (otherwise discard as noise)
				float displacement = abs(float(u)) + abs(float(v));
				float2 orient = sign(float2(u, v) * velocity);
				float distance = (orient.x + orient.y);

				if (abs(distance) == displacement)
				{
					result = velocity;
					max_magnitude_sq = mag_sq;
				}
			}
		}
	}

	return float4(ApplyNormalisationScaleBias(result), 0.0f, 1.0f);
}