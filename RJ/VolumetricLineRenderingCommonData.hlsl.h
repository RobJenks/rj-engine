#ifndef __VolumetricLineRenderingCommonDataH__
#define __VolumetricLineRenderingCommonDataH__

#include "../Definitions/CppHLSLLocalisation.hlsl.h"
#include "../Definitions/VertexDefinitions.hlsl.h"
#include "CommonShaderBufferDefinitions.hlsl.h"


// Basic constant buffer holding minimum per-frame data required
CBUFFER VolumetricLineRenderingFrameBuffer REGISTER(b4)
{
	RJ_ROW_MAJOR_MATRIX C_ViewMatrix;
	RJ_ROW_MAJOR_MATRIX C_ProjMatrix;

	float C_NearClipDistance;
	float C_FarClipDistance;
	float2 C_ViewportSize;
};

struct VLVertexInputType
{
	float3 position RJ_SEMANTIC(POSITION);
	
	PerInstanceData
};

struct VLGeomInputType
{
	float4 P0 RJ_SEMANTIC(POSITION0);
	float4 P1 RJ_SEMANTIC(POSITION1);
	float4 Colour RJ_SEMANTIC(COLOUR);
	float Radius RJ_SEMANTIC(RADIUS);
};

struct VLPixelInputType
{
	float4 position RJ_SEMANTIC(SV_POSITION);		// Position (in projection space)
	float3 p1 RJ_SEMANTIC(P1);						// P1 in view space
	float linelength RJ_SEMANTIC(LENGTH);			// Length between P1 and P2
	float3 p2 RJ_SEMANTIC(P2);						// P2 in view space
	float radius RJ_SEMANTIC(RADIUS);				// Radius of the line
	float radius_sq RJ_SEMANTIC(RADIUS_SQ);			// Squared radius of the line
	float3 view_pos RJ_SEMANTIC(VIEWPOS);			// Position in view space
	float3 line_viewpos RJ_SEMANTIC(LINE_VIEWPOS);	// View position in line space
	float3 line_viewdir RJ_SEMANTIC(LINE_VIEWDIR);	// View direction in line space
	float4 colour RJ_SEMANTIC(COLOUR);				// Colour (including alpha) of the line
};

// Buffer resources bound to the PS
TEXTURE2D VLDepthBufferInput REGISTER(t2);

// Resource names
#define VolumetricLineRenderingFrameBufferName BUFFER_NAME(VolumetricLineRenderingFrameBuffer)
#define VLDepthBufferInputName BUFFER_NAME(VLDepthBufferInput)




#endif