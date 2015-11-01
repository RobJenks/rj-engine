#include "DX11_Core.h"
#include "Utility.h"
#include "FastMath.h"
#include "SimulationObjectManager.h"
#include "ArticulatedModel.h"
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
	m_baserelativeorient = m_turretrelativeorient = ID_QUATERNION;
	m_position = NULL_VECTOR;
	m_orientation = m_invorient = ID_QUATERNION;
	m_yaw = m_pitch = 0.0f;
	m_yawrate = 0.5f;
	m_pitchrate = 0.25f;
	m_yaw_limited = false;
	m_yawmin = m_yawmax = 0.0f;
	m_pitchmin = -0.15f;
	m_pitchmax = +0.15f;
	m_minrange = 10.0f; m_minrangesq = (10.0f * 10.0f);
	m_maxrange = 10000.0f; m_maxrangesq = (10000.0f * 10000.0f);
	m_nexttargetanalysis = 0U;
	m_constraint_yaw = m_constraint_pitch = m_component_cannon = 0;
	m_firedelay = 0U;
}

// Primary full-simulation method for the turret.  Tracks towards targets and fires when possible.  Accepts
// a reference to an array of contacts in the immediate area; this cached array is used for greater
// efficiency when processing multiple turrets per object
void SpaceTurret::Update(std::vector<iSpaceObject*> & enemy_contacts)
{
	// Update the turret position and orientation based on its parent
	UpdatePositioning();

	// We will re-evaluate targets on a less frequent basis; check if we are ready to do so now
	if (Game::ClockMs >= m_nexttargetanalysis)
	{
		// Update the counter to prepare for the next analysis cycle
		m_nexttargetanalysis = (Game::ClockMs + TARGET_ANALYSIS_INTERVAL);

		// Evaluate all available targets and select one if possible and desirable
		EvaluateTargets(enemy_contacts);			
	}

	// Track towards the target if we have one, and if needed
	if (m_target)
	{
		// Determine any pitch/yaw required to keep the target in view (TODO: target leading)
		float yaw, pitch; D3DXQUATERNION invorient;
		D3DXQuaternionInverse(&invorient, &(m_turretrelativeorient * m_parent->GetOrientation()));
		DetermineYawAndPitchToTarget(CannonPosition(), invorient, m_target->GetPosition(), yaw, pitch);

		// If pitch and yaw are very close to target, we can begin firing
		float ayaw = fabs(yaw), apitch = fabs(pitch);
		if (ayaw < Game::C_DEFAULT_FIRING_CIRCLE_THRESHOLD && apitch < Game::C_DEFAULT_FIRING_CIRCLE_THRESHOLD)
		{
			// We are within the firing threshold.  This will only fire if the turret is ready to do so
			Fire();
		}
		
		// Attempt to yaw and pitch to keep the target in view
		if (ayaw > Game::C_EPSILON)		Yaw(yaw * m_yawrate * Game::TimeFactor);
		if (apitch > Game::C_EPSILON)	Pitch(pitch * m_pitchrate * Game::TimeFactor);
	}
}

// Analyse all potential targets in the area and change target if necessary/preferred
void SpaceTurret::EvaluateTargets(std::vector<iSpaceObject*> & enemy_contacts)
{
	// If there are no enemy contacts nearby then we know that no target can be possible
	if (enemy_contacts.size() == 0)
	{
		m_target = NULL;
		return;
	}

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
	else	/* if m_target != NULL */
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

// Force new target analysis next frame
void SpaceTurret::ForceNewTargetAnalysis(void)							
{ 
	m_nexttargetanalysis = 0U;
}

// Set the articulated model to be used by this turret.  Performs validation; if the model is not suitable, 
// returns an errorcode and the model is defaulted to NULL
Result SpaceTurret::SetArticulatedModel(ArticulatedModel *model)
{
	// If the model is NULL then simply set it and return success
	if (!model)
	{
		m_articulatedmodel = NULL;
		return ErrorCodes::NoError;
	}

	// Otherwise, make sure this model has the required model tags to identify important turret components
	int yaw = model->GetConstraintWithTag("turret_yaw");
	int pitch = model->GetConstraintWithTag("turret_pitch");
	int cannon = model->GetComponentWithTag("turret_cannon");
	if (yaw < 0 || pitch < 0 || cannon < 0)
	{
		m_articulatedmodel = NULL;
		return ErrorCodes::TurretModelDoesNotContainRequiredModelTags;
	}

	// Set the model, and store the yaw and pitch contraint indices
	m_articulatedmodel = model;
	m_constraint_yaw = yaw;
	m_constraint_pitch = pitch;
	m_component_cannon = cannon;

	// Return success
	return ErrorCodes::NoError;
}

// Fires a projectile from the turret
void SpaceTurret::Fire(void)
{
	// Check if a projectile can be fired from the active launcher.  If no projectile can be 
	// fired we are likely within the 'reload' interval and will simply return.  
	SpaceProjectileLauncher & launcher = m_launchers[m_nextlauncher];
	if (!launcher.CanLaunchProjectile()) return;
	
	// We only need to determine the turret firing position if we are ready to fire a projectile
	// We use the position and orientation of the model component tagged as "turret_cannon", and each
	// launcher holds an offset to be applied from that parent component
	ArticulatedModelComponent *cannon = m_articulatedmodel->GetComponent(m_component_cannon); 
	if (!cannon) return;	

	// Launch a new projectile.  Use turret (rather than cannon-component) orientation in world space
	// TODO: in future, may want to keep track of last projectile(s) fired?
	launcher.LaunchProjectile(cannon->GetPosition(), (m_turretrelativeorient * m_parent->GetOrientation()));

	// If this is a multi-launcher turret, set the next launcher to be fired
	if (m_launchercount != 1)
	{
		// Update the index of the next launcher
		if (++m_nextlauncher >= m_launchercount) m_nextlauncher = 0;

		// Make sure the time to next firing is at least the turret inter-launcher fire delay, at minimum
		unsigned int turretready = (Game::ClockMs + m_firedelay);
		if (m_launchers[m_nextlauncher].NextAvailableLaunch() < turretready)
		{
			m_launchers[m_nextlauncher].SetNextAvailableLaunch(turretready);
		}
	}

}

// Sets the parent object to this turret
void SpaceTurret::SetParent(iSpaceObject *parent)
{
	// Store the parent reference
	m_parent = parent;

	// Also update the parent object of all launchers within this turret
	for (int i = 0; i < m_launchercount; ++i)
		m_launchers[i].SetParent(parent);
}


// Sets the relative orientation of the turret on its parent object.  This will be the 'resting' orientation
void SpaceTurret::SetBaseRelativeOrientation(const D3DXQUATERNION & orient)
{
	// Store the new base orientation
	m_baserelativeorient = orient;
}

// Yaws the turret by the specified angle
void SpaceTurret::Yaw(float angle)
{
	// Determine the new yaw value that would result
	//float yaw = fmod((m_yaw + angle), TWOPI);
	float yaw = (m_yaw + angle);
	if (yaw > TWOPI)		yaw -= TWOPI;
	else if (yaw < -TWOPI)	yaw += TWOPI;

	// If this turret has a yaw limit, validate that we are allowed to execute the move
	if (m_yaw_limited)
	{
		// Limit to the yaw bounds
		if		(yaw < m_yawmin)	yaw = m_yawmin;
		else if (yaw > m_yawmax)	yaw = m_yawmax;
		
		// Early-exit if this means we can't make any further movement
		if (fabs(m_yaw - yaw) < Game::C_EPSILON) return;
	}

	// Update the yaw value
	m_yaw = yaw;
	
	// Update the orientation of the turret cannon
	D3DXQUATERNION delta;
	D3DXQuaternionRotationYawPitchRoll(&delta, m_yaw, m_pitch, 0.0f);
	m_turretrelativeorient = (m_baserelativeorient * delta);

	// Update the model rotation about its yaw constraint
	m_articulatedmodel->SetConstraintRotation(m_constraint_yaw, m_yaw);
}

// Pitches the turret by the specified angle
void SpaceTurret::Pitch(float angle)
{
	// Determine the new pitch value that would result
	float pitch = fmod((m_pitch + angle), TWOPI);

	// Limit to the turrent pitch bounds
	if		(pitch < m_pitchmin)	pitch = m_pitchmin;
	else if (pitch > m_pitchmax)	pitch = m_pitchmax;

	// Early-exit if this means we can't make any further movement
	if (fabs(m_pitch - pitch) < Game::C_EPSILON) return;
	
	// Update the yaw value
	m_pitch = pitch;

	// Update the orientation of the turret cannon
	D3DXQUATERNION delta;
	D3DXQuaternionRotationYawPitchRoll(&delta, m_yaw, m_pitch, 0.0f);
	m_turretrelativeorient = (m_baserelativeorient * delta);

	// Update the model rotation about its pitch constraint
	m_articulatedmodel->SetConstraintRotation(m_constraint_pitch, m_pitch);
}

// Reset the orientation of the turret back to its base (instantly)
void SpaceTurret::ResetOrientation(void)
{
	// Set both yaw and pitch back to starting positions
	m_yaw = m_pitch = 0.0f;

	// Reset the turret to its base orientation (which is equivalent to yaw/pitch of zero)
	m_turretrelativeorient = m_baserelativeorient;
}

// Determines the max range of the turret based on its component launchers & projectiles.  Is only
// an approximation since the projectiles may have linear velocity degradation or in-flight orientation 
// changes that we cannot simulate accurately here (without actually firing a projectile)
void SpaceTurret::DetermineApproxRange(void)
{
	// Make sure we have some launchers set up within the turret; assuming we do, take the max range of 
	// the first as our initial estimate
	if (m_launchercount < 1) { SetRange(1.0f, 2.0f); return; }
	float maxr = m_launchers[0].DetermineApproxRange();

	// Consider any other launchers; if they have a lower max range, take that as the overall range
	// of the turret so that the turret will only select targets it can engage with all launchers
	for (int i = 1; i < m_launchercount; ++i)
	{
		float r = m_launchers[i].DetermineApproxRange();
		if (r < maxr) maxr = r;
	}

	// Set the maximum turret range (TODO: NEED TO INCORPORATE MINIMUM RANGE OF EACH LAUNCHER  HERE TOO)
	SetRange(1.0f, maxr);
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

// Perform an update of the turret position and orientation in world space.  This is needed for full simulation (even
// if not being rendered) since it will calculate the position of the cannon components in world space.  When not in 
// full simulation we can likely approximate as only ship position/orientation
void SpaceTurret::UpdatePositioning(void)
{
	// Determine turret position in world space 
	XMVector3Rotate(&m_position, &m_relativepos, &m_parent->GetOrientation());
	m_position += m_parent->GetPosition();

	// Compose our base orientation with the parent object to get the turret resting orientation, 
	// then derive the inverse
	m_orientation = (m_baserelativeorient * m_parent->GetOrientation());
	D3DXQuaternionInverse(&m_invorient, &m_orientation);
}

// Returns a flag indicating whether the target is within the firing arc of this turret.  We do not 
// test range since turrets should only be passed potential targets that are within their range
bool SpaceTurret::CanHitTarget(iSpaceObject *target)
{ 
	// Transform the target vector by turret inverse orientation to get a target vector in local space
	D3DXVECTOR3 tgt_local;
	XMVector3Rotate(&tgt_local, &(target->GetPosition() - m_position), &m_invorient);

	// Before testing the firing arcs, make sure this target is actually in range
	if (D3DXVec3LengthSq(&tgt_local) > m_maxrangesq) return false;

	// Now test whether the vector lies within both our yaw & pitch firing arcs; it must lie in both to be valid
	return ( (m_yaw_limited == false || m_yaw_arc.VectorWithinArc(D3DXVECTOR2(tgt_local.x, tgt_local.z))) &&
			 (							m_pitch_arc.VectorWithinArc(D3DXVECTOR2(tgt_local.y, tgt_local.z))) );
}

// Searches for a new target in the given vector of enemy contacts and returns the first valid one
iSpaceObject * SpaceTurret::FindNewTarget(std::vector<iSpaceObject*> & enemy_contacts)
{
	// Make sure we have required data
	if (!m_parent) return NULL;

	// Consider each candidate in turn
	iSpaceObject *obj;
	std::vector<iSpaceObject*>::size_type n = enemy_contacts.size();
	for (std::vector<iSpaceObject*>::size_type i = 0; i < n; ++i)
	{
		// Make sure the object is valid
		obj = enemy_contacts[i]; if (!obj) continue;

		// Make sure we are only targeting hostile objects
		if (m_parent->GetDispositionTowardsObject(obj) != Faction::FactionDisposition::Hostile) continue;

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
	
	// Set any default properties for the launchers on creation
	for (int i = 0; i < m_launchercount; ++i)
	{
		m_launchers[i].SetParent(m_parent);
	}
}

// Recalculates all turret statistics based on contents; used post-initialisation to prepare turret for use
void SpaceTurret::RecalculateTurretStatistics(void)
{
	// Determine approximate range of the turret based on all launcher data
	DetermineApproxRange();

	// Initialise the firing readiness for each launcher
	for (int i = 0; i < m_launchercount; ++i)
	{
		m_launchers[i].ForceReload();
	}
}

// Retrieve a reference to one of the launchers within the turrent
SpaceProjectileLauncher * SpaceTurret::GetLauncher(int index)
{
	if (index < 0 || index > m_launcherubound)	return NULL;
	else										return &(m_launchers[index]);
}

// Sets a launcher to the specified object
Result SpaceTurret::SetLauncher(int index, const SpaceProjectileLauncher *launcher)
{
	// Parameter checks
	if (index < 0 || index > m_launcherubound || !launcher)	return ErrorCodes::CannotSetTurretLauncherDefinition;

	// Assign the new launcher object and return success
	m_launchers[index].CopyFrom(launcher);
	return ErrorCodes::NoError;
}


// Clears the reference to all turret launcher data; used during object clone to allow deep-copy of launcher data
void SpaceTurret::ClearLauncherReferences(void)
{
	// Reset all launcher data without deallocating it (required when cloning turret data)
	m_launchers = NULL;
	m_launchercount = m_launcherubound = m_nextlauncher = 0;
}

// Shut down the projectile object, deallocating all resources
void SpaceTurret::Shutdown(void)
{

}

// Make and return a copy of this turret
SpaceTurret * SpaceTurret::Copy(void)
{
	// Create an initial shallow copy using the default copy constructor
	SpaceTurret *t = new SpaceTurret(*this);

	// Remove all launcher references and deep-copy the data instead
	t->ClearLauncherReferences();
	t->InitialiseLaunchers(m_launchercount);
	for (int i = 0; i < m_launchercount; ++i)
	{
		t->SetLauncher(i, &(m_launchers[i]));
	}

	// Make a copy of the articulated model data, assuming some exists (if not, return NULL since we must have a model)
	if (!m_articulatedmodel) { SafeDelete(t); return NULL; }
	t->SetArticulatedModel(m_articulatedmodel->Copy());

	// Remove references to parent, since this is instance-specific
	t->SetParent(NULL);

	// Reset any AI or simulation values that are instance-specific
	t->SetTarget(NULL);
	t->DesignateTarget(NULL);
	t->ForceNewTargetAnalysis();

	// Reset orientation of the new turret
	t->ResetOrientation();
	
	// Recalculate all turret statistics to prepare it for use
	t->RecalculateTurretStatistics();

	// Return the new turret object
	return t;
}

