#if !defined(__VertexDefinitionsHLSLH__)
#define __VertexDefinitionsHLSLH__

// Common type definitions
using RM_SortKey = uint64_t;

// Enable common usage across C++ and HLSL by making preprocessor adjustments
#ifdef __cplusplus
#include <DirectXMath.h>
#include "../RJ/DefaultingFloat4.h"

using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;
typedef DefaultingFloat4<DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::NO_DEFAULT, DefaultComponent::USE_DEFAULT> Float4DefaultZeroW;

#	define RJ_ROW_MAJOR_MATRIX DirectX::XMFLOAT4X4
#	define RJ_SEMANTIC(sem) 

#else

typedef float4 Float4DefaultZeroW;

#	define RJ_ROW_MAJOR_MATRIX row_major float4x4
#	define RJ_SEMANTIC(sem) : sem

#endif




// 
// Components of standard vertex definition
// 

#define PerVertexData \
\
	float3					position		RJ_SEMANTIC(POSITION); \
	float3					normal			RJ_SEMANTIC(NORMAL); \
	float3					tangent			RJ_SEMANTIC(TANGENT); \
	float3					binormal		RJ_SEMANTIC(BINORMAL); \
	float2					tex				RJ_SEMANTIC(TEXCOORD0); 


#define PerInstanceData \
\
	RJ_ROW_MAJOR_MATRIX		Transform		RJ_SEMANTIC(Transform); \
	/*--------------------------------------------------------------------- ( 64 bytes ) */ \
	UINT32					Flags			RJ_SEMANTIC(Flags);									/* Flags defined in Definitions/InstanceProperties.hlsl.h */ \
	RM_SortKey				SortKey			RJ_SEMANTIC(SortKey); \
	float					_padding1		RJ_SEMANTIC(padding1); \
	/*--------------------------------------------------------------------- ( 16 bytes ) */ \
	Float4DefaultZeroW		Highlight		RJ_SEMANTIC(Highlight);								/* xyz = colour, w = [0.0 1.0] intensity of highlight.  w=0.0 = default = deactivated */
	/*--------------------------------------------------------------------- ( 16 bytes ) */ \
	/*--------------------------------------------------------------------- ( 16 * 6 = 96 bytes ) */ 



// 
// Standard vertex definition
// 

struct Vertex_Standard
{
	PerVertexData
};

struct Vertex_Inst_Standard
{
	PerVertexData
	PerInstanceData
};


#endif



