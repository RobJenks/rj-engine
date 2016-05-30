#if !defined(__vertex_definitionsH__)
#define __vertex_definitionsH__


// Standard vertex definition used by most shaders
struct Vertex_Inst_TexNormMatLit
{
	// Per-vertex data
	float3					position : POSITION;
	float2					tex : TEXCOORD0;
	float3					normal : NORMAL;
	unsigned int			material : MATERIAL;

	// Per-instance data
	row_major float4x4		mTransform : mTransform;
	float4					iParams : iParams;
	unsigned int			LightConfig : LightConfig;
	float3					padding : padding;
};


#endif



