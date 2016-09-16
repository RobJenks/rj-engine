#if !defined(__light_definitionH__)
#define __light_definitionH__

// Enable common usage across C++ and HLSL by making preprocessor adjustments
#ifdef __cplusplus
	#include <DirectXMath.h>

	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMFLOAT4;

#endif

// Constant array of bitstring values corresponding to each possile light config value
// TODO: ERROR?  REPEATS TWICE AT 2^0 AND 2^32
static const unsigned int CONFIG_VAL[64] = { 1U, 2U, 4U, 8U, 16U, 32U, 64U, 128U, 256U, 512U, 1024U, 2048U, 4096U, 8192U, 16384U, 32768U, 65536U, 131072U, 262144U, 524288U, 1048576U, 2097152U, 4194304U, 8388608U, 16777216U, 33554432U, 67108864U, 134217728U, 268435456U, 536870912U, 1073741824U, 2147483648U, 1U, 2U, 4U, 8U, 16U, 32U, 64U, 128U, 256U, 512U, 1024U, 2048U, 4096U, 8192U, 16384U, 32768U, 65536U, 131072U, 262144U, 524288U, 1048576U, 2097152U, 4194304U, 8388608U, 16777216U, 33554432U, 67108864U, 134217728U, 268435456U, 536870912U, 1073741824U, 2147483648U };

// Structure holding attentuation data for a light object
struct AttenuationData
{
	float							Constant;
	float							Linear;
	float							Exp;

#	ifdef __cplusplus
	AttenuationData(void) { }
	AttenuationData(float constant, float linear, float exp) : Constant(constant), Linear(linear), Exp(exp) { }
	AttenuationData(const AttenuationData & source) : Constant(source.Constant), Linear(source.Linear), Exp(source.Exp) { }
#	endif
};

// Structure holding data on a directional (unsituated) light
struct LightData
{
	// Float4
	unsigned int					ID;						
	float3							Colour;

	// Float4
	int								Type;							// Accepts a value from Light::LightType
	float							AmbientIntensity;
	float							DiffuseIntensity;
	float							SpecularPower;

	// Float4
	float3							Direction;						// Only relevant for directional or spot lights (different usage in each)
	float							SpotlightInnerHalfAngleCos;		// Only relevant for spot lights.  Inner half-angle where the spotlight intensity is still 1.0

	// Float4
	float							SpotlightOuterHalfAngleCos;		// Only relevant for spot lights.  Outer half-angle where light intensity drops to 0.0
	float3							Position;

	// Float4	
	AttenuationData					Attenuation;
	float							Range;

	// Size % 16 == 0

#	ifdef __cplusplus
		// Constructor; note that ID *CANNOT* be set since it needs to be unique
		LightData(void)
			: 
			ID(0U), Type(0), Colour(0.0f, 0.0f, 0.0f), AmbientIntensity(0.0f), DiffuseIntensity(0.0f), SpecularPower(0.0f), 
			Direction(0.0f, 0.0f, 1.0f), Range(0.0f), Position(0.0f, 0.0f, 0.0f) { }
		
		// Constructor; note that ID *CANNOT* be set since it needs to be unique
		LightData(int type, const float3 & colour, float ambient, float diffuse, float specular, const float3 & direction)
			: 
			ID(0U), Type(type), Colour(colour), AmbientIntensity(ambient), DiffuseIntensity(diffuse), SpecularPower(specular), Direction(direction) { }

		// Constructor; note that ID *CANNOT* be set since it needs to be unique
		LightData(	int type, const float3 & colour, float ambient, float diffuse, float specular, const float3 & direction,
					float range, const float3 & position, const AttenuationData & atten)
			:
			ID(0U), Type(type), Colour(colour), AmbientIntensity(ambient), DiffuseIntensity(diffuse), SpecularPower(specular), Direction(direction),
			Range(range), Position(position), Attenuation(atten) { }

		// Copy constructor; note that ID *CANNOT* be set since it needs to be unique
		LightData(const LightData & source)
			:	
			ID(0U), Type(source.Type), Colour(source.Colour), AmbientIntensity(source.AmbientIntensity),
			DiffuseIntensity(source.DiffuseIntensity), SpecularPower(source.SpecularPower), Direction(source.Direction),
			Range(source.Range), Position(source.Position), Attenuation(source.Attenuation) { }
#	endif
};






#endif

