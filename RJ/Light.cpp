#include "LightData.hlsl.h"
#include "FastMath.h"
#include "Light.h"
#include "GameConsoleCommand.h"

// Initialise static variables
Light::LightID Light::GlobalLightIDCount = 0U;


// Default constructor
Light::Light(void)
{
	AssignNewUniqueID();
}

// Default copy constructor
Light::Light(const Light & source) 
	: Data(source.Data)
{
	AssignNewUniqueID();
}

// Initialise a light to the specified type
void Light::InitialiseDirectionalLight(const XMFLOAT4 & directionWS, const XMFLOAT4 & colour, float intensity)
{
	Data.Type = LightType::Directional;
	Data.DirectionWS = directionWS;
	Data.Colour = colour;
	Data.Intensity = intensity;
}

// Initialise a light to the specified type
void Light::InitialisePointLight(const XMFLOAT4 & positionWS, const XMFLOAT4 & colour, float range, float intensity)
{
	Data.Type = LightType::Point;
	Data.PositionWS = positionWS;
	Data.Colour = colour;
	Data.Range = range;
	Data.Intensity = intensity;
}

// Initialise a light to the specified type
void Light::InitialiseSpotLight(const XMFLOAT4 & positionWS, const XMFLOAT4 & directionWS, const XMFLOAT4 & colour,
								float range, float intensity, float spotlight_angle)
{
	Data.Type = LightType::Spotlight;
	Data.PositionWS = positionWS;
	Data.DirectionWS = directionWS;
	Data.Colour = colour;
	Data.Range = range;
	Data.Intensity = intensity;

	// Spotlight angles must be in the range [0, PI]
	SetSpotlightAngle(spotlight_angle);
}

// Default destructor
Light::~Light(void)
{
}


// Translate a light type value to its string representation.  Defaults to "Directional" if the value is not recognised
std::string Light::TranslateLightTypeToString(LightType type)
{
	switch (type)
	{
		case LightType::Point:						return "Point";
		case LightType::Spotlight:					return "Spotlight";
		case LightType::Directional:				return "Directional";
		default:									return "Unknown";
	}
}

// Translate a light type value from its string representation.  Defaults to "Directional" if the value is not recognised
LightType Light::TranslateLightTypeFromString(std::string type)
{
	StrLowerC(type);
	
	if (type == "directional")						return LightType::Directional;
	else if (type == "spot")						return LightType::Spotlight;
	else											return LightType::Point;
}

// Process a debug command from the console.  "Light" objects are not part of the object hierarchy, but 
// members of that hierarchy will invokve this method when asked to perform lighting-related actions
void Light::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Note: all command parameters will begin at level 2, rather than one, since Light
	// objects are accessed via redirection from a parent object
	INIT_DEBUG_FN_TESTING_AT_LEVEL(command, 2)

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(IsActive)
	REGISTER_DEBUG_ACCESSOR_FN(GetID)
	REGISTER_DEBUG_ACCESSOR_FN(GetType)
	REGISTER_DEBUG_ACCESSOR_FN(GetPositionWS)
	REGISTER_DEBUG_ACCESSOR_FN(GetDirectionWS)
	REGISTER_DEBUG_ACCESSOR_FN(GetPositionVS)
	REGISTER_DEBUG_ACCESSOR_FN(GetDirectionVS)
	REGISTER_DEBUG_ACCESSOR_FN(GetColour)
	REGISTER_DEBUG_ACCESSOR_FN(GetRange)
	REGISTER_DEBUG_ACCESSOR_FN(GetIntensity)
	REGISTER_DEBUG_ACCESSOR_FN(GetSpotlightCosAngle)

	// Mutator methods
	REGISTER_DEBUG_FN(Activate)
	REGISTER_DEBUG_FN(Deactivate)
	REGISTER_DEBUG_FN(SetIsActive, command.ParameterAsBool(3))
	REGISTER_DEBUG_FN(SetType, (LightType)command.ParameterAsInt(3))
	REGISTER_DEBUG_FN(SetPositionWS, XMFLOAT4(command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5), 0.0f))
	REGISTER_DEBUG_FN(SetDirectionWS, XMFLOAT4(command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5), 0.0f))
	REGISTER_DEBUG_FN(SetPositionVS, XMFLOAT4(command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5), 0.0f))
	REGISTER_DEBUG_FN(SetDirectionVS, XMFLOAT4(command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5), 0.0f))
	REGISTER_DEBUG_FN(SetColour, XMFLOAT4(command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5), 0.0f))
	REGISTER_DEBUG_FN(SetRange, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetIntensity, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetSpotlightAngle, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetSpotlightCosAngle, command.ParameterAsFloat(3))

}














