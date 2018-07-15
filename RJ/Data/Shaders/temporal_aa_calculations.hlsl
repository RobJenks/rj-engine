#ifndef __TemporalAACalculationsHLSL__
#define __TemporalAACalculationsHLSL__

float3 ClosestFragmentIn3x3Neighbourhood(float2 uv)
{
	float2 dd = abs(C_texelsize);
	float2 du = float2(dd.x, 0.0);
	float2 dv = float2(0.0, dd.y);

	// TODO: Convert uv [0 1] to [0 texsize] and use .Load() ?
	float3 dtl = float3(-1, -1, GBuffer_DepthTextureVS.Sample(LinearClampSampler, uv - dv - du).x);
	float3 dtc = float3(0, -1, GBuffer_DepthTextureVS.Sample(LinearClampSampler, uv - dv).x);
	float3 dtr = float3(1, -1, GBuffer_DepthTextureVS.Sample(LinearClampSampler, uv - dv + du).x);

	float3 dml = float3(-1, 0, GBuffer_DepthTextureVS.Sample(LinearClampSampler, uv - du).x);
	float3 dmc = float3(0, 0, GBuffer_DepthTextureVS.Sample(LinearClampSampler, uv).x);
	float3 dmr = float3(1, 0, GBuffer_DepthTextureVS.Sample(LinearClampSampler, uv + du).x);

	float3 dbl = float3(-1, 1, GBuffer_DepthTextureVS.Sample(LinearClampSampler, uv + dv - du).x);
	float3 dbc = float3(0, 1, GBuffer_DepthTextureVS.Sample(LinearClampSampler, uv + dv).x);
	float3 dbr = float3(1, 1, GBuffer_DepthTextureVS.Sample(LinearClampSampler, uv + dv + du).x);

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







#endif