#include "FastMath.h"
#include "SpaceProjectile.h"
#include "SpaceProjectileDefinition.h"

#include "SpaceProjectileLauncher.h"


// Default constructor; sets all fields to default values
SpaceProjectileLauncher::SpaceProjectileLauncher(void) : 
	m_projectiledef(NULL), m_parent(NULL), m_relativepos(NULL_VECTOR), m_relativeorient(ID_QUATERNION), 
	m_launchmethod(ProjectileLaunchMethod::ApplyForce), m_launchimpulse(250.0f), m_degradelinearvelocity(true), 
	m_linveldegradation(0.1f), m_launchwithangvel(false), m_launchangularvelocity(NULL_VECTOR), 
	m_degradeangularvelocity(false), m_angveldegradation(0.0f), m_launchwithorientchange(false), 
	m_projectileorientchange(ID_QUATERNION), m_spread(0.0f), m_spread_delta(ID_QUATERNION), m_spread_divisor(0.0f),
	m_launchinterval(1000U), m_nextlaunch(0U)
{
	
}

// Launches a projectile.  Returns a pointer to the projectile object that was launched, if applicable.  Will 
// fire even if not ready (i.e. within reload interval), so CanLaunchProjectile() should be checked before firing
// Accepts a the position and orientation of the parent launch point as an input.  Returns a reference to the 
// projectile that was fired, or NULL if nothing was launched
SpaceProjectile *SpaceProjectileLauncher::LaunchProjectile(const D3DXVECTOR3 & launchpoint, const D3DXQUATERNION & launchorient)
{
	// Validate required properties
	if (!m_projectiledef || !m_parent || !m_parent->GetSpaceEnvironment()) return NULL;

	// Attempt to create a new projectile from the definition; return NULL if we cannot create one for any reason
	SpaceProjectile *proj = m_projectiledef->CreateProjectile();
	if (!proj) return NULL;

	// Determine launch position based upon our parent object
	D3DXVECTOR3 pos;
	D3DXVec3Rotate(&pos, &m_relativepos, &launchorient);
	pos += launchpoint;
	proj->SetPosition(pos);

	// Determine the spread effect for this projectile.  Set the projectile orientation depending on whether
	// any spready should be applied or not
	if (m_spread > Game::C_EPSILON)
	{
		DetermineNextProjectileSpreadDelta();
		proj->SetOrientation(m_spread_delta * m_relativeorient * launchorient);
	}
	else
	{
		proj->SetOrientation(m_relativeorient * launchorient);
	}

	// Perform an immediate refresh of projectile position/orientation to recalcualate its transform matrices
	proj->RefreshPositionImmediate();

	// Apply launch impulse to the projectile
	float impulse = (m_launchmethod == ProjectileLaunchMethod::ApplyForce ? (m_launchimpulse * proj->GetInverseMass()) : m_launchimpulse);
	impulse = clamp(impulse, 0.01f, Game::C_PROJECTILE_VELOCITY_LIMIT);
	proj->ApplyLocalLinearForceDirect(D3DXVECTOR3(0.0f, 0.0f, impulse));

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

	// Update the timestamp at which we will be ready to launch our next projectile
	m_nextlaunch = (Game::ClockMs + m_launchinterval);

	// Finally, move into the world and return a pointer to the new projectile
	proj->MoveIntoSpaceEnvironment(m_parent->GetSpaceEnvironment(), proj->GetPosition());
	return proj;
}

// Sets the degree of spread applied to projectiles (in radians).  Applies in both local yaw and pitch dimensions.  
// Measured from origin, so 'spread' is the radius of deviation in radians from actual orientation
void SpaceProjectileLauncher::SetProjectileSpread(float s)
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
void SpaceProjectileLauncher::SetLaunchInterval(unsigned int interval_ms)
{
	// Ensure there is always a minimum of 1ms interval, otherwise projectiles could be 
	// launched every cycle and overwhelm the simulation
	m_launchinterval = max(1U, interval_ms);
}

// Determines a new delta trajectory for the next projectile, based upon projectile spread properties
void SpaceProjectileLauncher::DetermineNextProjectileSpreadDelta(void)
{
	// Get random spread values in each of the pitch & yaw dimensions
	// Expand out the frand_lh macro to save repeated calculations: frand_lh(l,h) = (l + rand()/(RAND_MAX/(h-l)))
	// Divisor has been pre-calculated any time the spread value is changed
	// We will construct a quaternion to account for the resulting turret pitch/yaw spread
	D3DXQuaternionRotationYawPitchRoll(&m_spread_delta, 
		(-m_spread + ((float)rand() / m_spread_divisor)), 
		(-m_spread + ((float)rand() / m_spread_divisor)), 
		0.0f);
}

// Determines the max range of the launcher based on its launch properties and projectiles.  Is only
// an approximation since the projectiles may have linear velocity degradation or in-flight orientation 
// changes that we cannot simulate accurately here (without actually firing a projectile)
float SpaceProjectileLauncher::DetermineApproxRange(void) const
{
	float range;

	// We can't do much without information on the projectiles we will be firing
	if (!m_projectiledef) return 0.0f;

	// Initial estimate is based upon the launch impulse, and represents the range PER SECOND
	if (m_launchmethod == SpaceProjectileLauncher::ProjectileLaunchMethod::ApplyForce)
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



