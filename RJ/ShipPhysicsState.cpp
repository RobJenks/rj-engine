#include "FastMath.h"
#include "ShipPhysicsState.h"

ShipPhysicsState::ShipPhysicsState(void)
{
	// Initialise primary components to their starting default values
	this->Acceleration = NULL_VECTOR;
	this->WorldMomentum = NULL_VECTOR;
	this->Heading = NULL_VECTOR;
	this->AngularVelocity = NULL_VECTOR;

	// Initialise secondary derived components to equivalent default values
	this->LocalMomentum = NULL_VECTOR;
	this->WorldAcceleration = NULL_VECTOR;

	// Initialise size and mass to default values
	SetMass(1.0f);
}

// Sets the ship mass, recalculating derived fields as required
void ShipPhysicsState::SetMass(const float mass)
{
	// We restrict mass to positive non-zero values, to ensure inverse mass is also always valid
	m_mass =	max(mass,				Game::C_EPSILON * 2.0f);
	m_invmass = max(1.0f / m_mass,		Game::C_EPSILON * 2.0f);

	// We want to recalculate the object inertia tensor since the object mass has changed
	RecalculateInertiaTensor();
}


// Method to recalculate the object inertia tensor.  Called whenever a contributing factor changes
void ShipPhysicsState::RecalculateInertiaTensor(void)
{
	// Build an inertia tensor based upon the cuboid approximation (for now).  Dependent on size & mass.
	
}




