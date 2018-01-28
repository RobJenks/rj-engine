#ifndef __VertexDefinitionsHLSLH__
#define __VertexDefinitionsHLSLH__

#include "../Definitions/CppHLSLLocalisation.hlsl.h"


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
	_uint32					Flags			RJ_SEMANTIC(Flags);									/* Flags defined in Definitions/InstanceProperties.hlsl.h */ \
	RM_SortKey				SortKey			RJ_SEMANTIC(SortKey); \
	float2					_padding		RJ_SEMANTIC(padding); \
	/*--------------------------------------------------------------------- ( 16 bytes ) */ \
	Float4DefaultZeroW		Highlight		RJ_SEMANTIC(Highlight);								/* xyz = colour, w = [0.0 1.0] intensity of highlight.  w=0.0 = default = deactivated */ \
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



