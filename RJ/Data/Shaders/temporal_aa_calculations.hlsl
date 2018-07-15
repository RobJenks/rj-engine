#ifndef __TemporalAACalculationsHLSL__
#define __TemporalAACalculationsHLSL__

#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "temporal_aa_resources.hlsl"

// Constants
static const float FLT_EPS = 0.00000001f;


// Sample the 3x3 neighbourhood and locate the closest sample to the given uv
float3 ClosestFragmentIn3x3Neighbourhood(float2 uv)
{
	float2 dd = abs(C_texelsize);
	float2 du = float2(dd.x, 0.0);
	float2 dv = float2(0.0, dd.y);

	// TODO: Convert uv [0 1] to [0 texsize] and use .Load() ?
	float3 dtl = float3(-1, -1, TAADepthBufferInput.Sample(PointClampSampler, uv - dv - du).x);
	float3 dtc = float3(0, -1, TAADepthBufferInput.Sample(PointClampSampler, uv - dv).x);
	float3 dtr = float3(1, -1, TAADepthBufferInput.Sample(PointClampSampler, uv - dv + du).x);

	float3 dml = float3(-1, 0, TAADepthBufferInput.Sample(PointClampSampler, uv - du).x);
	float3 dmc = float3(0, 0, TAADepthBufferInput.Sample(PointClampSampler, uv).x);
	float3 dmr = float3(1, 0, TAADepthBufferInput.Sample(PointClampSampler, uv + du).x);

	float3 dbl = float3(-1, 1, TAADepthBufferInput.Sample(PointClampSampler, uv + dv - du).x);
	float3 dbc = float3(0, 1, TAADepthBufferInput.Sample(PointClampSampler, uv + dv).x);
	float3 dbr = float3(1, 1, TAADepthBufferInput.Sample(PointClampSampler, uv + dv + du).x);

	float3 dmin = dtl;
	if (dtc.z < dmin.z) dmin = dtc;
	if (dtr.z < dmin.z) dmin = dtr;

	if (dml.z < dmin.z) dmin = dml;
	if (dmc.z < dmin.z) dmin = dmc;
	if (dmr.z < dmin.z) dmin = dmr;

	if (dbl.z < dmin.z) dmin = dbl;
	if (dbc.z < dmin.z) dmin = dbc;
	if (dbr.z < dmin.z) dmin = dbr;

	return float3(uv + dd.xy * dmin.xy, dmin.z);
}

// Convert raw depth buffer values to linear eye depth
float LinearEyeDepth(float rawdepth)
{
	float x, y, z, w;
	x = 1.0 - C_NearClip / C_FarClip;
	y = C_NearClip / C_FarClip;
	z = x / C_NearClip;
	w = y / C_NearClip;

	return 1.0 / (z * rawdepth + w);
}


// Sample from the primary buffer, accounting for colour space
float4 SampleColour(Texture2D tex, float2 uv)
{
#	if USE_YCOCG
		float4 c = tex.Sample(LinearRepearSampler, uv);
		return float4(RGB_YCoCg(c.rgb), c.a);
#	else
		return tex.Sample(LinearRepeatSampler, uv);
#	endif
}

// Resolve colour into relevant colour space
float4 ResolveColour(float4 c)
{
#	if USE_YCOCG
		return float4(YCoCg_RGB(c.rgb).rgb, c.a);
#	else
		return c;
#	endif


// Clip to the colour-space neighbourhood of the current pixel
float4 ColourSpaceClipAABB(float3 aabb_min, float3 aabb_max, float4 p, float4 q)
{
#if FAST_APPROX_COLOUR_SPACE_CLIPPING		

	// Optimised version clips towards AABB centre only, but no noticeable quality impact & much faster
	float3 p_clip = 0.5 * (aabb_max + aabb_min);
	float3 e_clip = 0.5 * (aabb_max - aabb_min) + FLT_EPS;

	float4 v_clip = q - float4(p_clip, p.w);
	float3 v_unit = v_clip.xyz / e_clip;
	float3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0)
	{
		return float4(p_clip, p.w) + v_clip / ma_unit;
	}
	else
	{
		return q;	// Point is inside aabb
	}

#else

	// Slow but exact method
	float4 r = q - p;
	float3 rmax = aabb_max - p.xyz;
	float3 rmin = aabb_min - p.xyz;

	const float eps = FLT_EPS;

	if (r.x > rmax.x + eps)
		r *= (rmax.x / r.x);
	if (r.y > rmax.y + eps)
		r *= (rmax.y / r.y);
	if (r.z > rmax.z + eps)
		r *= (rmax.z / r.z);

	if (r.x < rmin.x - eps)
		r *= (rmin.x / r.x);
	if (r.y < rmin.y - eps)
		r *= (rmin.y / r.y);
	if (r.z < rmin.z - eps)
		r *= (rmin.z / r.z);

	return p + r;

#endif
}






#endif