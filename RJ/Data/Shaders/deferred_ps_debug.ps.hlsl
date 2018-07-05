#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "DeferredRendererDebugRenderingData.hlsl"


// GBuffer texture target bindings; up to 16 bindings are allowed
Texture2D DebugSourceTexture[16] : register(t0);

// Exponent applied to generate enough contrast in depth data for visualisation
static const float DEPTH_EXP_FACTOR = 6.0f;

// Constants
static const float4 EMPTY = float4(0, 0, 0, 0);

// Pixel shader that generates the G-Buffer
[earlydepthstencil]
float4 PS_Deferred_Debug(ScreenSpaceQuadVertexShaderOutput IN) : SV_Target0
{
	// Determine layout based on number of active views
	uint viewcount = C_debug_view_count;
	uint views_dim = (viewcount > 9 ? 4 : (viewcount > 4 ? 3 : (viewcount > 1 ? 2 : 1)));

	// Determine which view this pixel lies in, and early-exist if this view is not active
	float2 view_size = (IN.C_buffer_size / float2(views_dim, views_dim));
	uint2 view = floor(IN.texCoord / view_size);
	uint view_id = view.x + (view.y * views_dim);

	if (view_id > viewcount) return EMPTY;

	// Determine texcoord within the (potentially downsampled) source texture
	float view_scale_factor = (1.0f / (float)views_dim);
	float2 view_texcoord = (IN.texCoord * view_scale_factor);

	// Render differently based upon the source texture type
	if (C_debug_view[view_id] == DEF_DEBUG_STATE_ENABLED_NORMAL)
	{
		return DebugSourceTexture[view_id].Sample(LinearClampSampler, view_texcoord);
	}
	else if (C_debug_view[view_id] == DEF_DEBUG_STATE_ENABLED_DEPTH)
	{
		return pow(DebugSourceTexture[view_id].Sample(LinearClampSampler, IN.texCoord).rrrr, DEPTH_EXP_FACTOR);
	}
	
	// Default in case of unrecognised view state
	return EMPTY;
}
