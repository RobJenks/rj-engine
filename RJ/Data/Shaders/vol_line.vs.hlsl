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
    float4 P : POSITION;
	row_major float4x4 mTransform : mTransform;		// Row 0 (_11 to _14) is P1, Row 1 (_21 to _24) is P2, Row 2/3 are unused
	float4 iParams : iParams;
};

struct GeomInputType
{
    float4 P : POSITION;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
GeomInputType main(VertexInputType input)
{
    GeomInputType output;
    
	// P1 vertex will hold all zeroes; P2 vertex will hold all ones.  Copy relevant instance position 
	// and set w component to 1.0f for proper matrix calculation
	input.P =	(input.P.x < 0.5f ? 
					float4(input.mTransform._11, input.mTransform._12, input.mTransform._13, 1.0f)
				:
					float4(input.mTransform._21, input.mTransform._22, input.mTransform._23, 1.0f)
				);

    // Return the position of the vertex and its forward vector in view space
    output.P = mul(input.P, viewMatrix);    

    return output;
}