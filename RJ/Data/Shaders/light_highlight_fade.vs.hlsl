////////////////////////////////////////////////////////////////////////////////
// Filename: light_highlight_fade_sm_all.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix viewMatrix;
	matrix projectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    row_major float4x4 mTransform : mTransform;
    float4 iParams : iParams;		// Params xyz contain highlight colour, w contains alpha
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 highlight_alpha : TEXCOORD1;	// Params xyz contain highlight colour for the object, w contains alpha
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the instanced world matrix, plus constant view and projection matrices.
    output.position = mul(input.position, input.mTransform);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
	// Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)input.mTransform);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    // Pass through the highlight colour and alpha to be applied by the pixel shader
    output.highlight_alpha = input.iParams;

    return output;
}