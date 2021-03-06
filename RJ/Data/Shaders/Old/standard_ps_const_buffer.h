#if !defined(__standard_const_bufferH__)
#define __standard_const_bufferH__

#include "../../LightData.hlsl.h"
#include "../../MaterialData.hlsl.h"
#include "old_render_constants.h"

// Enable common usage across C++ and HLSL by making preprocessor adjustments
#ifdef __cplusplus
	#include <DirectXMath.h>
	using float2 = DirectX::XMFLOAT2;
	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMFLOAT4;
	#define cbuffer struct
#endif

// Define constants used in these definitions
#define			FOG_STATE_DISABLED		0
#define			FOG_STATE_ENABLED		1


// Constant buffer holding all material, lighting and other data for the render pass
cbuffer StandardPSConstBuffer
{
	// Float4
	float3							EyeWorldPos;
	int								FogState;

	// Float4
	float4							FogColour;

	// Float4
	float							FogStart;
	float							FogRange;
	unsigned int					LightCount;
	unsigned int					MaterialCount;

	// Float4
	MaterialData					Materials[C_MATERIAL_LIMIT];		// Each element is guaranteed to have SIZE % 16 == 0

	// Float4
	LightData						Lights[C_LIGHT_LIMIT];				// Each element is guaranteed to have SIZE % 16 == 0

	// Padding only needs to account for the primitive types; all structs have been padded to % 16 == 0 already
};


#endif



