#include "FastMath.h"
#include "SpaceSystem.h"
#include "SpaceProjectile.h"
#include "BasicProjectileSet.h"
#include "SpaceProjectileDefinition.h"

#include "ProjectileLauncher.h"


// Default constructor; sets all fields to default values
ProjectileLauncher::ProjectileLauncher(void) : 
	m_projectiletype(Projectile::ProjectileType::BasicProjectile), m_basicprojdef(NULL), m_projectiledef(NULL), 
	m_parent(NULL), m_parentturret(NULL), m_relativepos(NULL_VECTOR), m_relativeorient(ID_QUATERNION), 
	m_launchmethod(ProjectileLaunchMethod::ApplyForce), m_launchimpulse(250.0f), m_degradelinearvelocity(true), 
	m_linveldegradation(0.1f), m_launchwithangvel(false), m_launchangularvelocity(NULL_VECTOR), 
	m_degradeangularvelocity(false), m_angveldegradation(0.0f), m_launchwithorientchange(false), 
	m_projectileorientchange(ID_QUATERNION), m_spread(0.0f), m_spread_delta(ID_QUATERNION), m_spread_divisor(0.0f),
	m_launchinterval(1000U), m_nextlaunch(0U)
{
	
}

// Set the projectile definition to be used by this launcher.  Also sets the projectiletype to match
void ProjectileLauncher::SetProjectileDefinition(const BasicProjectileDefinition *def)
{
	// Store the new definition and update our projectile type
	ChangeProjectileType(Projectile::ProjectileType::BasicProjectile);
	m_basicprojdef = def;
}

// Set the projectile definition to be used by this launcher.  Also sets the projectiletype to match
void ProjectileLauncher::SetProjectileDefinition(const SpaceProjectileDefinition *def)
{
	ChangeProjectileType(Projectile::ProjectileType::SpaceProjectile);
	m_projectiledef = def;
}

// Launches a projectile.  Returns a pointer to the projectile object that was launched, if applicable.  Will 
// fire even if not ready (i.e. within reload interval), so CanLaunchProjectile() should be checked before firing
// Accepts a the position and orientation of the parent launch point as an input.  Returns a reference to the 
// projectile that was fired, or NULL if nothing was launched
SpaceProjectile *ProjectileLauncher::LaunchProjectile(const FXMVECTOR launchpoint, const FXMVECTOR launchorient)
{
	// Validate required properties
	if (!m_parent || !m_parent->GetSpaceEnvironment()) return NULL;

	// Determine launch position based upon our parent object
	XMVECTOR pos = XMVectorAdd(launchpoint, XMVector3Rotate(m_relativepos, launchorient));

	// Determine the spread effect for this projectile, if applicable
	XMVECTOR orient;
	if (m_spread > Game::C_EPSILON)
	{
		DetermineNextProjectileSpreadDelta();
		orient = XMQuaternionMultiply(XMQuaternionMultiply(m_spread_delta, m_relativeorient), launchorient);
	}
	else
	{
		orient = XMQuaternionMultiply(m_relativeorient, launchorient);
	}

	// Update the timestamp at which we will be ready to launch our next projectile
	m_nextlaunch = (Game::ClockMs + m_launchinterval);


	// Take different action depending on whether we are launching a basic or full projectile
	if (m_projectiletype == Projectile::ProjectileType::BasicProjectile)
	{
		// Create a new projectile from the stored definition
		if (!m_basicprojdef) return NULL;
		m_parent->GetSpaceEnvironment()->Projectiles.AddProjectile(m_basicprojdef, m_parent->GetID(), pos, orient, m_parent->PhysicsState.WorldMomentum);

		// We do not return a reference to basic projectiles since they are transient
		return NULL;
	}
	else if (m_projectiletype == Projectile::ProjectileType::SpaceProjectile)
	{
		// Attempt to create a new projectile from the definition; return NULL if we cannot create one for any reason
		if (!m_projectiledef) return NULL;
		SpaceProjectile *proj = m_projectiledef->CreateProjectile();
		if (!proj) return NULL;

		// Set position, orientation, and maintain a reference to the object which launched us
		proj->SetPosition(pos);
		proj->SetOrientation(orient);
		proj->SetOwner(m_parent);

		// Perform an immediate refresh of projectile position/orientation to recalcualate its transform matrices
		proj->RefreshPositionImmediate();

		// Apply launch impulse to the projectile
		float impulse = (m_launchmethod == ProjectileLaunchMethod::ApplyForce ? (m_launchimpulse * proj->GetInverseMass()) : m_launchimpulse);
		impulse = clamp(impulse, 0.01f, Game::C_PROJECTILE_VELOCITY_LIMIT);
		proj->ApplyLocalLinearForceDirect(XMVectorSetZ(NULL_VECTOR, impulse));

		// Store linear trajectory properties in the projectile
		proj->SetLinearVelocityDegradation(m_degradelinearvelocity);
		if (m_degradelinearvelocity) proj->SetLinearVelocityDegradePc(m_linveldegradation);

		// Apply angular velocity & degradation to the projectile, if applicable
		if (m_launchwithangvel)
		{
			proj->ApplyAngularVelocity(m_launchangularvelocity);
			proj->SetAngularVelocityDegradation(m_degradeangularvelocity);
			if (m_degradeangularvelocity) proj->SetAngularVelocityDegradePc(m_angveldegradation);
		}
		else { proj->SetAngularVelocityDegradation(false); }

		// Apply any in-flight orientation shift, if applicable
		proj->SetOrientationShift(m_launchwithorientchange);
		if (m_launchwithorientchange) proj->SetOrientationShiftAmount(m_projectileorientchange);

		// Set collision exclusions with the "owner" object, which will then be removed a short time after launch
		proj->AddCollisionExclusion(m_parent->GetID());
		m_parent->AddCollisionExclusion(proj->GetID());

		// Finally, move into the world and return a pointer to the new projectile
		proj->MoveIntoSpaceEnvironment(m_parent->GetSpaceEnvironment(), proj->GetPosition());
		return proj;
	}

	// We should never reach this point, unless in error
	return NULL;
}

// Sets the degree of spread applied to projectiles (in radians).  Applies in both local yaw and pitch dimensions.  
// Measured from origin, so 'spread' is the radius of deviation in radians from actual orientation
void ProjectileLauncher::SetProjectileSpread(float s)
{
	// Store the new value
	m_spread = s;

	// Expand out the frand_lh macro to save repeated calculations: frand_lh(l,h) = (l + rand()/(RAND_MAX/(h-l)))
	// We can precalc the second divisor here for efficiency [ (RAND_MAX / (h-l) ]
	if (fabs(m_spread) > Game::C_EPSILON)
		m_spread_divisor = (float)RAND_MAX / (m_spread + m_spread);	// (m_sp - (-m_sp)) == (m_sp + m_sp)
	else
	{
		m_spread_divisor = -1.0f;
		m_spread_delta = ID_QUATERNION;
	}
}

// Sets the minimum launch interval (ms) for projectiles from this launcher
void ProjectileLauncher::SetLaunchInterval(unsigned int interval_ms)
{
	// Ensure there is always a minimum of 1ms interval, otherwise projectiles could be 
	// launched every cycle and overwhelm the simulation
	m_launchinterval = max(1U, interval_ms);
}

// Determines a new delta trajectory for the next projectile, based upon projectile spread properties
void ProjectileLauncher::DetermineNextProjectileSpreadDelta(void)
{
	// Get random spread values in each of the pitch & yaw dimensions
	// Expand out the frand_lh macro to save repeated calculations: frand_lh(l,h) = (l + rand()/(RAND_MAX/(h-l)))
	// Divisor has been pre-calculated any time the spread value is changed
	// We will construct a quaternion to account for the resulting turret pitch/yaw spread
	m_spread_delta = XMQuaternionRotationRollPitchYaw(	(-m_spread + ((float)rand() / m_spread_divisor)),
														(-m_spread + ((float)rand() / m_spread_divisor)),
														0.0f);
}

// Determines the max range of the launcher based on its launch properties and projectiles.  Is only
// an approximation since the projectiles may have linear velocity degradation or in-flight orientation 
// changes that we cannot simulate accurately here (without actually firing a projectile)
float ProjectileLauncher::DetermineApproxRange(void) const
{
	float range;

	// We can't do much without information on the projectiles we will be firing
	if (!m_projectiledef) return 0.0f;

	// Initial estimate is based upon the launch impulse, and represents the range PER SECOND
	if (m_launchmethod == ProjectileLauncher::ProjectileLaunchMethod::ApplyForce)
	{
		range = (m_launchimpulse / m_projectiledef->GetMass());		// a = F/m
	}
	else
	{
		range = m_launchimpulse;
	}

	// Scale the range/sec by the maximum projectile lifetime, assuming no collisions/deviations
	float lifetime = m_projectiledef->GetDefaultLifetime();
	range *= lifetime;

	// Account for linear velocity degradation in the estimate 
	if (m_degradelinearvelocity)
	{
		// Velocity degrades at 'm_linveldegradation' percent per second, so we will multiply by (1-m_linveldegradation)^lifetime
		range *= pow((1.0f - m_linveldegradation), lifetime);
	}

	// Return our best estimate of the launcher range
	return range;
}

// Copy all launcher data from the specified source object
void ProjectileLauncher::CopyFrom(const ProjectileLauncher *source)
{
	// Parameter check
	if (!source) return;

	// Copy all required data from the source object (no instance-specific data like 'parent' or relative pos/orient)
	SetCode(source->GetCode());
	SetName(source->GetName());
	SetProjectileDefinition(source->GetProjectileDefinition());
	SetLaunchMethod(source->GetLaunchMethod());
	SetProjectileSpread(source->GetProjectileSpread());
	SetLaunchInterval(source->GetLaunchInterval());
	SetLaunchImpulse(source->GetLaunchImpulse());
	SetProjectileOrientationChange(source->GetProjectileOrientationChange());
	SetLaunchAngularVelocity(source->GetLaunchAngularVelocity());
	SetLinearVelocityDegradeState(source->LinearVelocityDegrades());
	SetLinearVelocityDegradeRate(source->GetLinearVelocityDegradeRate());
	SetAngularVelocityDegradeState(source->AngularVelocityDegrades());
	SetAngularVelocityDegradeRate(source->GetAngularVelocityDegradeRate());
	SetProjectileSpread(source->GetProjectileSpread());
}

// Static method to translate launch method from its string representation
ProjectileLauncher::ProjectileLaunchMethod ProjectileLauncher::TranslateLaunchMethodFromString(std::string method)
{
	StrLowerC(method);

	if (method == "setvelocitydirect")				return ProjectileLauncher::ProjectileLaunchMethod::SetVelocityDirect;
	else											return ProjectileLauncher::ProjectileLaunchMethod::ApplyForce;
}

// Static method to translate launch method to its string representation
std::string ProjectileLauncher::TranslateLaunchMethodToString(ProjectileLauncher::ProjectileLaunchMethod method)
{
	switch (method)
	{
		case ProjectileLauncher::ProjectileLaunchMethod::SetVelocityDirect:		return "setvelocitydirect";
		default:																		return "applyforce";
	}
}




