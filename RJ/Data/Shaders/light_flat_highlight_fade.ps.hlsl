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
	float4 highlight_alpha : TEXCOORD2;			// Highlight colour and alpha value for the object
	unsigned int material : MATERIAL;
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

	// Determine the average color intensity across all components, then use the highlight colour weighted by this value
	// Also set the alpha value manually to override any sampled data
	float4 colour = shaderTexture.Sample(SampleType, input.tex);
	float intensity = (colour.r + colour.g + colour.b) * 0.3333f;
	colour = float4( saturate(input.highlight_alpha.xyz * intensity), input.highlight_alpha.a );

	// Return the final calculated colour
	return colour;
}
