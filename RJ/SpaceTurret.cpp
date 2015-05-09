#include "DX11_Core.h"
#include "Utility.h"
#include "FastMath.h"
#include "SimulationObjectManager.h"
#include "SpaceProjectile.h"
#include "SpaceProjectileLauncher.h"

#include "SpaceTurret.h"


// Default constructor
SpaceTurret::SpaceTurret(void)
{
	// Set default values
	m_parent = NULL;
	m_launchers = NULL; m_launchercount = 0; m_launcherubound = 0;
	m_nextlauncher = 0;
	m_target = m_designatedtarget = NULL;
	m_relativepos = NULL_VECTOR;
	m_baserelativeorient = m_relativeorient = ID_QUATERNION;
	m_yaw = m_pitch = 0.0f;
	m_yawrate = 0.5f;
	m_pitchrate = 0.25f;
	m_yaw_limited = false;
	m_yawmin = m_yawmax = 0.0f;
	m_pitchmin = -0.15f;
	m_pitchmax = +0.15f;
	m_maxrange = 1000.0f; m_maxrangesq = (1000.0f * 1000.0f);
	m_nexttargetanalysis = 0U;
}

// Primary simulation method for the turret.  Tracks towards targets and fires when possible.  Accepts
// a reference to an array of contacts in the immediate area; this cached array is used for greater
// efficiency when processing multiple turrets per object
void SpaceTurret::Update(std::vector<iSpaceObject*> enemy_contacts)
{
	// We will re-evaluate targets on a less frequent basis; check if we are ready to do so now
	if (Game::ClockMs >= m_nexttargetanalysis)
	{
		// Update the counter to prepare for the next analysis cycle
		m_nexttargetanalysis = (Game::ClockMs + TARGET_ANALYSIS_INTERVAL);

		// If our current target is null we want to check for new available targets
		if (m_target == NULL)
		{
			// If we have been designated a target then attempt to focus on it now
			if (m_designatedtarget != NULL)
			{
				// Only switch to this designated target if we are in range and have line-of-sight
				if (CanHitTarget(m_designatedtarget))
				{
					SetTarget(m_designatedtarget);
				}
				else
				{
					// We want to find a new target
					SetTarget(FindNewTarget(enemy_contacts));
				}
			}
			else
			{
				// We do not have a designated target, so locate one within the array of contacts
				SetTarget(FindNewTarget(enemy_contacts));
			}
		}
		else
		{
			// We have a target; however, if we can instead engage our designated target (if applicable) then choose it preferentially
			if (m_designatedtarget != NULL && CanHitTarget(m_designatedtarget))
			{
				SetTarget(m_designatedtarget);
			}
			else
			{
				// Otherwise, test to make sure we can still hit our current target, and select a new one if we cannot
				if (!CanHitTarget(m_target))
				{
					SetTarget(FindNewTarget(enemy_contacts));
				}
			}
		}
			
	}
}

// Fires a projectile from the turret
void SpaceTurret::Fire(void)
{
	// Request a projectile be fired from the active launcher.  If no projectile is fired we are likely
	// within the 'reload' interval and will simply return.  TODO: in future, may want to keep track of 
	// last projectile(s) fired?
	if (m_launchers[m_nextlauncher].LaunchProjectile() == NULL)
	{
		return;
	}
	
	// If this is a multi-launcher turret, set the next launcher to be fired
	if (m_launchercount != 1)
	{
		m_nextlauncher = max(++m_nextlauncher, m_launchercount);
	}

}

// Sets the relative orientation of the turret on its parent object.  This will be the 'resting' orientation
void SpaceTurret::SetBaseRelativeOrientation(const D3DXQUATERNION & orient)
{
	// Store the new base orientation
	m_baserelativeorient = orient;
}

// Yaws the turret by the specified angle.  Does not check the yaw limits; this is the responsibility
// of the calling function
void SpaceTurret::Yaw(float angle)
{
	// Update the yaw value
	m_yaw += angle;
	
	// Update the orientation of the turret
	D3DXQUATERNION delta;
	D3DXQuaternionRotationYawPitchRoll(&delta, m_yaw, m_pitch, 0.0f);
	m_relativeorient = (m_baserelativeorient * delta);
}

// Pitches the turret by the specified angle.  Does not check the pitch limits; this is the responsibility
// of the calling function
void SpaceTurret::Pitch(float angle)
{
	// Update the pitch value
	m_pitch += angle;

	// Update the orientation of the turret
	D3DXQUATERNION delta;
	D3DXQuaternionRotationYawPitchRoll(&delta, m_yaw, m_pitch, 0.0f);
	m_relativeorient = (m_baserelativeorient * delta);
}

// Determines the max range of the turret based on its component launchers & projectiles.  Is only
// an approximation since the projectiles may have linear velocity degradation or in-flight orientation 
// changes that we cannot simulate accurately here (without actually firing a projectile)
void SpaceTurret::DetermineApproxRange(void)
{
	// Make sure we have some launchers set up within the turret; assuming we do, take the max range of 
	// the first as our initial estimate
	if (m_launchercount < 1) { SetMaxRange(0.0f); return; }
	float maxr = m_launchers[0].DetermineApproxRange();

	// Consider any other launchers; if they have a lower max range, take that as the overall range
	// of the turret so that the turret will only select targets it can engage with all launchers
	for (int i = 1; i < m_launchercount; ++i)
	{
		float r = m_launchers[i].DetermineApproxRange();
		if (r < maxr) maxr = r;
	}

	// Set the maximum turret range
	SetMaxRange(maxr);
}

// Specify the yaw limits for this turret, in radians.  These only have an effect if the yaw limit flag is set
void SpaceTurret::SetYawLimits(float y_min, float y_max)
{
	// Store new values, and update the turret firing arc
	m_yawmin = y_min;
	m_yawmax = y_max;
	m_yaw_arc = FiringArc(y_min, y_max);
}

// Specify the pitch limits for this turret, in radians
void SpaceTurret::SetPitchLimits(float p_min, float p_max)
{
	// Store new values, and update the turret firing arc
	m_pitchmin = p_min;
	m_pitchmax = p_max;
	m_pitch_arc = FiringArc(p_min, p_max);
}


// Sets the current target for the turret
void SpaceTurret::SetTarget(iSpaceObject *target)
{
	m_target = target;
}

// Designates a target for the turret; will be engaged if possible, otherwise alternative targets
// will be engaged in the meantime
void SpaceTurret::DesignateTarget(iSpaceObject *target)
{
	m_designatedtarget = target;
}

// Returns a flag indicating whether the target is within the firing arc of this turret.  We do not 
// test range since turrets should only be passed potential targets that are within their range
bool SpaceTurret::CanHitTarget(iSpaceObject *target)
{
	// Get turret position in world space 
	D3DXVECTOR3 pos;
	D3DXVec3Rotate(&pos, &m_relativepos, &m_parent->GetOrientation());
	pos += m_parent->GetPosition();

	// Compose our base orientation with the parent object to get the turret resting orientation, 
	// then derive the inverse
	D3DXQUATERNION orient, invorient;
	orient = (m_parent->GetOrientation() * m_baserelativeorient);
	D3DXQuaternionInverse(&invorient, &orient);

	// Transform the target vector by this inverse orientation to get a target vector in local space
	D3DXVECTOR3 tgt_local;
	D3DXVec3Rotate(&tgt_local, &(target->GetPosition() - pos), &invorient);

	// Before testing the firing arcs, make sure this target is actually in range
	if (D3DXVec3LengthSq(&tgt_local) > m_maxrange) return false;

	// Now test whether the vector lies within both our yaw & pitch firing arcs; it must lie in both to be valid
	return ( (m_yaw_limited == false || m_yaw_arc.VectorWithinArc(D3DXVECTOR2(tgt_local.x, tgt_local.z))) &&
			 (							m_pitch_arc.VectorWithinArc(D3DXVECTOR2(tgt_local.x, tgt_local.y))) );
}

// Searches for a new target in the given vector of enemy contacts and returns the first valid one
iSpaceObject * SpaceTurret::FindNewTarget(std::vector<iSpaceObject*> & enemy_contacts)
{
	// Consider each candidate in turn
	iSpaceObject *obj;
	int n = enemy_contacts.size();
	for (int i = 0; i < n; ++i)
	{
		// Make sure the object is valid
		obj = enemy_contacts[i]; if (!obj) continue;

		// Make sure we are only targeting hostile objects
		// TODO: *** DO THIS *** >> continue;

		// Make sure the object is in range and within our firing arc
		if (!CanHitTarget(obj)) continue;

		// The object has passed all tests, so we can consider it a valid target
		return obj;
	}

	// We could not locate any valid targets
	return NULL;
}

// Allocates space for the required number of launcher objects
void SpaceTurret::InitialiseLaunchers(int launcher_count)
{
	// Make sure the requested launcher count is valid (and reasonable)
	if (launcher_count < 1 || launcher_count > 100) launcher_count = 1;

	// Deallocate any launchers that have currently been allocated, if applicable
	if (m_launchers != NULL)
	{
		SafeDeleteArray(m_launchers);
	}

	// Allocate new space for the launcher objects
	m_launchers = new SpaceProjectileLauncher[launcher_count];

	// Update the count of launcher objects, and reset the related parameters accordingly
	m_launchercount = launcher_count;
	m_launcherubound = (launcher_count - 1);
	m_nextlauncher = 0;
}

// Shut down the projectile object, deallocating all resources
void SpaceTurret::Shutdown(void)
{

}
