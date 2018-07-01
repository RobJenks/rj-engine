#include "../../CommonShaderPipelineStructures.hlsl.h"


// Motion blur neighbourhood determination: entry point
float4 PS_MotionBlur_Neighbourhood(VertexShaderStandardOutput IN) : SV_Target0
{
	return float4(0, 0, 0, 0);
}