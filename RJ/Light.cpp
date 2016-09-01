#include "FastMath.h"
#include "Light.h"
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
	: Data()
{
	// Assign a new unique ID
	Data.ID = Light::NewUniqueID();
}

// Custom copy constructor
Light::Light(const Light & source) 
	: Data(source.Data)
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