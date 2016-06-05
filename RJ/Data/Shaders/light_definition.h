#if !defined(__light_definitionH__)
#define __light_definitionH__

// Enable common usage across C++ and HLSL by making preprocessor adjustments
#ifdef __cplusplus
	#include <DirectXMath.h>

	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMFLOAT4;

#endif

// Constant array of bitstring values corresponding to each possile light config value
static const unsigned int CONFIG_VAL[64] = { 1U, 2U, 4U, 8U, 16U, 32U, 64U, 128U, 256U, 512U, 1024U, 2048U, 4096U, 8192U, 16384U, 32768U, 65536U, 131072U, 262144U, 524288U, 1048576U, 2097152U, 4194304U, 8388608U, 16777216U, 33554432U, 67108864U, 134217728U, 268435456U, 536870912U, 1073741824U, 2147483648U, 1U, 2U, 4U, 8U, 16U, 32U, 64U, 128U, 256U, 512U, 1024U, 2048U, 4096U, 8192U, 16384U, 32768U, 65536U, 131072U, 262144U, 524288U, 1048576U, 2097152U, 4194304U, 8388608U, 16777216U, 33554432U, 67108864U, 134217728U, 268435456U, 536870912U, 1073741824U, 2147483648U };

// Structure holding data on a directional (unsituated) light
struct BaseLightData
{
	// Float4
	unsigned int					ID;						
	float3							Colour;

	// Float4
	int								Type;					// Accepts a value from LightingManager::LightType
	float							AmbientIntensity;
	float							DiffuseIntensity;
	float							SpecularPower;

	// Float4
	float3							Direction;				// Only relevant for directional lights; calculated otherwise
	float							_padding;

	// Size % 16 == 0

#	ifdef __cplusplus
		// Constructor; note that ID *CANNOT* be set since it needs to be unique
		BaseLightData(void)
			: ID(0U), Type(0), Colour(0.0f, 0.0f, 0.0f), AmbientIntensity(0.0f), DiffuseIntensity(0.0f), SpecularPower(0.0f), Direction(0.0f, 0.0f, 1.0f) { }
		
		// Constructor; note that ID *CANNOT* be set since it needs to be unique
		BaseLightData(int type, const float3 & colour, float ambient, float diffuse, float specular, const float3 & direction)
			: ID(0U), Type(type), Colour(colour), AmbientIntensity(ambient), DiffuseIntensity(diffuse), SpecularPower(specular), Direction(direction) { }

		// Copy constructor; note that ID *CANNOT* be set since it needs to be unique
		BaseLightData(const BaseLightData & source) 
			: ID(0U), Type(source.Type), Colour(source.Colour), AmbientIntensity(source.AmbientIntensity), 
			  DiffuseIntensity(source.DiffuseIntensity), SpecularPower(source.SpecularPower), Direction(source.Direction) { }
#	endif
};

// Structure holding attentuation data for a light object
struct AttenuationData
{
	float							Constant;
	float							Linear;
	float							Exp;

#	ifdef __cplusplus
		AttenuationData(float constant, float linear, float exp)	: Constant(constant), Linear(linear), Exp(exp) { }
		AttenuationData(const AttenuationData & source)				: Constant(source.Constant), Linear(source.Linear), Exp(source.Exp) { }
#	endif
};

// Structure holding data on a point light which is situated in space
struct LightData : BaseLightData
{
	// Float4
	float3							Position;
	float							Range;

	// Float4	
	AttenuationData					Attenuation;
	float							_padding;


	// Size % 16 == 0

#	ifdef __cplusplus
		// Constructor; note that ID *CANNOT* be set since it needs to be unique
		LightData(void) 
			: BaseLightData(), Position(0.0f, 0.0f, 0.0f), Range(0.0f), Attenuation(0.0f, 0.0f, 0.0f) { }

		// Constructor; note that ID *CANNOT* be set since it needs to be unique
		LightData(const BaseLightData & base, float3 position, float range, const AttenuationData & atten) 
			: BaseLightData(base), Position(position), Range(range), Attenuation(atten) { }

		// Copy constructor; note that ID is *NOT* copied from the source object since it needs to be unique
		LightData(const LightData & source) :	BaseLightData((BaseLightData)source), Position(source.Position), 
												Range(source.Range), Attenuation(source.Attenuation) { }
#	endif

};


#endif

