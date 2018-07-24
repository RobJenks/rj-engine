#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/MaterialData.hlsl.h"
#include "velocity_calculations.hlsl"

// Forward declarations
float4 DoNormalMapping(float3x3 TBN, Texture2D tex, sampler s, float2 uv);
float4 DoBumpMapping(float3x3 TBN, Texture2D tex, sampler s, float2 uv, float bumpScale);
float3 ExpandNormal(float3 n);


// Pixel shader that generates the G-Buffer
[earlydepthstencil]
DeferredPixelShaderGeometryOutput PS_Deferred_Geometry(VertexShaderStandardOutput IN)
{
	DeferredPixelShaderGeometryOutput OUT;

	float4 diffuse = Mat.DiffuseColor;
	if (Mat.HasDiffuseTexture)
	{
		float4 diffuseTex = DiffuseTexture.Sample(LinearRepeatSampler, IN.texCoord);
		if (any(diffuse.rgb))
		{
			diffuse *= diffuseTex;
		}
		else
		{
			diffuse = diffuseTex;
		}
	}

	// By default, use the alpha from the diffuse component.
	float alpha = diffuse.a;
	if (Mat.HasOpacityTexture)
	{
		// If the material has an opacity texture, use that to override the diffuse alpha.
		alpha = OpacityTexture.Sample(LinearRepeatSampler, IN.texCoord).r;
	}

	if (alpha * Mat.Opacity < Mat.AlphaThreshold)
	{
		discard;
	}

	OUT.Diffuse = diffuse;

	float4 ambient = Mat.AmbientColor;
	if (Mat.HasAmbientTexture)
	{
		float4 ambientTex = AmbientTexture.Sample(LinearRepeatSampler, IN.texCoord);
		if (any(ambient.rgb))
		{
			ambient *= ambientTex;
		}
		else
		{
			ambient = ambientTex;
		}
	}

	// Combine the global ambient term.
	ambient *= Mat.GlobalAmbient;

	float4 emissive = Mat.EmissiveColor;
	if (Mat.HasEmissiveTexture)
	{
		float4 emissiveTex = EmissiveTexture.Sample(LinearRepeatSampler, IN.texCoord);
		if (any(emissive.rgb))
		{
			emissive *= emissiveTex;
		}
		else
		{
			emissive = emissiveTex;
		}
	}

	// TODO: Also compute directional lighting in the LightAccumulation buffer.
	OUT.LightAccumulation = (ambient + emissive);

	// Normal mapping
	float4 N;
	if (Mat.HasNormalTexture)
	{
		// For models with normal mapping, no need to invert the binormal.
		float3x3 TBN = float3x3(normalize(IN.tangentVS),
			normalize(IN.binormalVS),
			normalize(IN.normalVS));

		N = DoNormalMapping(TBN, NormalTexture, LinearRepeatSampler, IN.texCoord);
	}
	// Bump mapping
	else if (Mat.HasBumpTexture)
	{
		// For most models using bump mapping, we have to invert the binormal.
		float3x3 TBN = float3x3(normalize(IN.tangentVS),
			normalize(-IN.binormalVS),
			normalize(IN.normalVS));

		N = DoBumpMapping(TBN, BumpTexture, LinearRepeatSampler, IN.texCoord, Mat.BumpIntensity);
	}
	// Just use the normal from the model
	else
	{
		N = normalize(float4(IN.normalVS, 0));	// TODO: Can guarantee that all normals are already pre-normalised via model pipeline?
	}

	OUT.NormalVS = N;

	float specularPower = Mat.SpecularPower;
	if (Mat.HasSpecularPowerTexture)
	{
		specularPower = SpecularPowerTexture.Sample(LinearRepeatSampler, IN.texCoord).r * Mat.SpecularScale;
	}

	float4 specular = 0;
	if (specularPower > 1.0f) // If specular power is too low, don't use it.
	{
		specular = Mat.SpecularColor;
		if (Mat.HasSpecularTexture)
		{
			float4 specularTex = SpecularTexture.Sample(LinearRepeatSampler, IN.texCoord);
			if (any(specular.rgb))
			{
				specular *= specularTex;
			}
			else
			{
				specular = specularTex;
			}
		}
	}

	// Method of packing specular power from "Deferred Rendering in Killzone 2" presentation 
	// from Michiel van der Leeuw, Guerrilla (2007)
	OUT.Specular = float4(specular.rgb, log2(specularPower) / 10.5f);

	// Calculate per-pixel velocity vector based on current and prior frame object geometry (with frustum-projected jitter component removed)
	OUT.VelocitySS = CalculateScreenSpacePixelVelocity(IN.lastframeposition_unjittered, IN.thisframeposition_unjittered);

	return OUT;
}

float4 DoNormalMapping(float3x3 TBN, Texture2D tex, sampler s, float2 uv)
{
	float3 normal = tex.Sample(s, uv).xyz;
	normal = ExpandNormal(normal);

	// Transform normal from tangent space to view space.
	normal = mul(normal, TBN);
	return normalize(float4(normal, 0));
}

float4 DoBumpMapping(float3x3 TBN, Texture2D tex, sampler s, float2 uv, float bumpScale)
{
	// Sample the heightmap at the current texture coordinate.
	float height_00 = tex.Sample(s, uv).r * bumpScale;
	// Sample the heightmap in the U texture coordinate direction.
	float height_10 = tex.Sample(s, uv, int2(1, 0)).r * bumpScale;
	// Sample the heightmap in the V texture coordinate direction.
	float height_01 = tex.Sample(s, uv, int2(0, 1)).r * bumpScale;

	float3 p_00 = { 0, 0, height_00 };
	float3 p_10 = { 1, 0, height_10 };
	float3 p_01 = { 0, 1, height_01 };

	// normal = tangent x bitangent
	float3 normal = cross(normalize(p_10 - p_00), normalize(p_01 - p_00));

	// Transform normal from tangent space to view space.
	normal = mul(normal, TBN);

	return float4(normal, 0);
}

float3 ExpandNormal(float3 n)
{
	return n * 2.0f - 1.0f;
}