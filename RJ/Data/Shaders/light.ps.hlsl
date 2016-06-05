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

// Pixel input format
struct PixelInputType
{
	float4 position : SV_POSITION;				// Position projected into screen space
	float2 tex : TEXCOORD0;
	float3 worldpos : TEXCOORD1;				// Position in world space (interpolated)
	float3 normal : NORMAL;						// Un-normalised; will be interpolated so is normalised in the PS
	unsigned int material : MATERIAL;
	unsigned int LightConfig : LightConfig;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	// Normalise the input normal.  This is passed un-normalised since interpolation at the 
	// PS means it needs to be normalised here
	float3 normal = normalize(input.normal);

    // Apply global directional light first
	float4 TotalLight = CalculateDirectionalLight(DirLight, input.material, input.worldpos, normal);

	// Now apply the effect of any active point lights
	unsigned int bit;
	for (unsigned int i = 0; i < LightCount; ++i)
	{
		bit = CONFIG_VAL[i];
		if ((input.LightConfig & bit) == bit)
		{
			TotalLight += CalculatePointLight(i, input.material, input.worldpos, normal);
		}
	}

	// Final pixel colour will be the sampled texture colour illuminated by this total volume of light
	return (shaderTexture.Sample(SampleType, input.tex) * TotalLight);
}
