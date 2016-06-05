#if !defined(__vertex_definitionsH__)
#define __vertex_definitionsH__

// Enable common usage across C++ and HLSL by making preprocessor adjustments
#ifdef __cplusplus
	#include <DirectXMath.h>

	using float2 = DirectX::XMFLOAT2;
	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMFLOAT4;
#	define RJ_ROW_MAJOR_MATRIX DirectX::XMFLOAT4X4
#	define RJ_SEMANTIC(sem) 

#else

#	define RJ_ROW_MAJOR_MATRIX row_major float4x4
#	define RJ_SEMANTIC(sem) : sem

#endif


// Standard vertex definition used by most shaders
struct Vertex_Inst_TexNormMatLit
{
	// Per-vertex data
	float3					position		RJ_SEMANTIC(POSITION);
	float2					tex				RJ_SEMANTIC(TEXCOORD0);
	float3					normal			RJ_SEMANTIC(NORMAL);
	unsigned int			material		RJ_SEMANTIC(MATERIAL);

	// Per-instance data
	RJ_ROW_MAJOR_MATRIX		mTransform		RJ_SEMANTIC(mTransform);
	float4					iParams			RJ_SEMANTIC(iParams);
	unsigned int			LightConfig		RJ_SEMANTIC(LightConfig);
	float3					padding			RJ_SEMANTIC(padding);
};


#endif



