////////////////////////////////////////////////////////////////////////////////
// Filename: vol_line.vs.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix viewMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 P1 : POSITION0;
    float4 P2 : POSITION1;
};

struct GeomInputType
{
    float4 P1 : POSITION0;
    float4 P2 : POSITION1;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
GeomInputType main(VertexInputType input)
{
    GeomInputType output;
    
    // Change the input vectors to be 4 units for proper matrix calculations.
    input.P1.w = 1.0f;
	input.P2.w = 1.0f;

    // Return the position of the vertex and its forward vector in view space
    output.P1 = mul(input.P1, viewMatrix);
    output.P2 = mul(input.P2, viewMatrix);
    

    return output;
}