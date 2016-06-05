#include "LightSource.h"


// Default constructor
LightSource::LightSource(void)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::LightSourceObject);

	// Light sources will not collide with anything (although their collision radii are used for illumination tests)
	SetCollisionMode(Game::CollisionMode::NoCollision);
	SetCollisionSphereRadius(1.0f);

	// Light sources do perform a post-simulation update to reposition their internal light component
	SetPostSimulationUpdateFlag(true);
}


// Set the lighting data for this light source
void LightSource::SetLight(const Light & data)
{
	// Store the lighting data
	m_light = data;

	// Set light range directly, which will perform validation and update object properties accordingly
	SetRange(data.Data.Range);
}

// Set the range of this light source
void LightSource::SetRange(float range)
{
	// Store the new range value; validate to ensure valid positive values
	m_light.Data.Range = max(range, Game::C_EPSILON);

	// The object collision radius will be set to equal the light range, for more efficient light 'collision' testing with illuminated objects
	SetCollisionSphereRadius(m_light.Data.Range);
}


// Light sources do implement a post-simulation update method to reposition their internal light component
void LightSource::PerformPostSimulationUpdate(void)
{
	// Update the position of our internal light component
	m_light.Data.Position = m_positionf;

	// Update light direction based on orientation of the light source object
	XMVECTOR DirAdj = XMVector3Rotate(XMLoadFloat3(&m_light.Data.Direction), m_orientation);
	XMStoreFloat3(&m_light.Data.Direction, DirAdj);
}






