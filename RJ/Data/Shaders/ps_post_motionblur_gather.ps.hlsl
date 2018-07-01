#include "../../CommonShaderPipelineStructures.hlsl.h"


// Motion blur gather phase: entry point
float4 PS_MotionBlur_Gather(VertexShaderStandardOutput IN) : SV_Target0
{
	return float4(0, 0, 0, 0);
}