#pragma once

#ifndef __SpaceProjectileH__
#define __SpaceProjectileH__

#include "iSpaceObject.h"
class SpaceProjectileDefinition;

class SpaceProjectile : public iSpaceObject
{
public:

	// Default constructor
	SpaceProjectile(void);

	// Constructor accepting the projectile definition as an initialisation parameter
	SpaceProjectile(const SpaceProjectileDefinition *definition);

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
	CMPINLINE void					SetOrientationShiftAmount(const D3DXQUATERNION & dq_per_sec)	{ m_orient_change_amount = dq_per_sec; }


	// Primary simulation method for the projectile.  Inherited from iSpaceObject
	void							SimulateObject(void);

	// Method called when this object collides with another.  Inherited from iObject
	void							CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact);

	// Method called when the projectile exceeds its defined lifetime
	void							EndProjectileLifetime(void);

	// Shut down the projectile object, deallocating all resources.  Inherited from iObject
	void							Shutdown(void);


protected:

	// Pointer back to the projectile definition
	const SpaceProjectileDefinition *	m_definition;			

	// Other key fields for the projectile object
	float								m_lifetime;								// The time (secs) remaining of this projectile's lifetime

	// Data determining whether the projectile momentum is degrading over time
	bool								m_degrade_lv, m_degrade_av;
	float								m_degrade_lv_pc;
	float								m_degrade_av_pc;

	// Data determining whether the projectile orientation will change over time
	bool								m_orient_change;
	D3DXQUATERNION						m_orient_change_amount;

};




#endif