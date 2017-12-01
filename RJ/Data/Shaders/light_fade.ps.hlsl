// Globals
Texture2D shaderTexture;
SamplerState SampleType;

// Import all light structures and definitions from the external definition file

#include "LightData.hlsl.h"
#include "vertex_definitions.h.hlsl"

// Import a standard constant buffer holding data on materials, lighting etc
#include "standard_ps_const_buffer.h"

// Import key lighting calcuations
#include "old_light_calculation.h.hlsl"


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 tex_alpha : TEXCOORD0;		// Share semantic for efficiency.  xy = tex, z = alpha
	float3 worldpos : TEXCOORD1;		// Position in world space (interpolated)
	float3 normal : NORMAL;				// Un-normalised; will be interpolated so is normalised in the PS
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

	// Apply the effect of each light in turn
	unsigned int bit;
	for (unsigned int i = 0U; i < LightCount; ++i)
	{
		bit = CONFIG_VAL[i];
		if ((input.LightConfig & bit) == bit)
		{
			TotalLight += CalculateLight(i, input.material, input.worldpos, normal);
		}
	}

	// Final pixel colour will be the sampled texture colour illuminated by this total volume of light
	float4 colour = shaderTexture.Sample(SampleType, input.tex_alpha.xy) * TotalLight;

	// Set the desired alpha value and return the result
	colour.a = input.tex_alpha.z;
	return colour;
}
