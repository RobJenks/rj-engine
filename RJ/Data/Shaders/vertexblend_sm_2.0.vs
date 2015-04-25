//////////////////////////////////////////////////////////////////
// 
// Vertex blending VS shader for use in rendering skinned models
//
// Desc: Vertex blending vertex shader. Supports meshes with 2-4
// bone influences per vertex. We can dynamically set
// NumVertInfluences so that the shader knows how many
// weights it is processing per vertex. In order to support
// dynamic loops, we must use at least vertex shader
// version 2.0.
//
//////////////////////////////////////////////////////////////////

extern float4x4 WorldViewProj;
extern float4x4 FinalTransforms[35];
extern texture Tex;
extern int NumVertInfluences = 2;//<--- Normally set dynamically.

sampler S0 = sampler_state
{
	Texture = <Tex>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};

struct VS_OUTPUT
{
	float4 pos : POSITION0;
	float2 texCoord : TEXCOORD;
	float4 diffuse : COLOR0;
};

VS_OUTPUT VertexBlend(	float4 pos : POSITION0,
						float2 texCoord : TEXCOORD0,
						float4 weights : BLENDWEIGHT0,
						int4 boneIndices : BLENDINDICES0)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	float4 p = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float lastWeight = 0.0f;
	int n = NumVertInfluences-1;
	
	// Compute a bone-weighted influence for the vertex
	for(int i = 0; i < n; ++i)
	{
		lastWeight += weights[i];
		p += weights[i] * mul(pos, FinalTransforms[boneIndices[i]]);
	}
	
	lastWeight = 1.0f - lastWeight;
	p += lastWeight * mul(pos, FinalTransforms[boneIndices[n]]);
	p.w = 1.0f;
	
	output.pos		= mul(p, WorldViewProj);
	output.texCoord	= texCoord;
	output.diffuse	= float4(1.0f, 1.0f, 1.0f, 1.0f);
	return output;
}

technique VertexBlendingTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 VertexBlend();
		Sampler[0] = <S0>;
		Lighting = false;
	}
}