#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "DeferredRendererDebugRenderingData.hlsl"


// GBuffer texture target bindings
Texture2D DebugSourceTexture[DEF_DEBUG_MAX_RENDER_VIEWS] : register(t0);

// Exponent applied to generate enough contrast in depth data for visualisation
static const float DEPTH_EXP_FACTOR = 6.0f;

// Constants
static const float4 EMPTY = float4(0, 0, 0, 0);

// Sampler-support methods
float4 ContextDependentSample(Texture2D tex, float2 texcoord, uint buffer_type)
{
	if (buffer_type == DEF_DEBUG_STATE_ENABLED_NORMAL)
	{
		return tex.Sample(LinearClampSampler, texcoord);
	}
	else if (buffer_type == DEF_DEBUG_STATE_ENABLED_DEPTH)
	{
		return pow(tex.Sample(LinearClampSampler, texcoord).rrrr, DEPTH_EXP_FACTOR);
	}

	return EMPTY;
}

float4 DebugViewSample(uint view_id, float2 texcoord)
{
	[unroll]
	for (uint i = 0U; i < 16U; ++i)
	{
		if (i == view_id) return ContextDependentSample(DebugSourceTexture[i], texcoord, C_debug_view[i].state);
	}

	return EMPTY;
}


// Pixel shader that generates the G-Buffer
[earlydepthstencil]
float4 PS_Deferred_Debug(ScreenSpaceQuadVertexShaderOutput IN) : SV_Target0
{
	// Determine layout based on number of active views
	uint viewcount = C_debug_view_count;
	uint views_dim = (viewcount > 9 ? 4 : (viewcount > 4 ? 3 : (viewcount > 1 ? 2 : 1)));

	// Determine which view this pixel lies in, and early-exit if this view is not active
	float2 view_size = (C_debug_buffer_size / float2(views_dim, views_dim));
	float tc_viewpc = (1.0f / views_dim);
	float2 view_pc = float2(tc_viewpc, tc_viewpc);

	uint2 view = floor(IN.texCoord / view_pc);
	uint view_id = view.x + (view.y * views_dim);

	if (view_id >= viewcount) return EMPTY;

	// Determine texcoord within the (potentially downsampled) source texture
	float2 view_texcoord;
	if (C_debug_render_mode == DEF_DEBUG_RENDER_COMPOSITE)	// Render one image with subregions sampled from each debug texture
	{
		view_texcoord = IN.texCoord;
	}
	else													// Default: render each segment of the window as a separate debug view
	{
		float2 tc_viewpos = ((float2)view * view_pc);
		float view_scale_factor = (float)views_dim;
		view_texcoord = ((IN.texCoord - tc_viewpos) * view_scale_factor);
	}

	// Render differently based upon the source texture type and active views
	return DebugViewSample(view_id, view_texcoord);
}

