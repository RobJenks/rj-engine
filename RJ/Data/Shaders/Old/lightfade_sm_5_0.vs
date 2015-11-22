////////////////////////////////////////////////////////////////////////////////
// Filename: light.vs
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
    float4 iParams : iParams;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 tex_alpha : TEXCOORD0;		// Share semantic for efficiency.  xy = tex, z = alpha
    float3 normal : NORMAL;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightFadeVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the instanced world matrix, plus constant view and projection matrices.
    output.position = mul(input.position, input.mTransform);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader (in xy).  Also pass alpha in this field (in z)
	output.tex_alpha = float3(input.tex, input.iParams.x);
    
	// Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)input.mTransform);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    return output;
}