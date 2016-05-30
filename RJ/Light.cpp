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
void Light::InitialiseDirectionalLight(const XMFLOAT3 & direction, const XMFLOAT4 & ambient, const XMFLOAT4 & diffuse, const XMFLOAT4 & specular)
{
	Data.Direction = direction;
	Data.Ambient = ambient;
	Data.Diffuse = diffuse;
	Data.Specular = specular;
}

// Initialise a light to the specified type
void Light::InitialisePointLight(	const XMFLOAT3 & position, const XMFLOAT3 & direction, float range, const XMFLOAT4 & ambient, const XMFLOAT4 & diffuse,
									const XMFLOAT4 & specular, const AttenuationData & attenuation)
{
	Data.Position = position;
	Data.Direction = direction;
	Data.Range = range;
	Data.Ambient = ambient;
	Data.Diffuse = diffuse;
	Data.Specular = specular;
	Data.Attenuation = attenuation;
}


// Default destructor
Light::~Light(void)
{
}