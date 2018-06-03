#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"
#include "SDFDecalRenderingCommonData.hlsl.h"
#include "DeferredRenderingGBuffer.hlsl.h"


float3 ReconstructWorldPosition(float3 viewRay, float depth) 
{
	float3 viewPosition = viewRay * depth;

	//Calculate world position of this pixel
	return mul(float4(viewPosition, 1), InvViewMatrix).xyz;
}

float3 CalculatePixelWorldPosition(float4 screenpos, float4 viewpos, float depth)
{
	// Read depth information from the GBuffer for the fragment
	int2 texCoord = screenpos.xy;
	float sampledDepth = GBuffer_DepthTextureVS.Load(int3(texCoord, 0)).r;

	if (performZTest)
	{
		clip(sampledDepth - depth);		// Reject if result is < 0, i.e. if pixel depth > sampled GBuffer geometry depth
	}

	float3 frustumRay = viewpos.xyz * (FarClipDistance / -viewpos.z);
	return ReconstructWorldPosition(frustumRay, sampledDepth);
}

float4 DiffuseValue(float2 texcoord, float alpha)
{
	float4 diffuse = GBuffer_DiffuseTextureVS.Load(int3(texcoord, 0)) * baseColour;
	diffuse.a *= alpha;

	return diffuse;
}


// PS_SDFDecal_Direct: Renders a decal directly based on a source signed-distance field representation
// Input from http://martindevans.me/game-development/2015/02/27/Drawing-Stuff-On-Other-Stuff-With-Deferred-Screenspace-Decals/
float4 PS_SDFDecal_Deferred(SDFDecalRenderingDeferredVertexShaderOutput IN) : SV_TARGET
{
	float3 worldPosition = CalculatePixelWorldPosition(IN.position, IN.positionVS, IN.depth);

	return DiffuseValue(IN.texCoord, 1.0f);
}