#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/MaterialData.hlsl.h"
#include "../../../Definitions/LightData.hlsl.h"
#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "hlsl_common.hlsl"
#include "lighting_calculations.hlsl"
#include "noise_calculations.hlsl"
#include "shadowmap_calculations.hlsl"
#include "LightDataBuffers.hlsl"
#include "DeferredRenderingBuffers.hlsl"
#include "DeferredRenderingGBuffer.hlsl.h"


// Conditional compilation for shadow mapping support
#ifdef SHADER_SHADOWMAPPED
#	define SHADER_ENTRY		PS_Deferred_Lighting_ShadowMapped 
#	define SHADER_INPUT		VertexShaderStandardOutput	// No longer different, but leave option open for future & CSMs
#else
#	define SHADER_ENTRY		PS_Deferred_Lighting
#	define SHADER_INPUT		VertexShaderStandardOutput
#endif

// Injectable compile-time macros for debug rendering
#define DEBUG_RENDER_PCF	0

// Determines the relative strength of generated noise when modulating the calculated lighting values
static const float LIGHTING_NOISE_STRENGTH = 0.1f;


// Pixel shader that generates the G-Buffer
[earlydepthstencil]
float4 SHADER_ENTRY(SHADER_INPUT IN) : SV_Target0
{
	// All calculations are performed in view space
	float4 eyePos = { 0, 0, 0, 1 };

	// Read all non-depth GBuffer information for this fragment
	int2 texCoord = IN.position.xy - C_Jitter.xy;

	float4 diffuse = GBuffer_DiffuseTextureVS.Load(int3(texCoord, 0));
	float4 specular = GBuffer_SpecularTextureVS.Load(int3(texCoord, 0));
	float4 N = GBuffer_NormalTextureVS.Load(int3(texCoord, 0));

	// Specular power is stored in the alpha component of the specular color; unpack by reversing compression
	float specularPower = exp2(specular.a * 10.5f);

	// Read depth information from the GBuffer for the fragment
	float depth = GBuffer_DepthTextureVS.Load(int3(texCoord, 0)).r;

	// Now determine the view vector from this x/y/depth point to the eye position
	float4 P = ScreenToView(float4(texCoord, depth, 1.0f));
	float4 V = normalize(eyePos - P);
	
	// Retrieve the light being processed
	LightData light = Lights[LightIndex];
	
	// Generate a temorary material and populate the GBuffer data, for use in lighting calculations
	// (TODO: Use smaller, more targeted structure to minimise resource copying cost?)
	MaterialData mat = (MaterialData)0;
	mat.DiffuseColor = diffuse;
	mat.SpecularColor = specular;
	mat.SpecularPower = specularPower;

	// Perform lighting calculations based on light type
	LightingResult lit = (LightingResult)0;
	switch (light.Type)
	{
		case LightType::Directional:
			lit = CalculateDirectionalLight(light, mat, V, P, N);
			break;
		case LightType::Point:
			lit = CalculatePointLight(light, mat, V, P, N);
			break;
		case LightType::Spotlight:
			lit = CalculateSpotLight(light, mat, V, P, N);
			break;
	}

	// Generate rendering noise to apply to the calculated lighting values
	float4 noise = float4(RandomNoise(IN.position.xy), RandomNoise(IN.position.yx), RandomNoise(texCoord*255), 0.0f);
	
	// Determine the shadow factor, if this is a shadow-casting light
#ifdef SHADER_SHADOWMAPPED
	float2 texUV = (texCoord / ScreenDimensions);
	float3 shadow_uv = CalculateShadowMapUVProjection(texUV, depth);
	float shadow_factor = ComputeShadowFactor(shadow_uv.xy, shadow_uv.z);
#	if DEBUG_RENDER_PCF
		float unweighted = (shadow_factor - (1.0f - SHADOW_SHADING_FACTOR)) * (1.0f / SHADOW_SHADING_FACTOR);
#	endif
#else
	static const float shadow_factor = 1.0f;
#endif

	// Determine the total lighting contribution from both GBuffer/Material and lighting calculation data
	float4 total_diffuse = (diffuse * lit.Diffuse * (1.0f - LIGHTING_NOISE_STRENGTH));
	float4 total_specular = (specular * lit.Specular);
	float4 total_noise = (noise * lit.Diffuse * LIGHTING_NOISE_STRENGTH);

	// Debug rendering of PCF thresholds
#if DEBUG_RENDER_PCF && defined(SHADER_SHADOWMAPPED)
	if (unweighted > 0.00001f && unweighted < 0.99999f)
	{
		return (total_diffuse * 0.25f) + float4(shadow_factor, 1.0f - shadow_factor, 0.0f, 1.0f);
	}
#endif

	// Return the combined contribution from all lighting components
	return (total_diffuse + total_specular + total_noise) * shadow_factor;
}