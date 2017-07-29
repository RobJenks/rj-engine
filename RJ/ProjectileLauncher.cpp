#include "FastMath.h"
#include "SpaceSystem.h"
#include "SpaceProjectile.h"
#include "BasicProjectileSet.h"
#include "BasicProjectileDefinition.h"
#include "SpaceProjectileDefinition.h"
#include "Ship.h"
#include "CoreEngine.h"
#include "AudioManager.h"

#include "ProjectileLauncher.h"


// Default constructor; sets all fields to default values
ProjectileLauncher::ProjectileLauncher(void) : 
	m_projectiletype(Projectile::ProjectileType::BasicProjectile), m_basicprojdef(NULL), m_projectiledef(NULL), 
	m_parent(NULL), m_parentturret(NULL), m_relativepos(NULL_VECTOR), m_relativeorient(ID_QUATERNION), 
	m_launchmethod(ProjectileLaunchMethod::ApplyForce), m_launchimpulse(250.0f), m_degradelinearvelocity(true), 
	m_linveldegradation(0.1f), m_launchwithangvel(false), m_launchangularvelocity(NULL_VECTOR), 
	m_degradeangularvelocity(false), m_angveldegradation(0.0f), m_launchwithorientchange(false), 
	m_projectileorientchange(ID_QUATERNION), m_spread(0.0f), m_spread_delta(ID_QUATERNION), m_spread_divisor(1.0f),
	m_launchinterval(1000U), m_nextlaunch(0U), m_launch_velocity(1.0f), 
	m_launchinterval_variance(0.0f), m_launchinterval_min(1000U), m_launchinterval_max(1000U)
{
	
}

// Set the projectile definition to be used by this launcher.  Also sets the projectiletype to match
void ProjectileLauncher::SetProjectileDefinition(const BasicProjectileDefinition *def)
{
	// Store the new definition and update our projectile type
	ChangeProjectileType(Projectile::ProjectileType::BasicProjectile);
	m_basicprojdef = def;

	// Recalculate data that may rely on this definition
	RecalculateLauncherStatistics();
}

// Set the projectile definition to be used by this launcher.  Also sets the projectiletype to match
void ProjectileLauncher::SetProjectileDefinition(const SpaceProjectileDefinition *def)
{
	ChangeProjectileType(Projectile::ProjectileType::SpaceProjectile);
	m_projectiledef = def;

	// Recalculate data that may rely on this definition
	RecalculateLauncherStatistics();
}

// Precalculates data based on the projectile launcher, projectile definitions and other factors
void ProjectileLauncher::RecalculateLauncherStatistics(void)
{
	// Certain statistics are dependent on projectile type
	if (m_projectiletype == Projectile::ProjectileType::BasicProjectile && m_basicprojdef)
	{
		m_launch_velocity = m_basicprojdef->Speed;
	}
	else if (m_projectiletype == Projectile::ProjectileType::SpaceProjectile && m_projectiledef)
	{
		// Create a temporary projectile object to help calculate launcher statistics
		SpaceProjectile *proj = m_projectiledef->CreateProjectile();
		if (proj)
		{
			m_launch_velocity = (m_launchmethod == ProjectileLaunchMethod::ApplyForce ? (m_launchimpulse * proj->GetInverseMass()) : m_launchimpulse);
			m_launch_velocity = clamp(m_launch_velocity, 0.01f, Game::C_PROJECTILE_VELOCITY_LIMIT);
		}
	}
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
	m_nextlaunch = CalculateNextLaunchTime();

	// Take different action depending on whether we are launching a basic or full projectile
	if (m_projectiletype == Projectile::ProjectileType::BasicProjectile)
	{
		// Create a new projectile from the stored definition
		if (!m_basicprojdef) return NULL;
		m_parent->GetSpaceEnvironment()->Projectiles.AddProjectile(m_basicprojdef, m_parent->GetID(), pos, orient, m_parent->PhysicsState.WorldMomentum);

		// Play any relevant audio effect
		if (m_basicprojdef->GetLaunchAudio().Exists())
		{
			// TODO: Need a better way to apply volume_modifier.  E.g. if this projectile is being launched in space, and we are 
			// in an environment, we should be using the AudioManager::ENV_SPACE_VOLUME_MODIFIER modifier in place of 1.0f
			XMFLOAT3 pos_f;
			XMStoreFloat3(&pos_f, pos);
			Game::Engine->GetAudioManager()->Create3DInstance(m_basicprojdef->GetLaunchAudio().AudioId, pos_f, m_basicprojdef->GetLaunchAudio().Volume, 1.0f);
		}

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
		proj->ApplyLocalLinearForceDirect(XMVectorSetZ(NULL_VECTOR, m_launch_velocity));

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

		// Play any relevant audio effect
		if (m_projectiledef->GetLaunchAudio().Exists())
		{
			// TODO: Need a better way to apply volume_modifier.  E.g. if this projectile is being launched in space, and we are 
			// in an environment, we should be using the AudioManager::ENV_SPACE_VOLUME_MODIFIER modifier in place of 1.0f
			XMFLOAT3 pos_f;
			XMStoreFloat3(&pos_f, pos);
			Game::Engine->GetAudioManager()->Create3DInstance(m_projectiledef->GetLaunchAudio().AudioId, pos_f, m_projectiledef->GetLaunchAudio().Volume, 1.0f);
		}

		// Move into the world, set full simulation state and return a pointer to the new projectile
		proj->SetSimulationState(iObject::ObjectSimulationState::FullSimulation); 
		proj->MoveIntoSpaceEnvironment(m_parent->GetSpaceEnvironment());
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
	m_spread = max(s, 0.0f);

	// Expand out the frand_lh macro to save repeated calculations: frand_lh(l,h) = (l + rand()/(RAND_MAX/(h-l)))
	// We can precalc the second divisor here for efficiency [ (RAND_MAX / (h-l) ]
	if (m_spread > Game::C_EPSILON)
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

	// Recalculate launch interval parameters after a change to the launcher properties
	RecalculateLaunchIntervalParameters();
}

// Sets the possible launch interval variance, as a percentage of the defined interval.  Half-open range [0.0-...]
void ProjectileLauncher::SetLaunchIntervalVariance(float variance)
{
	// Validate and store the parameter
	m_launchinterval_variance = max(0.0f, variance);

	// Recalculate launch interval parameters after a change to the launcher properties
	RecalculateLaunchIntervalParameters();
}

// Recalculate launch interval parameters after a change to the launcher properties
void ProjectileLauncher::RecalculateLaunchIntervalParameters(void)
{
	// Calculate min & max possible launch interval based on the defined variance
	unsigned int delta = (unsigned int)(m_launchinterval_variance * (float)m_launchinterval);
	m_launchinterval_min = (m_launchinterval - delta);
	m_launchinterval_max = (m_launchinterval + delta);

	// Ensure the range is valid
	m_launchinterval_min = max(1U, m_launchinterval_min);
	m_launchinterval_max = max(1U, m_launchinterval_max);
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
	float range = 0.0f;

	// Take different action depending on the projectile type
	if (m_projectiletype == Projectile::ProjectileType::BasicProjectile && m_basicprojdef)
	{
		// Basic projectiles have a constant defined velocity per second; multiply
		// by the maximum lifetime to get the maximum possible range (note: lifetime is in ms so /1000)
		range = m_basicprojdef->Speed * ((float)m_basicprojdef->Lifetime * 0.001f);
	}
	else if (m_projectiletype == Projectile::ProjectileType::SpaceProjectile && m_projectiledef)
	{
		// Initial estimate is based upon the launch impulse, and represents the range PER SECOND
		if (m_launchmethod == ProjectileLauncher::ProjectileLaunchMethod::ApplyForce)
			range = m_launchimpulse / max(m_projectiledef->GetMass(), 1.0f);
		else
			range = m_launchimpulse;

		// Scale the range/sec by the maximum projectile lifetime, assuming no collisions/deviations
		float lifetime = m_projectiledef->GetDefaultLifetime();
		range *= lifetime;

		// Account for linear velocity degradation in the estimate 
		if (m_degradelinearvelocity)
		{
			// Velocity degrades at 'm_linveldegradation' percent per second, so we will multiply by (1-m_linveldegradation)^lifetime
			range *= pow((1.0f - m_linveldegradation), lifetime);
		}
	}

	// Return our best estimate of the launcher range
	return range;
}

// Copy all launcher data from the specified source object
void ProjectileLauncher::CopyFrom(const ProjectileLauncher *source)
{
	// Parameter check
	if (!source) return;

	// Copy projectile definition from the source, which will also determine the turret projectile type
	if (source->GetProjectileType() == Projectile::ProjectileType::BasicProjectile)
		SetProjectileDefinition(source->GetBasicProjectileDefinition());
	else
		SetProjectileDefinition(source->GetProjectileDefinition());

	// Copy all other required data from the source object (no instance-specific data like 'parent' or relative pos/orient)
	SetCode(source->GetCode());
	SetName(source->GetName());
	SetLaunchMethod(source->GetLaunchMethod());
	SetProjectileSpread(source->GetProjectileSpread());
	SetLaunchInterval(source->GetLaunchInterval());
	SetLaunchIntervalVariance(source->GetLaunchIntervalVariance());
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




