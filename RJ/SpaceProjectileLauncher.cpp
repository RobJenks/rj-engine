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
	m_projectileorientchange(ID_QUATERNION)
{
	
}

// Launches a projectile.  Returns a pointer to the projectile object that was launched, if applicable
SpaceProjectile * SpaceProjectileLauncher::LaunchProjectile(void)
{
	// Validate required properties
	if (!m_projectiledef || !m_parent || !m_parent->GetSpaceEnvironment()) return NULL;

	// Attempt to create a new projectile from the definition; return NULL if we cannot create one for any reason
	SpaceProjectile *proj = m_projectiledef->CreateProjectile();
	if (!proj) return NULL;

	// Determine launch position and orientation based upon our parent object
	D3DXVECTOR3 pos;
	D3DXVec3TransformCoord(&pos, &m_relativepos, m_parent->GetWorldMatrix());
	proj->SetPosition(pos);
	proj->SetOrientation(m_relativeorient * m_parent->GetOrientation());

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
		proj->ApplyAngularMomentum(m_launchangularvelocity);
		proj->SetAngularVelocityDegradation(m_degradeangularvelocity);
		if (m_degradeangularvelocity) proj->SetAngularVelocityDegradePc(m_angveldegradation);
	}
	else { proj->SetAngularVelocityDegradation(false); }
	
	// Apply any in-flight orientation shift, if applicable
	proj->SetOrientationShift(m_launchwithorientchange);
	if (m_launchwithorientchange) proj->SetOrientationShiftAmount(m_projectileorientchange);

	// Finally, move into the world and return a pointer to the new projectile
	proj->MoveIntoSpaceEnvironment(m_parent->GetSpaceEnvironment(), proj->GetPosition());
	return proj;
}



