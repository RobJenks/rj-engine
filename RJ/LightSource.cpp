#include "LightSource.h"


// Default constructor
LightSource::LightSource(void)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::LightSourceObject);

	// Light sources will not collide with anything (although their collision radii are used for illumination tests)
	SetCollisionMode(Game::CollisionMode::NoCollision);
}


// Set the lighting data for this light source
void LightSource::SetLight(const Light & data)
{
	// Store the lighting data
	m_light = data;

	// The object collision radius will be set to equal the light range, for more efficient light 'collision' testing with illuminated objects
	SetCollisionSphereRadius(m_light.Data.Range);
}