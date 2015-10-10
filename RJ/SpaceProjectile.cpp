#include "SpaceProjectileDefinition.h"
#include "Model.h"

#include "SpaceProjectile.h"


// Default constructor; set default values
SpaceProjectile::SpaceProjectile(void)
	: m_definition(NULL), m_owner(NULL), m_lifetime(1.0f), m_degrade_lv(true), m_degrade_av(false),
	m_degrade_lv_pc(0.01f), m_degrade_av_pc(0.0f), 
	m_orient_change(false), m_orient_change_amount(ID_QUATERNION), m_detach_time(0U),
	m_detached_from_owner(false)
{
	// Set the object type
	this->SetObjectType(iObject::ObjectType::ProjectileObject);

	// This class of space object will perform full collision detection by default (iSpaceObject default = no collision)
	this->SetCollisionMode(Game::CollisionMode::FullCollision);
}

// Constructor accepting the projectile definition as an initialisation parameter
#ifdef RJ_CPP11_SUPPORT
	SpaceProjectile::SpaceProjectile(const SpaceProjectileDefinition *definition) : SpaceProjectile()
#else
	// Perform full construction if C++11 constructor delegation not supported
	SpaceProjectile::SpaceProjectile(const SpaceProjectileDefinition *definition)		
		: m_owner(NULL), m_lifetime(1.0f), m_degrade_lv(true), m_degrade_av(false), m_degrade_lv_pc(0.01f),
		m_degrade_av_pc(0.0f), m_orient_change(false), m_orient_change_amount(ID_QUATERNION), 
		m_detach_time(0U), m_detached_from_owner(false)
#endif
{
	// Set the object type
	this->SetObjectType(iObject::ObjectType::ProjectileObject);

	// This class of space object will perform full collision detection by default (iSpaceObject default = no collision)
	this->SetCollisionMode(Game::CollisionMode::FullCollision);

	// Store and pull data from the definition, if a valid pointer was provided
	m_definition = definition;
	if (m_definition)
	{
		this->SetModel(m_definition->GetModel());
		this->SetMass(m_definition->GetMass());
		this->SetLifetime(m_definition->GetDefaultLifetime());
		
		// Projectile size is derived from the projectile model
		if (m_model)
		{
			this->SetSize(m_model->GetModelSize());
			FloorVector(m_size, 1.0f);
		}
			
	}
}

// Primary simulation method for the projectile.  Inherited from iObject.  Ignores the PermitMovement 
// flag for now, since no expected situations where a projectile needs to be attached & static (yet)
void SpaceProjectile::SimulateObject(void)
{
	// Only simulate if we exist in the world
	if (m_simulationstate == iObject::ObjectSimulationState::NoSimulation) return;

	// Check whether we have exceeded our lifetime
	if ((m_lifetime -= Game::TimeFactor) < 0.0f)
	{
		EndProjectileLifetime();
		return;
	}

	// Check whether we are inside the projectile detachment period
	if (!m_detached_from_owner)
	{
		// Slight hack; if the detach timer is 0, this is the first frame we are simulating.  In this case,
		// don't perform any simulation so that the first rendered frame has the projectile in its 
		// starting location
		if (m_detach_time == 0U) { m_detach_time += Game::ClockDelta; return; }

		// Reduce the countdown and see if we should now detach
		if ((m_detach_time += Game::ClockDelta) > Game::C_PROJECTILE_OWNER_DETACH_PERIOD)
		{
			// Remove the collision exclusion that has been protecting parent and projectile from colliding with each other
			if (m_owner)
			{
				this->RemoveCollisionExclusion(m_owner->GetID());
				m_owner->RemoveCollisionExclusion(this->GetID());
			}
			m_detached_from_owner = true;
		}
	}

	// Degrade linear velocity, if applicable
	if (m_degrade_lv)
	{
		PhysicsState.WorldMomentum -= (PhysicsState.WorldMomentum * m_degrade_lv_pc * Game::TimeFactor);
		RecalculateLocalMomentum();
	}

	// Degrade angular velocity, if applicable
	if (m_degrade_av)
	{
		PhysicsState.AngularVelocity -= (PhysicsState.AngularVelocity * m_degrade_av_pc * Game::TimeFactor);
	}

	// Apply a shift in orientation if required
	if (m_orient_change)
	{
		this->SetOrientation((m_orient_change_amount * Game::TimeFactor) * m_orientation);
	}

	// Move the projectile based upon its current linear momentum
	D3DXVECTOR3 delta = (PhysicsState.WorldMomentum * Game::TimeFactor);
	this->AddDeltaPosition(delta);
	PhysicsState.DeltaMoveDistanceSq = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);

	// Also rotate the object if it has any angular momentum
	if (!IsZeroVector(this->PhysicsState.AngularVelocity))
	{
		this->SetOrientation(m_orientation + (0.5f * D3DXQUATERNION(this->PhysicsState.AngularVelocity.x, this->PhysicsState.AngularVelocity.y,
																	this->PhysicsState.AngularVelocity.z, 0.0f) * this->m_orientation * Game::TimeFactor));
		D3DXQuaternionNormalize(&m_orientation, &m_orientation);
	}
	else
	{
		// If we applied an orientation change, but didn't normalise final orientation based on angular velocity, normalise 
		// our orientation now to ensure it always remains a unit quaternion
		if (m_orient_change) D3DXQuaternionNormalize(&m_orientation, &m_orientation);
	}

}

// Method called when this object collides with another.  Inherited from iObject
void SpaceProjectile::CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact)
{

}

// Method called when the projectile exceeds its defined lifetime
void SpaceProjectile::EndProjectileLifetime(void)
{
	// Take different action depending on the type of projectile
	if (m_definition)
	{
		SpaceProjectileDefinition::LifetimeEndAction action = m_definition->GetLifetimeEndAction();

		// If this projectile detonates at life-end then generate the explosion, damage and impulse here
		if (action == SpaceProjectileDefinition::LifetimeEndAction::Detonate)
		{

		}
	}

	// Regardless of lifetime end action, we finally want to destroy the projectile object and remove it from the simulation
	Shutdown();
}

// Shut down the projectile object, deallocating all resources.  Inherited from iObject
void SpaceProjectile::Shutdown(void)
{
	// Pass control to the base class
	iSpaceObject::Shutdown();
}






