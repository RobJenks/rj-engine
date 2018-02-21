#include "../../../Definitions/LightData.hlsl.h"
#include "../../../Definitions/MaterialData.hlsl.h"


// Common result type for all lighting calculations
struct LightingResult
{
	float4 Diffuse;
	float4 Specular;
};


// Determine diffuse component
float4 CalculateDiffuse(LightData light, float4 L, float4 N)
{
	float NdotL = max(dot(N, L), 0);
	return light.Colour * NdotL;
}

// Determine specular component
float4 CalculateSpecular(LightData light, MaterialData material, float4 V, float4 L, float4 N)
{
	float4 R = normalize(reflect(-L, N));
	float RdotV = max(dot(R, V), 0);

	return light.Colour * pow(RdotV, material.SpecularPower);
}

// Compute attenuation based on the range of the light
float CalculateAttenuation(LightData light, float d)
{
	return 1.0f - smoothstep(light.Range * 0.75f, light.Range, d);
}

// Calculate extent of the spotlight cone and influence on the current fragment
float CalculateSpotCone(LightData light, float4 L)
{
	// If the cosine angle of the light's direction 
	// vector and the vector from the light source to the point being 
	// shaded is less than minCos, then the spotlight contribution will be 0
	// TODO: consider including CosAngle in light again to avoid cos() per fragment
	float minCos = cos(light.SpotlightAngle);

	// If the cosine angle of the light's direction vector
	// and the vector from the light source to the point being shaded
	// is greater than maxCos, then the spotlight contribution will be 1
	float maxCos = lerp(minCos, 1, 0.5f);

	// Blend between the maxixmum and minimum cosine angles
	float cosAngle = dot(light.DirectionVS, -L);
	return smoothstep(minCos, maxCos, cosAngle);
}





// Perform lighting calculations for an infinitesimal point light
LightingResult CalculatePointLight(LightData light, MaterialData mat, float4 V, float4 P, float4 N)
{
	LightingResult result;

	float4 L = light.PositionVS - P;
	float distance = length(L);
	L = L / distance;

	float attenuation = CalculateAttenuation(light, distance);

	result.Diffuse = CalculateDiffuse(light, L, N) * attenuation * light.Intensity;
	result.Specular = CalculateSpecular(light, mat, V, L, N) * attenuation * light.Intensity;

	return result;
}

// Perform lighting calculations for a directional light at infinite distance from the target
LightingResult CalculateDirectionalLight(LightData light, MaterialData mat, float4 V, float4 P, float4 N)
{
	LightingResult result;

	float4 L = normalize(-light.DirectionVS);

	result.Diffuse = CalculateDiffuse(light, L, N) * light.Intensity;
	result.Specular = CalculateSpecular(light, mat, V, L, N) * light.Intensity;

	return result;
}

// Perform lighting calculations for a spotlight with precalculated angles and falloff
LightingResult CalculateSpotLight(LightData light, MaterialData mat, float4 V, float4 P, float4 N)
{
	LightingResult result;

	float4 L = light.PositionVS - P;
	float distance = length(L);
	L = L / distance;

	float attenuation = CalculateAttenuation(light, distance);
	float spotIntensity = CalculateSpotCone(light, L);

	result.Diffuse = CalculateDiffuse(light, L, N) * attenuation * spotIntensity * light.Intensity;
	result.Specular = CalculateSpecular(light, mat, V, L, N) * attenuation * spotIntensity * light.Intensity;

	return result;
}



