////////////////////////////////////////////////////////////////////////////////
// Filename: light_highlight_sm_all.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
	float padding;
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 highlight : TEXCOORD1;	// Highlight colour for the object
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


    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    // Set the default ambient colour to the ambient colour, for all pixels.  We apply all other effects on top of the ambient colour
    color = ambientColor;

    // Invert the light direction for calculations.
    lightDir = -lightDirection;

    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.normal, lightDir));

    // Ensure N dot L is greater than zero, otherwise -ve diffuse light would subtract from the ambient light
    if (lightIntensity > 0.0f)
    {
        // Determine the final diffuse colour based on the diffuse colour and level of light intensity
        color += (diffuseColor * lightIntensity);
    }

    // Determine the average color intensity across all components, then use the highlight colour weighted by this value
    float intensity = (color.r + color.g + color.b) * 0.3333f;
    color.rgb = input.highlight * intensity;


    // Saturate the final light colour
    color = saturate(color);

    // Multiply the texture pixel and the final diffuse color to get the final pixel color result.
    color = color * textureColor;



    return color;
}
