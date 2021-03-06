#pragma once

#ifndef __ProjectileLauncherH__
#define __ProjectileLauncherH__

#include "CompilerSettings.h"
#include "DX11_Core.h"
#include "Projectile.h"
class Ship;
class SpaceTurret;
class SpaceProjectile;
class BasicProjectileDefinition;
class SpaceProjectileDefinition;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ProjectileLauncher : public ALIGN16<ProjectileLauncher>
{
public:

	// Enumeration of the different methods of launching projectiles
	enum ProjectileLaunchMethod { ApplyForce = 0, SetVelocityDirect };

	// Default constructor
	ProjectileLauncher(void);

	// Return or set the unique string code for this projectile launcher type
	CMPINLINE const std::string & 		GetCode(void) const					{ return m_code; }
	CMPINLINE void						SetCode(const std::string & code)	{ m_code = code; }

	// Return or set the descriptive string name for this projectile launcher type
	CMPINLINE const std::string & 		GetName(void) const					{ return m_name; }
	CMPINLINE void						SetName(const std::string & name)	{ m_name = name; }

	// Returns a value indicating whether we are ready to launch a new projectile
	CMPINLINE bool						CanLaunchProjectile(void) const		{ return (Game::ClockMs >= m_nextlaunch); }

	// Launches a projectile.  Returns a pointer to the projectile object that was launched, if applicable.  Will 
	// fire even if not ready (i.e. within reload interval), so CanLaunchProjectile() should be checked before firing
	// Accepts a the position and orientation of the parent launch point as an input.  Returns a reference to the 
	// projectile that was fired, or NULL if nothing was launched
	SpaceProjectile *					LaunchProjectile(const FXMVECTOR launchpoint, const FXMVECTOR launchorient);

	// Returns or changes the projectile class used by this launcher
	CMPINLINE Projectile::ProjectileType		GetProjectileType(void) const									{ return m_projectiletype; }
	CMPINLINE void								ChangeProjectileType(Projectile::ProjectileType projtype)		{ m_projectiletype = projtype; }

	// Set the projectile definition to be used by this launcher.  Also sets the projectiletype to match
	void										SetProjectileDefinition(const BasicProjectileDefinition *def);
	void										SetProjectileDefinition(const SpaceProjectileDefinition *def);

	// Returns the basic projectile type that is used by this launcher (if type == basicprojectile)
	CMPINLINE const BasicProjectileDefinition *	GetBasicProjectileDefinition(void) const						{ return m_basicprojdef; }
	
	// Returns the space projectile type that is used by this launcher (if type == spaceprojectile)
	CMPINLINE const SpaceProjectileDefinition * GetProjectileDefinition(void) const								{ return m_projectiledef; }

	// Methods to get or set basic object properties
	CMPINLINE const Ship *				GetParent(void) const										{ return m_parent; }
	CMPINLINE void						SetParent(Ship *parent)										{ m_parent = parent; }	
	CMPINLINE const SpaceTurret *		GetParentTurret(void) const									{ return m_parentturret; }
	CMPINLINE void						SetParentTurret(SpaceTurret *turret)						{ m_parentturret = turret; }

	CMPINLINE XMVECTOR					GetRelativePosition(void) const								{ return m_relativepos; }
	CMPINLINE void						SetRelativePosition(const FXMVECTOR pos)					{ m_relativepos = pos; }
	CMPINLINE XMVECTOR					GetRelativeOrientation(void) const							{ return m_relativeorient; }
	CMPINLINE void						SetRelativeOrientation(const FXMVECTOR orient)				{ m_relativeorient = orient; }

	CMPINLINE ProjectileLaunchMethod	GetLaunchMethod(void) const									{ return m_launchmethod; }
	CMPINLINE void						SetLaunchMethod(ProjectileLaunchMethod method)				{ m_launchmethod = method; }

	CMPINLINE float						GetProjectileSpread(void) const								{ return m_spread; }
	CMPINLINE XMVECTOR					GetProjectileSpreadDelta(void) const						{ return m_spread_delta; }

	CMPINLINE unsigned int				NextAvailableLaunch(void) const								{ return m_nextlaunch; }
	CMPINLINE void						SetNextAvailableLaunch(unsigned int launchtime)				{ m_nextlaunch = launchtime; }

	// Restarts the counter until the launcher can next fire
	CMPINLINE void						ForceReload(void)											{ m_nextlaunch = (Game::ClockMs + m_launchinterval); }

	// Launch impulse is either a force or a velocity, depending on the launch method
	CMPINLINE float						GetLaunchImpulse(void) const								{ return m_launchimpulse; }
	CMPINLINE void						SetLaunchImpulse(float i)									{ m_launchimpulse = max(0.01f, i); }

	// Returns the actual projectile at launch, calculated based on projectile definition, type and launch method
	CMPINLINE float						GetLaunchVelocity(void) const								{ return m_launch_velocity; }

	CMPINLINE bool						ImpartsOrientationShiftInFlight(void) const					{ return m_launchwithorientchange; }
	CMPINLINE XMVECTOR					GetProjectileOrientationChange(void) const					{ return m_projectileorientchange; }
	CMPINLINE void						SetProjectileOrientationChange(const FXMVECTOR dq)	
	{
		// Ignore any near-zero orientation changes for efficiency
		if (XMVector3Less(XMVectorAbs(dq), Game::C_EPSILON_V))
		{
			m_projectileorientchange = ID_QUATERNION;
			m_launchwithorientchange = false;
			return;
		}

		// Set and normalise the orientation change value, and also the flag which indicates whether any change exists
		m_projectileorientchange = XMVector3NormalizeEst(dq); 
		m_launchwithorientchange = (!IsIDQuaternion(m_projectileorientchange));
	}

	CMPINLINE bool						ImpartsAngularVelocityOnLaunch(void) const					{ return m_launchwithangvel; }
	CMPINLINE XMVECTOR					GetLaunchAngularVelocity(void) const						{ return m_launchangularvelocity; }
	CMPINLINE void						SetProjectileSpin(float rad_per_sec)						
	{ 
		SetLaunchAngularVelocity(XMVectorSetZ(NULL_VECTOR, rad_per_sec));
	}
	CMPINLINE void						SetLaunchAngularVelocity(const FXMVECTOR av)			
	{
		m_launchangularvelocity = av; 
		m_launchwithangvel = (!IsZeroVector3(m_launchangularvelocity));
	}

	CMPINLINE bool						LinearVelocityDegrades(void) const							{ return m_degradelinearvelocity; }
	CMPINLINE bool						AngularVelocityDegrades(void) const							{ return m_degradeangularvelocity; }
	CMPINLINE float						GetLinearVelocityDegradeRate(void) const					{ return m_linveldegradation; }
	CMPINLINE float						GetAngularVelocityDegradeRate(void) const					{ return m_angveldegradation; }
	CMPINLINE void						SetLinearVelocityDegradeState(bool degrades)				{ m_degradelinearvelocity = degrades; }
	CMPINLINE void						SetAngularVelocityDegradeState(bool degrades)				{ m_degradeangularvelocity = degrades; }

	// Set the percentage of launch linear/angular velocity that will degrade per second, if applicable
	CMPINLINE void						SetLinearVelocityDegradeRate(float pc_per_sec)				{ m_linveldegradation = clamp(pc_per_sec, 0.0f, 1.0f); }
	CMPINLINE void						SetAngularVelocityDegradeRate(float pc_per_sec)				{ m_angveldegradation = clamp(pc_per_sec, 0.0f, 1.0f); }

	// Sets the degree of spread applied to projectiles (in radians).  Applies in both local yaw and pitch dimensions.  
	// Measured from origin, so 'spread' is the radius of deviation in radians from actual orientation
	void								SetProjectileSpread(float s);
	
	// The defined launch interval (ms) for projectiles from this launcher
	CMPINLINE unsigned int				GetLaunchInterval(void) const								{ return m_launchinterval; }
	void								SetLaunchInterval(unsigned int interval_ms);

	// The possible launch interval variance, as a percentage of the defined interval.  Half-open range [0.0-...]
	CMPINLINE float						GetLaunchIntervalVariance(void) const						{ return m_launchinterval_variance; }
	void								SetLaunchIntervalVariance(float variance);

	// Determines a new delta trajectory for the next projectile, based upon projectile spread properties
	void								DetermineNextProjectileSpreadDelta(void);

	// Determines the max range of the launcher based on its launch properties and projectiles.  Is only
	// an approximation since the projectiles may have linear velocity degradation or in-flight orientation 
	// changes that we cannot simulate accurately here (without actually firing a projectile)
	float								DetermineApproxRange(void) const;

	// Precalculates data based on the projectile launcher, projectile definitions and other factors
	void								RecalculateLauncherStatistics(void);

	// Copy all launcher data from the specified source object
	void								CopyFrom(const ProjectileLauncher *source);

	// Shutdown method - not required for this class
	CMPINLINE void Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	// Static methods to translate launch method to and from its string representation
	static ProjectileLaunchMethod		TranslateLaunchMethodFromString(std::string method);
	static std::string					TranslateLaunchMethodToString(ProjectileLaunchMethod method);

protected:

	// Recalculate launch interval parameters after a change to the launcher properties
	void								RecalculateLaunchIntervalParameters(void);

	// Returns the clock time at which this launcher is next eligible to fire
	CMPINLINE unsigned int				CalculateNextLaunchTime(void) const 
	{ 
		return (Game::ClockMs + (m_launchinterval_min == m_launchinterval_max ? m_launchinterval_min :
			(unsigned int)irand_lh(m_launchinterval_min, m_launchinterval_max)));
	}

protected:

	std::string							m_code;							// Unique string code for this launcher type
	std::string							m_name;							// Descriptive string name for this launcher type

	Projectile::ProjectileType			m_projectiletype;				// Indicates the type of projectile (basic or full) which will be launched
	const BasicProjectileDefinition *	m_basicprojdef;					// The type of projectile that will be launched (if we are launching basic projectiles)
	const SpaceProjectileDefinition *	m_projectiledef;				// The type of projectile that will be launched (if we are launching full projectiles)

	Ship *								m_parent;						// The parent object that this launcher belongs to
	SpaceTurret *						m_parentturret;					// The parent object turret that this launcher belongs to (or NULL if N/A)

	AXMVECTOR							m_relativepos;					// Relative position in the parent object's coordinate space
	AXMVECTOR							m_relativeorient;				// Relative orientation, relative to the parent object

	float								m_spread;						// Spread, in radians, at launcher origin.  Applies in both local yaw & pitch dimensions
	AXMVECTOR							m_spread_delta;					// Delta orientation to account for spread, to be applied to the next projectile
	float								m_spread_divisor;				// Intermediate calculation; cached for use in each derivation of projectile spread

	unsigned int						m_launchinterval;				// The minimum interval (ms) between projectile launches
	unsigned int						m_nextlaunch;					// The clock timestamp (ms) at which this turret will be able to fire again

	float								m_launchinterval_variance;		// The possible variance in fire interval per launch, as a percentage of the defined interval
	unsigned int						m_launchinterval_min;			// Precalculated launch interval range, to avoid
	unsigned int						m_launchinterval_max;			// recalculating on every launch

	ProjectileLaunchMethod				m_launchmethod;					// The method used to give projectiles their initial trajectory
	float								m_launchimpulse;				// The velocity OR force (depending on launch method) applied to objects at launch
	bool								m_degradelinearvelocity;		// Indicates whether projectile speed will bleed off as it travels, or remain constant
	float								m_linveldegradation;			// The percentange of launch impulse that will bleed off per second, if applicable

	bool								m_launchwithangvel;				// Flag indicating whether projectiles are launched with angular velocity
	AXMVECTOR							m_launchangularvelocity;		// The angular velocity of projectiles at launch
	bool								m_degradeangularvelocity;		// Indicates whether angular velocity will remain constant, or degrade as the projectile travels
	float								m_angveldegradation;			// The percentage of launch angular velocity that will bleed off per second, if applicable

	bool								m_launchwithorientchange;		// Indicates whether projectiles are launched with a 'spin', changing their orientation in flight
	AXMVECTOR							m_projectileorientchange;		// Allow us to impart a gradual orientation change on the projectile during flight

	float								m_launch_velocity;				// Projectile velocity at launch; precalculated to aid in runtime efficiency.  Distance /sec
};




#endif