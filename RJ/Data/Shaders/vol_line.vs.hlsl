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
    float3 P : POSITION;
	row_major float4x4 mTransform : mTransform;		// Row 0 (_11 to _14) is P1, Row 1 (_21 to _24) is P2, Row 2/3 are unused
	float4 iParams : iParams;
};

struct GeomInputType
{
    float4 P0 : POSITION0;
	float4 P1 : POSITION1;
	float4 Colour : COLOUR;
	float Radius : RADIUS;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
GeomInputType main(VertexInputType input)
{
    GeomInputType output;
    
	// Transform the two input points to view space 
	output.P0 = mul(float4(input.mTransform._11, input.mTransform._12, input.mTransform._13, 1.0f), viewMatrix);
	output.P1 = mul(float4(input.mTransform._21, input.mTransform._22, input.mTransform._23, 1.0f), viewMatrix);

	// Add all additional pass-through parameters required further down the pipeline
	output.Colour = input.mTransform._31_32_33_34;
	output.Radius = input.iParams.x;				// Params.x == line radius

    return output;
}