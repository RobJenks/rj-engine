// Globals
Texture2D shaderTexture;
SamplerState SampleType;

// Import all light structures and definitions from the external definition file
#include "render_constants.h"
#include "light_definition.h"
#include "vertex_definitions.h.hlsl"

// Import a standard constant buffer holding data on materials, lighting etc
#include "standard_ps_const_buffer.h"


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 tex_alpha : TEXCOORD0;		// Share semantic for efficiency.  xy = tex, z = alpha
    float3 normal : TEXCOORD1;
	unsigned int material : MATERIAL;
	unsigned int LightConfig : LightConfig;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
   /* float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;


    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = shaderTexture.Sample(SampleType, input.tex_alpha.xy);

    // Set the default ambient colour to the ambient colour, for all pixels.  We apply all other effects on top of the ambient colour
    color = DirLight.Ambient;

    // Invert the light direction for calculations.
    lightDir = -DirLight.Direction;

    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.normal, lightDir));

    // Ensure N dot L is greater than zero, otherwise -ve diffuse light would subtract from the ambient light
    if (lightIntensity > 0.0f)
    {
        // Determine the final diffuse colour based on the diffuse colour and level of light intensity
        color += (DirLight.Diffuse * lightIntensity);
    }

    // Saturate the final light colour
    color = saturate(color);

    // Multiply the texture pixel and the final diffuse color to get the final pixel color result.
    color = color * textureColor;

    // Apply the desired alpha value to this colour
    color.a = input.tex_alpha.z;

    return color;*/
	return float4(0, 0, 0, 0);
}
