#include "../../VolumetricLineRenderingCommonData.hlsl.h"
#include "../../../Definitions/VertexDefinitions.hlsl.h"

// Input instance transform matrix: Row 0 (_11 to _14) is P1, Row 1 (_21 to _24) is P2, Row 2 (_31 to 34) is colour/alpha, row 3 is unused

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
VLGeomInputType VS_Vol_Line(Vertex_Inst_Standard input)
{
	VLGeomInputType output;
    
	// Transform the two input points to view space 
	output.P0 = mul(float4(input.Transform._11, input.Transform._12, input.Transform._13, 1.0f), C_ViewMatrix);
	output.P1 = mul(float4(input.Transform._21, input.Transform._22, input.Transform._23, 1.0f), C_ViewMatrix);

	// Add all additional pass-through parameters required further down the pipeline
	output.Colour = input.Transform._31_32_33_34;
	output.Radius = input.Highlight.x;				// Params.x == line radius

    return output;
}