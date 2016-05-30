#if !defined(__standard_const_bufferH__)
#define __standard_const_bufferH__

#include "render_constants.h"
#include "light_definition.h"
#include "material_definition.h"

// Enable common usage across C++ and HLSL by making preprocessor adjustments
#ifdef __cplusplus
	#include <DirectXMath.h>
	using float2 = DirectX::XMFLOAT2;
	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMFLOAT4;
	#define cbuffer struct
#endif


// Constant buffer holding all material, lighting and other data for the render pass
cbuffer StandardPSConstBuffer
{
	/* GENERAL DATA */
	float3					EyeWorldPos;
	int					FogEnabled;
	float4					FogColour;
	float					FogStart;
	float					FogRange;
	
	unsigned int			MaterialCount;
	unsigned int			LightCount;


	MaterialData			Materials[C_MATERIAL_LIMIT];		// Guaranteed to have SIZE % 16 == 0
	
	/*MATERIAL_ID						ID;
	float4							Ambient;
	float4							Diffuse;
	float4							Specular; // w = SpecPower
	float4							Reflect;
	// Size = 68, Size+pad = 80, 80 % 16 = 0
	float3							_padding;*/

	/* LIGHTING DATA*/

	// Single unsituated directional light
	//TODO30
	DirLightData			DirLight;							// Guaranteed to have SIZE % 16 == 0

	// Array of lights in the scene, and the number (<= LC_LIGHT_LIMIT) which are active
	
	//LightData				Lights[C_LIGHT_LIMIT];				// Guaranteed to have SIZE % 16 == 0


	// Padding only needs to account for the primitive types; all structs have been padded to % 16 == 0 already
	// Primitive size = 12*4 = 48, 48 % 16 == 0, no additional padding required
	// /* None req */		_padding;
};


#endif



