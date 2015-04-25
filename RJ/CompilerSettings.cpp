#include "CompilerSettings.h"

// Define different feature sets for high or low DX feature-level rendering
#ifdef  REDUCE_DX_FEATURE_LEVEL

	const D3D_FEATURE_LEVEL			CMP_D3D_FEATURE_LEVEL				= D3D_FEATURE_LEVEL_9_1;
	const char *					CMP_VERTEX_SHADER_LEVEL				= "vs_2_0";
	const char *					CMP_PIXEL_SHADER_LEVEL				= "vs_2_0";

#else
	
	const D3D_FEATURE_LEVEL			CMP_D3D_FEATURE_LEVEL				= D3D_FEATURE_LEVEL_11_0;
	const char *					CMP_VERTEX_SHADER_LEVEL				= "vs_5_0";
	const char *					CMP_PIXEL_SHADER_LEVEL				= "ps_5_0";

#endif

