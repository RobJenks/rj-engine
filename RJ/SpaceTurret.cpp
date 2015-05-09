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
	m_relativeorient = ID_QUATERNION;
	m_yaw = m_pitch = 0.0f;
	m_yawrate = 0.5f;
	m_pitchrate = 0.25f;
	m_yaw_limited = false;
	m_yawmin = m_yawmax = 0.0f;
	m_pitchmin = -0.15f;
	m_pitchmax = +0.15f;
	m_baseyaw = m_basepitch = 0.0f;
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

// Returns a flag indicating whether the target is within the firing arc of this turret.  We do not 
// test range since turrets should only be passed potential targets that are within their range
bool SpaceTurret::CanHitTarget(iSpaceObject *target)
{
	*** DO THIS ***
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
