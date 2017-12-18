
// Output of the standard vertex shader
struct VertexShaderStandardOutput
{
	float3 positionVS : TEXCOORD0;		// View space position
	float2 texCoord : TEXCOORD1;		// Texture coordinate
	float3 tangentVS : TANGENT;			// View space tangent
	float3 binormalVS : BINORMAL;		// View space binormal
	float3 normalVS : NORMAL;			// View space normal
	float4 position : SV_POSITION;		// Clip space position
};


// Output of the deferred geometry PS; mapping into the textures comprising the GBuffer
struct DeferredPixelShaderGeometryOutput
{
	float4 LightAccumulation    : SV_Target0;   // Ambient + emissive (R8G8B8_ ) UNUSED (A8_UNORM)
	float4 Diffuse              : SV_Target1;   // Diffuse Albedo (R8G8B8_UNORM) UNUSED (A8_UNORM)
	float4 Specular             : SV_Target2;   // Specular Color (R8G8B8_UNORM) Specular Power(A8_UNORM)
	float4 NormalVS             : SV_Target3;   // View space normal (R32G32B32_FLOAT) UNUSED(A32_FLOAT)
};


