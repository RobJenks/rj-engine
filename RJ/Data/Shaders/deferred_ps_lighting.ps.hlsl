#include "../../CommonShaderPipelineStructures.hlsl.h"
#include "../../CommonShaderBufferDefinitions.hlsl.h"
#include "../../../Definitions/MaterialData.hlsl.h"
#include "../../../Definitions/LightData.hlsl.h"
#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"
#include "hlsl_projection_common.hlsl"
#include "lighting_calculations.hlsl"
#include "LightDataBuffers.hlsl"
#include "DeferredRenderingBuffers.hlsl"
#include "DeferredRenderingGBuffer.hlsl.h"


// Pixel shader that generates the G-Buffer
[earlydepthstencil]
float4 PS_Deferred_Lighting(VertexShaderStandardOutput IN) : SV_Target0
{
	// All calculations are performed in view space
	float4 eyePos = { 0, 0, 0, 1 };

	// Read all non-depth GBuffer information for this fragment
	int2 texCoord = IN.position.xy;

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

	// Return the total lighting contribution from both GBuffer/Material and lighting calculation data
	return (diffuse * lit.Diffuse) + (specular * lit.Specular);
}