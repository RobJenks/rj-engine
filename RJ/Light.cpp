#include "FastMath.h"
#include "Light.h"
#include "GameConsoleCommand.h"
#include "Data\\Shaders\\light_definition.h"

// Static counter used to assign unique light IDs
unsigned int Light::LAST_ID = 0U;

// Method to return a new unique ID
unsigned int Light::NewUniqueID(void)
{
	return (++Light::LAST_ID);
}

// Default constructor
Light::Light(void)
	: m_active(true), Data()
{
	// Assign a new unique ID
	Data.ID = Light::NewUniqueID();
}

// Constructor accepting the core light data struct as a parameter
Light::Light(const LightData & light_data)
	: m_active(true), Data(light_data)
{
	// Assign a new unique ID
	Data.ID = Light::NewUniqueID();
}

// Custom copy constructor
Light::Light(const Light & source) 
	: m_active(source.IsActive()), Data(source.Data)
{
	// Assign a new unique ID to distingush from the source light object
	Data.ID = Light::NewUniqueID();
}

// Initialise a light to the specified type
void Light::InitialiseDirectionalLight(const XMFLOAT3 & direction, const XMFLOAT3 & colour, float ambient, float diffuse, float specular)
{
	Data.Type = Light::LightType::Directional;
	Data.Direction = direction;
	Data.Colour = colour;
	Data.AmbientIntensity = ambient;
	Data.DiffuseIntensity = diffuse;
	Data.SpecularPower = specular;
}

// Initialise a light to the specified type
void Light::InitialisePointLight(	const XMFLOAT3 & position, const XMFLOAT3 & colour, float range, 
									float ambient, float diffuse, float specular, const AttenuationData & attenuation)
{
	Data.Type = Light::LightType::PointLight;
	Data.Position = position;
	Data.Colour = colour;
	Data.Range = range;
	Data.AmbientIntensity = ambient;
	Data.DiffuseIntensity = diffuse;
	Data.SpecularPower = specular;
	Data.Attenuation = attenuation;
}

// Initialise a light to the specified type
void Light::InitialiseSpotLight(const XMFLOAT3 & position, const XMFLOAT3 & colour, float range,
								float ambient, float diffuse, float specular, const AttenuationData & attenuation,
								const XMFLOAT3 & direction, float inner_half_angle, float outer_half_angle)
{
	Data.Type = Light::LightType::SpotLight;
	Data.Position = position;
	Data.Colour = colour;
	Data.Range = range;
	Data.AmbientIntensity = ambient;
	Data.DiffuseIntensity = diffuse;
	Data.SpecularPower = specular;
	Data.Attenuation = attenuation;
	Data.Direction = direction;

	// Spotlight angles must be in the range [0, PI], and must have outer > inner
	Data.SpotlightInnerHalfAngleCos =     std::cosf(clamp(inner_half_angle, 0.0f, PI));
	Data.SpotlightOuterHalfAngleCos = min(std::cosf(clamp(outer_half_angle, 0.0f, PI)), Data.SpotlightInnerHalfAngleCos - 0.01f);
}

// Default destructor
Light::~Light(void)
{
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
	REGISTER_DEBUG_ACCESSOR_FN(GetColour)
	REGISTER_DEBUG_ACCESSOR_FN(GetType)
	REGISTER_DEBUG_ACCESSOR_FN(GetAmbient)
	REGISTER_DEBUG_ACCESSOR_FN(GetDiffuse)
	REGISTER_DEBUG_ACCESSOR_FN(GetSpecular)
	REGISTER_DEBUG_ACCESSOR_FN(GetDirection)
	REGISTER_DEBUG_ACCESSOR_FN(GetSpotlightInnerHalfAngleCos)
	REGISTER_DEBUG_ACCESSOR_FN(GetSpotlightOuterHalfAngleCos)
	REGISTER_DEBUG_ACCESSOR_FN(GetPosition)
	REGISTER_DEBUG_ACCESSOR_FN(GetRange)
	REGISTER_DEBUG_ACCESSOR_FN(GetAttenuationConstant)
	REGISTER_DEBUG_ACCESSOR_FN(GetAttenuationLinear)
	REGISTER_DEBUG_ACCESSOR_FN(GetAttenuationExp)

	// Mutator methods
	REGISTER_DEBUG_FN(Activate)
	REGISTER_DEBUG_FN(Deactivate)
	REGISTER_DEBUG_FN(SetIsActive, command.ParameterAsBool(3))
	REGISTER_DEBUG_FN(SetID, (unsigned int)command.ParameterAsInt(3))
	REGISTER_DEBUG_FN(SetColour, XMFLOAT3(command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5)))
	REGISTER_DEBUG_FN(SetType, command.ParameterAsInt(3))
	REGISTER_DEBUG_FN(SetAmbient, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetDiffuse, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetSpecular, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetDirection, XMFLOAT3(command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5)))
	REGISTER_DEBUG_FN(SetSpotlightInnerHalfAngleCos, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetSpotlightOuterHalfAngleCos, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetPosition, XMFLOAT3(command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5)))
	REGISTER_DEBUG_FN(SetRange, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetAttenuationConstant, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetAttenuationLinear, command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(SetAttenuationExp, command.ParameterAsFloat(3))

}














