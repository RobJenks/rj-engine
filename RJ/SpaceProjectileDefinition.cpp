#include "SpaceProjectile.h"

#include "SpaceProjectileDefinition.h"

// Default constructor; set default values
SpaceProjectileDefinition::SpaceProjectileDefinition(void)
	: m_code(NullString), m_model(NULL), m_projtype(ProjectileType::Impactor), 
	m_defaultlifetime(10.0f), m_lifeendaction(LifetimeEndAction::Disappear)
{

}

// Creates and returns a new projectile based upon this definition
SpaceProjectile * SpaceProjectileDefinition::CreateProjectile(void) const
{
	// Create a new projectile object, providing this definition as a source of instance data
	SpaceProjectile *proj = new SpaceProjectile(this);
	if (!proj) return NULL;

	// Apply any other changes as required

	// Return a reference to the new projectile
	return proj;
}
