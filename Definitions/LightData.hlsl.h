#ifndef __LightDataHLSLH__
#define __LightDataHLSLH__

#if defined(__cplusplus) && !defined(RJ_COMPILING_HLSL)
	#include <iostream>
#endif

#include "../Definitions/CppHLSLLocalisation.hlsl.h"


// Limit on the number of lights that can contribute to a scene.  Should be far less of a 
// constraint under deferred rendering system
static const unsigned int LIGHT_RENDER_LIMIT = 512U;

// Supported light types; workaround for lack of enumerations in HLSL
#if defined(__cplusplus) && !defined(RJ_COMPILING_HLSL)
	enum LightType
	{
		Point = 0, 
		Spotlight = 1, 
		Directional = 2
	};
#else
#	define LightType int
#	define LightType::Point 0
#	define LightType::Spotlight 1
#	define LightType::Directional 2
#endif

	// Attenuation parameters
	struct AttenuationData
	{
		float					Constant; 
		float					Linear;
		float					Quadratic;

		// CPP implementation
#		if defined(__cplusplus) && !defined(RJ_COMPILING_HLSL)

			inline AttenuationData(void)
				:
				Constant(0.0f), Linear(0.0f), Quadratic(0.0f)
			{}

			inline AttenuationData(float constant, float linear, float quadratic)
				:
				Constant(constant), Linear(linear), Quadratic(quadratic)
			{}

			inline friend std::ostream & operator<<(std::ostream & out, AttenuationData & data)
			{
				out << "{ Constant: " << data.Constant << ", Linear: " << data.Linear << ", Quadratic: " << data.Quadratic << " }";
				return out;
			}
#		endif
	};

	// Lighting flags
	static const _uint32		LIGHT_FLAG_ENABLED					= (1 << 0);
	static const _uint32		LIGHT_FLAG_SHADOW_MAP				= (1 << 1);

	// Default lighting state
	static const _uint32		LIGHT_FLAG_DEFAULTS = 
	(
		LIGHT_FLAG_ENABLED    
		/* | ... | ... */
	);


	// Primary structure holding light data
	struct LightData
	{
		float4					PositionWS;							// Light position (world space).  Relevant to point & spot lights
		//------------------------------------------ ( 16 bytes )
		float4					DirectionWS;						// Light direction (world space).  Relevant to directional & spot lights
		//------------------------------------------ ( 16 bytes )
		float4					PositionVS;							// Light position (view space).  Relevant to point & spot lights
		//------------------------------------------ ( 16 bytes )
		float4					DirectionVS;						// Light direction (view space).  Relevant to directional & spot lights
		//------------------------------------------ ( 16 bytes )
		float4					Colour;								// Light colour.  Incorporates diffuse + specular components; we don't currently differentiate
		//------------------------------------------ ( 16 bytes )
		LightType				Type;								// The type of light being rendered
		_uint32					Flags;								// As per LIGHT_FLAG_*
		float					Range;								// Range at which light is fully-attenuated and no longer has any effect
		float					Intensity;							// Overall intensity of the light
		//------------------------------------------ ( 16 bytes )
		float					SpotlightAngle;						// Spotlight angle (radians).  Must be in the range [0 PI].  Relevant to spotlights only
		AttenuationData			Attenuation;						// Light attenuation as a function of distance
		//------------------------------------------ ( 16 bytes )
		//------------------------------------------ ( 16 * 7 = 112 bytes)


		// Constructor for use within CPP classes
#if defined(__cplusplus) && !defined(RJ_COMPILING_HLSL)
		inline LightData(void)
			: PositionWS(0, 0, 0, 1)
			, DirectionWS(0, 0, -1, 0)
			, PositionVS(0, 0, 0, 1)
			, DirectionVS(0, 0, 1, 0)
			, Colour(1, 1, 1, 1)
			, Type(LightType::Point)
			, Flags(LIGHT_FLAG_DEFAULTS)
			, Range(100.0f)
			, Intensity(1.0f)
			, SpotlightAngle(0.7854) // == PI/4 == 45 degrees
		{}
#endif

	};


	// TODO: Consider reinstating attenuation parameters from previous implementation as alternative to smoothstep based on range
	




#endif