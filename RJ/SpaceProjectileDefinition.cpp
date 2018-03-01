#include "SpaceProjectile.h"

#include "SpaceProjectileDefinition.h"

// Default constructor; set default values
SpaceProjectileDefinition::SpaceProjectileDefinition(void)
	: m_code(NullString), m_model(NULL), m_size(ONE_VECTOR), m_projtype(ProjectileType::Impactor), 
	m_defaultlifetime(10.0f), m_lifeendaction(LifetimeEndAction::Disappear), 
	m_launch_audio(AudioParameters::Null)
{

}

// Creates and returns a new projectile based upon this definition
SpaceProjectile * SpaceProjectileDefinition::CreateProjectile(void) const
{
	// Create a new projectile object, providing this definition as a source of instance data
	SpaceProjectile *proj = new SpaceProjectile(this);
	if (!proj) return NULL;

	// Initialise the simulation state.  This will likely be overidden in the next frame to
	// full simulation, but this will ensure that it is at least registered upon creation
	proj->SetSimulationState(iObject::ObjectSimulationState::StrategicSimulation);

	// Apply any other changes as required

	// Return a reference to the new projectile
	return proj;
}

// Static method to translate a projectile type from its string representation
SpaceProjectileDefinition::ProjectileType SpaceProjectileDefinition::TranslateProjectileTypeFromString(std::string type)
{
	StrLowerC(type);

	if (type == "explosive")						return SpaceProjectileDefinition::ProjectileType::Explosive; 
	else											return SpaceProjectileDefinition::ProjectileType::Impactor;
}

// Static method to translate a projectile type to its string representation
std::string SpaceProjectileDefinition::TranslateProjectileTypeToString(SpaceProjectileDefinition::ProjectileType type)
{
	switch (type)
	{
		case SpaceProjectileDefinition::ProjectileType::Explosive:			return "explosive";
		default:															return "impactor";
	}
}

// Static method to translate a lifetime-end action from its string representation
SpaceProjectileDefinition::LifetimeEndAction SpaceProjectileDefinition::TranslateLifetimeEndActionFromString(std::string action)
{
	StrLowerC(action);

	if (action == "detonate")						return SpaceProjectileDefinition::LifetimeEndAction::Detonate;
	else											return SpaceProjectileDefinition::LifetimeEndAction::Disappear;
}

// Static method to translate a lifetime-end action to its string representation
std::string SpaceProjectileDefinition::TranslateLifetimeEndActionToString(SpaceProjectileDefinition::LifetimeEndAction action)
{
	switch (action)
	{
		case SpaceProjectileDefinition::LifetimeEndAction::Detonate:		return "detonate";
		default:															return "disappear";
	}
}



