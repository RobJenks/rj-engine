#pragma once

#ifndef __SpaceProjectileLauncherH__
#define __SpaceProjectileLauncherH__

#include "CompilerSettings.h"
#include "DX11_Core.h"
class iSpaceObject;
class SpaceProjectileDefinition;


class SpaceProjectileLauncher
{
public:

	// Enumeration of the different methods of launching projectiles
	enum ProjectileLaunchMethod { ApplyForce = 0, SetVelocityDirect };

	// Default constructor
	SpaceProjectileLauncher(void);

	// Launches a projectile.  Returns a pointer to the projectile object that was launched, if applicable
	SpaceProjectile *					LaunchProjectile(void);

	// Set or return the projectile type that is used by this launcher
	CMPINLINE const SpaceProjectileDefinition * GetProjectileDefinition(void) const								{ return m_projectiledef; }
	CMPINLINE void								SetProjectileDefinition(const SpaceProjectileDefinition *def)	{ m_projectiledef = def; }

	// Methods to get or set basic object properties
	CMPINLINE const iSpaceObject *		GetParent(void) const										{ return m_parent; }
	CMPINLINE void						SetParent(iSpaceObject *parent)								{ m_parent = parent; }	
	CMPINLINE D3DXVECTOR3				GetRelativePosition(void) const								{ return m_relativepos; }
	CMPINLINE void						SetRelativePosition(const D3DXVECTOR3 & pos)				{ m_relativepos = pos; }
	CMPINLINE D3DXQUATERNION			GetRelativeOrientation(void) const							{ return m_relativeorient; }
	CMPINLINE void						SetRelativeOrientation(const D3DXQUATERNION & orient)		{ m_relativeorient = orient; }

	CMPINLINE ProjectileLaunchMethod	GetLaunchMethod(void) const									{ return m_launchmethod; }
	CMPINLINE void						SetLaunchMethod(ProjectileLaunchMethod method)				{ m_launchmethod = method; }

	// Launch impulse is either a force or a velocity, depending on the launch method
	CMPINLINE float						GetLaunchImpulse(void) const								{ return m_launchimpulse; }
	CMPINLINE void						SetLaunchImpulse(float i)									{ m_launchimpulse = max(0.01f, i); }

	CMPINLINE bool						ImpartsOrientationShiftInFlight(void) const					{ return m_launchwithorientchange; }
	CMPINLINE D3DXQUATERNION			GetProjectileOrientationChange(void) const					{ return m_projectileorientchange; }
	CMPINLINE void						SetProjectileOrientationChange(const D3DXQUATERNION & dq)	{ 
		m_projectileorientchange = dq; 
		D3DXQuaternionNormalize(&m_projectileorientchange, &m_projectileorientchange);
		m_launchwithorientchange = (IsIDQuaternion(m_projectileorientchange));
	}

	CMPINLINE bool						ImpartsAngularVelocityOnLaunch(void) const					{ return m_launchwithangvel; }
	CMPINLINE D3DXVECTOR3				GetLaunchAngularVelocity(void) const						{ return m_launchangularvelocity; }
	CMPINLINE void						SetProjectileSpin(float rad_per_sec)						{ SetLaunchAngularVelocity(D3DXVECTOR3(0.0f, 0.0f, rad_per_sec)); }
	CMPINLINE void						SetLaunchAngularVelocity(const D3DXVECTOR3 & av)			
	{
		m_launchangularvelocity = av; 
		m_launchwithangvel = (!IsZeroVector(m_launchangularvelocity));
	}

	CMPINLINE bool						LinearVelocityDegrades(void) const							{ return m_degradelinearvelocity; }
	CMPINLINE bool						AngularVelocityDegrades(void) const							{ return m_degradeangularvelocity; }
	CMPINLINE void						SetLinearVelocityDegradeState(bool degrades)				{ m_degradelinearvelocity = degrades; }
	CMPINLINE void						SetAngularVelocityDegradeState(bool degrades)				{ m_degradeangularvelocity = degrades; }

	// Set the percentage of launch linear/angular velocity that will degrade per second, if applicable
	CMPINLINE void						SetLinearVelocityDegradeRate(float pc_per_sec)				{ m_linveldegradation = clamp(pc_per_sec, 0.0f, 1.0f); }
	CMPINLINE void						SetAngularVelocityDegradeRate(float pc_per_sec)				{ m_angveldegradation = clamp(pc_per_sec, 0.0f, 1.0f); }

protected:

	const SpaceProjectileDefinition *	m_projectiledef;				// The type of projectile that will be launched

	iSpaceObject *						m_parent;						// The parent object that this launcher belongs to

	D3DXVECTOR3							m_relativepos;					// Relative position in the parent object's coordinate space
	D3DXQUATERNION						m_relativeorient;				// Relative orientation, relative to the parent object

	ProjectileLaunchMethod				m_launchmethod;					// The method used to give projectiles their initial trajectory
	float								m_launchimpulse;				// The velocity OR force (depending on launch method) applied to objects at launch
	bool								m_degradelinearvelocity;		// Indicates whether projectile speed will bleed off as it travels, or remain constant
	float								m_linveldegradation;			// The percentange of launch impulse that will bleed off per second, if applicable

	bool								m_launchwithangvel;				// Flag indicating whether projectiles are launched with angular velocity
	D3DXVECTOR3							m_launchangularvelocity;		// The angular velocity of projectiles at launch
	bool								m_degradeangularvelocity;		// Indicates whether angular velocity will remain constant, or degrade as the projectile travels
	float								m_angveldegradation;			// The percentage of launch angular velocity that will bleed off per second, if applicable

	bool								m_launchwithorientchange;		// Indicates whether projectiles are launched with a 'spin', changing their orientation in flight
	D3DXQUATERNION						m_projectileorientchange;		// Allow us to impart a gradual orientation change on the projectile during flight

};




#endif