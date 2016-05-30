#if !defined(__light_definitionH__)
#define __light_definitionH__

// Enable common usage across C++ and HLSL by making preprocessor adjustments
#ifdef __cplusplus
	#include <DirectXMath.h>

	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMFLOAT4;

#endif


// Structure holding data on a directional (unsituated) light
struct DirLightData
{
	float4							Ambient;
	float4							Diffuse;
	float4							Specular;		// w = power
	float3							Direction;

	// Size = 60, Size+pad = 64, 64 % 16 = 0
	float							_padding;		

#	ifdef __cplusplus
		DirLightData(void) { }
		DirLightData(const float4 & ambient, const float4 & diffuse, const float4 & specular, const float3 & direction)
			: Ambient(ambient), Diffuse(diffuse), Specular(specular), Direction(direction) { }
		DirLightData(const DirLightData & source) 
			: Ambient(source.Ambient), Diffuse(source.Diffuse), Specular(source.Specular), Direction(source.Direction) { }
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
struct LightData
{
	unsigned int					ID;
	float3							Position;
	int								Type;				// Accepts a value from LightingManager::LightType
	float3							Direction;
	float4							Ambient;
	float4							Diffuse;
	float4							Specular;
	float							Range;
	AttenuationData					Attenuation;


	// Size = 96, Size+pad = 96, 96 % 16 = 0
	///*Not req*/					_padding;

#	ifdef __cplusplus
		// Default constructor; note that ID *CANNOT* be set since it needs to be unique
		LightData(void) :	ID(0), Type(0 /* = Directional */), Position(float3(0.0f, 0.0f, 0.0f)), Direction(float3(0.0f, 0.0f, 1.0f)), Range(0.0f),
							Ambient(float4(0.0f, 0.0f, 0.0f, 0.0f)), Diffuse(float4(0.0f, 0.0f, 0.0f, 0.0f)), Specular(float4(0.0f, 0.0f, 0.0f, 0.0f)),
							Attenuation(0.0f, 0.0f, 0.0f) { }

		// Copy constructor; note that ID is *NOT* copied from the source object since it needs to be unique
		LightData(const LightData & source) :	ID(0), Type(source.Type), Position(source.Position), Direction(source.Direction),
												Ambient(source.Ambient), Diffuse(source.Diffuse), Specular(source.Specular),
												Range(source.Range), Attenuation(source.Attenuation) { }
#	endif

};


#endif

