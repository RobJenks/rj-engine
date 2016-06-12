// Include common data
#include "vertex_definitions.h.hlsl"


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 tex_alpha : TEXCOORD0;		// Share semantic for efficiency.  xy = tex, z = alpha
	float3 worldpos : TEXCOORD1;		// Position in world space (interpolated)
	float3 normal : NORMAL;				// Un-normalised; will be interpolated so is normalised in the PS
	unsigned int material : MATERIAL;
	unsigned int LightConfig : LightConfig;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType main(Vertex_Inst_TexNormMatLit input)
{
	PixelInputType output;

	// Calculate the position of the vertex against the instanced world matrix, plus constant view and projection matrices.
	// Extend the position vector to be 4 units with w == 1.0f for proper matrix calculations
	float4 worldpos = mul(float4(input.position, 1.0f), input.mTransform);
	output.worldpos = float3(worldpos.x, worldpos.y, worldpos.z);
	output.position = mul(worldpos, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader (in xy).  Also pass alpha in this field (in z)
	output.tex_alpha = float3(input.tex, input.iParams.x);
    
	// Calculate the normal vector against the world matrix only.  Leave un-normalised; will 
	// be interpolated by the PS so will be normalised there instead
	output.normal = mul(input.normal, (float3x3)input.mTransform);

	// Fields passed straight through to the pixel shader
	output.material = input.material;
	output.LightConfig = input.LightConfig;

	return output;
}