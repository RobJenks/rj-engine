// Globals
Texture2D shaderTexture;
SamplerState SampleType;

// Import all light structures and definitions from the external definition file
#include "render_constants.h"
#include "light_definition.h"
#include "vertex_definitions.h.hlsl"

// Import a standard constant buffer holding data on materials, lighting etc
#include "standard_ps_const_buffer.h"

// Pixel input format
struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	unsigned int material : MATERIAL;
	unsigned int LightConfig : LightConfig;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;
//TODO30
	float4 amb = float4(0.25f, 0.25f, 0.25f, 1.0f);
	float4 dif = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 spc = float4(0.5f, 0.5f, 0.5f, 0.5f);
	float3 dr = float3(0.0f, 0.0f, 1.0f);

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    // Set the default ambient colour to the ambient colour, for all pixels.  We apply all other effects on top of the ambient colour
	color = DirLight.Ambient; //color = amb;

    // Invert the light direction for calculations.
	lightDir = -dr;// DirLight.Direction;

    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.normal, lightDir));

    // Ensure N dot L is greater than zero, otherwise -ve diffuse light would subtract from the ambient light
    if (lightIntensity > 0.0f)
    {
        // Determine the final diffuse colour based on the diffuse colour and level of light intensity
		color += (dif/*DirLight.Diffuse*/ * lightIntensity);
    }


    // Saturate the final light colour
    color = saturate(color);

    // Multiply the texture pixel and the final diffuse color to get the final pixel color result.
    color = color * textureColor;



    return color;
}
