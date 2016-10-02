#pragma once

#ifndef __SpaceProjectileH__
#define __SpaceProjectileH__

#include "iSpaceObject.h"
class SpaceProjectileDefinition;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class SpaceProjectile : public ALIGN16<SpaceProjectile>, public iSpaceObject
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(SpaceProjectile)

	// Default constructor
	SpaceProjectile(void);

	// Constructor accepting the projectile definition as an initialisation parameter
	SpaceProjectile(const SpaceProjectileDefinition *definition);

	// Return or set the owner that launched this projectile
	CMPINLINE iSpaceObject *		GetOwner(void) const											{ return m_owner; }
	CMPINLINE void					SetOwner(iSpaceObject *owner)									{ m_owner = owner; }

	// Return the amount of time (secs) remaining in this projectile's lifetime
	CMPINLINE float					GetLifetime(void) const											{ return m_lifetime; }

	// Sets the lifetime (secs) of this projectile
	CMPINLINE void					SetLifetime(float L)											{ m_lifetime = L; }

	// Retrieve or set properties of the projectile that will affect its trajectory during flight
	CMPINLINE bool					IsLinearVelocityDegrading(void) const							{ return m_degrade_lv; }
	CMPINLINE bool					IsAngularVelocityDegrading(void) const							{ return m_degrade_av; }
	CMPINLINE void					SetLinearVelocityDegradation(bool degrades)						{ m_degrade_lv = degrades; }
	CMPINLINE void					SetAngularVelocityDegradation(bool degrades)					{ m_degrade_av = degrades; }
	CMPINLINE void					SetLinearVelocityDegradePc(float pc)							{ m_degrade_lv_pc = pc; }
	CMPINLINE void					SetAngularVelocityDegradePc(float pc)							{ m_degrade_av_pc = pc; }
	CMPINLINE bool					HasOrientationShiftDuringFlight(void) const						{ return m_orient_change; }
	CMPINLINE void					SetOrientationShift(bool active)								{ m_orient_change = active; }
	CMPINLINE void					SetOrientationShiftAmount(const FXMVECTOR dq_per_sec)			{ m_orient_change_amount = dq_per_sec; }


	// Primary simulation method for the projectile.  Inherited from iSpaceObject
	void							SimulateObject(void);

	// Method called when this object collides with another.  Inherited from iObject
	void							CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact);

	// Method called when the projectile exceeds its defined lifetime
	void							EndProjectileLifetime(void);

	// Event triggered upon destruction of the object
	void							DestroyObject(void);

	// Shut down the projectile object, deallocating all resources.  Inherited from iObject
	void							Shutdown(void);

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void							ProcessDebugCommand(GameConsoleCommand & command);


protected:

	// Pointer back to the projectile definition
	const SpaceProjectileDefinition *	m_definition;			

	// Maintain a reference back to the object that launched this projectile
	iSpaceObject *						m_owner;

	// Projectiles will be protected from colliding with their owners for a short period after launch, until this counter reaches the threshold
	unsigned int						m_detach_time;						// Time that the projectile has existed, until detach takes place (ms)
	bool								m_detached_from_owner;

	// Other key fields for the projectile object
	float								m_lifetime;								// The time (secs) remaining of this projectile's lifetime

	// Data determining whether the projectile momentum is degrading over time
	bool								m_degrade_lv, m_degrade_av;
	float								m_degrade_lv_pc;
	float								m_degrade_av_pc;

	// Data determining whether the projectile orientation will change over time
	bool								m_orient_change;
	AXMVECTOR							m_orient_change_amount;

};




#endif