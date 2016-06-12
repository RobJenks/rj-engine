#include "light_definition.h"
#include "material_definition.h"

// Import a standard constant buffer holding data on materials, lighting etc
#include "standard_ps_const_buffer.h"


//---------------------------------------------------------------------------------------
// Performs shadowmap test to determine if a pixel is in shadow.
//---------------------------------------------------------------------------------------

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow, 
                       Texture2D shadowMap, 
					   float4 shadowPosH)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;
	
	// Depth in NDC space.
	float depth = shadowPosH.z;

	// Texel size.
	const float dx = SMAP_DX;

	float percentLit = 0.0f;
	const float2 offsets[9] = 
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for(int i = 0; i < 9; ++i)
	{
		percentLit += shadowMap.SampleCmpLevelZero(samShadow, 
			shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit /= 9.0f;
}


//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.  
//---------------------------------------------------------------------------------------
void ComputeDirectionalLight(MaterialData mat, LightData L, 
                             float3 normal, float3 toEye,
					         out float4 ambient,
						     out float4 diffuse,
						     out float4 spec)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -L.Direction;

	// Add ambient term.
	ambient = mat.Ambient * L.AmbientIntensity;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	
	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.DiffuseIntensity;
		spec    = specFactor * mat.Specular * L.SpecularPower;
	}
}


//---------------------------------------------------------------------------------------
// Transforms a normal map sample to world space.
//---------------------------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float4 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW.xyz - dot(tangentW.xyz, N)*N);
	float3 B = tangentW.w*cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

cbuffer cbPerSubset
{
	MaterialData Material;
}; 

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap : register(t0);
Texture2D gNormalMap : register(t1);
//Texture2D gShadowMap;
//Texture2D gSsaoMap;
//TextureCube gCubeMap;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerComparisonState samShadow
{
	Filter   = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    ComparisonFunc = LESS_EQUAL;
};

struct VertexOut
{
	float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
	float4 TangentW   : TANGENT;
	float2 Tex        : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
	float4 SsaoPosH   : TEXCOORD2;
};
 
float4 main(VertexOut pin, 
          uniform int _gLightCount, 
		  uniform bool _gUseTexure, 
		  uniform bool _gAlphaClip, 
		  uniform bool _gFogEnabled, 
		  uniform bool _gReflectionEnabled) : SV_Target
{
int gLightCount = 3;
bool gUseTexure = true;
bool gAlphaClip = false;
bool gFogEnabled = false;
bool gReflectionEnabled = false;

	// Interpolating normal can unnormalize it, so normalize it.
	pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = EyeWorldPos - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
	
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample( samLinear, pin.Tex );

		if(gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.1.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
			clip(texColor.a - 0.1f);
		}
	}

	//
	// Normal mapping
	//

	float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);
	 
	//
	// Lighting.
	//

	float4 litColor = texColor;
	
	// Only the first light casts a shadow.
	float3 shadow = float3(1.0f, 1.0f, 1.0f);
	shadow[0] = 0.5f; //CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

	// Finish texture projection and sample SSAO map.
	pin.SsaoPosH /= pin.SsaoPosH.w;
	float ambientAccess = 1.0f; //gSsaoMap.Sample(samLinear, pin.SsaoPosH.xy, 0.0f).r;
		
	// First calculate light contribution from the global directional light (which sets initial values for each light property)
	float4 ambient, diffuse, spec;
	ComputeDirectionalLight(Material, Lights[0], bumpedNormalW, toEye, ambient, diffuse, spec);	// ***** TODO ****** <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	ambient *= ambientAccess;
	diffuse *= shadow[0];
	spec *= shadow[0];

	// Now calculate and add the light contribution from each point light source.  
	//[unroll] 
	for(unsigned int i = 0; i < LightCount; ++i)
	{
		float4 A, D, S;
		/*ComputeDirectionalLight(gMaterial, gDirLights[i], bumpedNormalW, toEye,			// TODO: EXTEND TO THE POINT LIGHTS
			A, D, S);

		ambient += ambientAccess*A;    
		diffuse += shadow[i]*D;
		spec    += shadow[i]*S;*/
	}
		   
	litColor = texColor*(ambient + diffuse) + spec;

	//if( gReflectionEnabled )
	//{
	//	float3 incident = -toEye;
	//	float3 reflectionVector = reflect(incident, bumpedNormalW);
	//	float4 reflectionColor  = gCubeMap.Sample(samLinear, reflectionVector);
	//
	//	litColor += gMaterial.Reflect*reflectionColor;
	//}

 
	//
	// Fogging
	//

	if( FogState == FOG_STATE_ENABLED )
	{
		float fogLerp = saturate( (distToEye - FogStart) / FogRange ); 

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, FogColour, fogLerp);
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = Material.Diffuse.a * texColor.a;

    return litColor;
}

