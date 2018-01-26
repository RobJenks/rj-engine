#ifndef __LightDataHLSLH__
#define __LightDataHLSLH__

#include "CppHLSLLocalisation.hlsl.h"


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
		bool					Enabled;							// Disabled light will be skipped entirely during lighting calculations.  TODO: bool portable across cpp/hlsl?
		float					Range;								// Range at which light is fully-attenuated and no longer has any effect
		float					Intensity;							// Overall intensity of the light
		//------------------------------------------ ( 16 bytes )
		float					SpotlightCosAngle;					// Cosine of the spotlight angle.  Relevant to spotlights only
		float3					_padding;							// To maintain alignment and packing requirements
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
			, Enabled(true)
			, Range(100.0f)
			, Intensity(1.0f)
			, SpotlightCosAngle(0.7854) // == PI/4 == 45 degrees
		{}
#endif

	};


	// TODO: Consider reinstating attenuation parameters from previous implementation as alternative to smoothstep based on range
	




#endif