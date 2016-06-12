// Globals
Texture2D shaderTexture;
SamplerState SampleType;

// Import all light structures and definitions from the external definition file
#include "render_constants.h"
#include "light_definition.h"
#include "vertex_definitions.h.hlsl"

// Import a standard constant buffer holding data on materials, lighting etc
#include "standard_ps_const_buffer.h"

// Import key lighting calcuations
#include "light_calculation.h.hlsl"


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 worldpos : TEXCOORD1;				// Position in world space (interpolated)
	float3 normal : NORMAL;
	float4 highlight_alpha : TEXCOORD2;			// Highlight colour and alpha for the object
	unsigned int material : MATERIAL;
	unsigned int LightConfig : LightConfig;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	float4 TotalLight = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Normalise the input normal.  This is passed un-normalised since interpolation at the 
	// PS means it needs to be normalised here
	float3 normal = normalize(input.normal);

	// Apply global directional lights first
	for (unsigned int i = 0U; i < DirLightCount; ++i)
	{
		TotalLight += CalculateDirectionalLight(i, input.material, input.worldpos, normal);
	}

	// Now apply the effect of any active point lights
	unsigned int bit;
	for (unsigned int j = 0U; j < LightCount; ++j)
	{
		bit = CONFIG_VAL[j];
		if ((input.LightConfig & bit) == bit)
		{
			// Index into the light array is (DIR_LIGHT_LIMIT + i), since light data is packed as [Dir_1, ..., Dir_n, Pt_1, ..., Pt_n]
			TotalLight += CalculatePointLight(C_DIR_LIGHT_LIMIT + j, input.material, input.worldpos, normal);
		}
	}

	// Determine the average color intensity across all components, then use the highlight colour weighted by this value
	float intensity = (TotalLight.r + TotalLight.g + TotalLight.b) * 0.3333f;
	TotalLight.rgb = input.highlight_alpha.xyz * intensity;

	// Saturate the final light colour and set desired alpha value
	TotalLight = saturate(TotalLight);
	TotalLight.a = input.highlight_alpha.a;

	// Multiply the texture pixel and the final diffuse color to get the final pixel color result.
	return (shaderTexture.Sample(SampleType, input.tex) * TotalLight);
}
