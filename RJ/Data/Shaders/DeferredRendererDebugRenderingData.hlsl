#ifndef __DeferredRendererDebugRenderingDataHLSL__
#define __DeferredRendererDebugRenderingDataHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"

// Constants
static const unsigned int DEF_DEBUG_MAX_RENDER_VIEWS = 16U;

// State values for each debug texture slot
static const unsigned int DEF_DEBUG_STATE_DISABLED = 0U;
static const unsigned int DEF_DEBUG_STATE_ENABLED_NORMAL = 1U;
static const unsigned int DEF_DEBUG_STATE_ENABLED_DEPTH = 2U;
static const unsigned int DEF_DEBUG_STATE_ENABLED_VELOCITY = 3U;

// Render mode types
static const unsigned int DEF_DEBUG_RENDER_VIEWS = 0U;
static const unsigned int DEF_DEBUG_RENDER_COMPOSITE = 1U;


// Structure holding data on a specific debug render view (must be 16 byte-aligned to avoid CBuffer size varying across CPP & HLSL)
struct DeferredRendererDebugRenderViewData
{
	unsigned int state;
	uint3 __def_debug_view_padding;
};

// Constant buffer used for debug rendering by the deferred renderer
CBUFFER DeferredRendererDebugRenderingData
{
	// Output buffer size
	uint2 C_debug_buffer_size;

	// Count of debug views currently in use
	unsigned int C_debug_view_count;

	// Render mode; either multiple views, or a composite created by all views rendering one UV set
	unsigned int C_debug_render_mode;

	/* Padding to keep CB to 16-byte boundaries */
	//uint# __def_debug_padding;

	// State of each potential debug view; only those != 0 will be rendered
	DeferredRendererDebugRenderViewData C_debug_view[16];
};

#define DeferredRendererDebugRenderingDataName BUFFER_NAME(DeferredRendererDebugRenderingData)



#endif