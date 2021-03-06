////////////////////////////////////////////////////////////////////////////////
// Filename: texture.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
TextureCube shaderTexture;
SamplerState SampleType;


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 inputposition : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
{
	// Determine the vector direction to this pixel
	float3 direction = normalize(input.inputposition.xyz);
	
	// Return the pixel colour in this vector direction, using the cube map & sampler
	return shaderTexture.Sample(SampleType, direction);
}
