#include "DX11_Core.h"
#include "Utility.h"
#include "FastMath.h"
#include "GameDataExtern.h"
#include "ArticulatedModel.h"
#include "SpaceProjectile.h"
#include "ProjectileLauncher.h"
#include "ObjectReference.h"
#include "Ship.h"

#include "SpaceTurret.h"


// Initialise static data
const SpaceTurret::TurretID SpaceTurret::NULL_TURRET = 0U;
SpaceTurret::TurretID SpaceTurret::TurretIDCounter = 0U;

// Default constructor
SpaceTurret::SpaceTurret(void)
{
	// Set default values
	m_parent = NULL;
	m_launchers = NULL; m_launchercount = 0; m_launcherubound = 0;
	m_nextlauncher = 0;
	m_mode = SpaceTurret::ControlMode::AutomaticControl;
	m_target = m_designatedtarget = NULL;
	m_relativepos = NULL_VECTOR;
	m_baserelativeorient = m_turretrelativeorient = ID_QUATERNION;
	m_cannonorient = m_invcannonorient = ID_QUATERNION;
	m_position = NULL_VECTOR;
	m_orientation = m_invorient = ID_QUATERNION;
	m_worldmatrix = ID_MATRIX;
	m_articulatedmodel = NULL;
	m_turretstatus = TurretStatus::Idle;
	m_atrest = true;
	m_isfixed = false;
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
	m_firing_spread_multiplier = Game::C_DEFAULT_FIRING_SPREAD_THRESHOLD;
	m_firing_region_threshold = Game::C_DEFAULT_FIRING_REGION_THRESHOLD;
	m_constraint_yaw = m_constraint_pitch = m_component_cannon = 0;
	m_firedelay = 0U;
}

// Static factory method to create new turret objects
SpaceTurret *SpaceTurret::Create(void)
{
	// Invoke the spawn function using a template ship corresponding to this definition (if one exists)
	// We need to assign a new unique ID directly since it is normally assigned via SpaceTurret::Copy()
	SpaceTurret *turret = new SpaceTurret();
	turret->AssignNewUniqueTurretID();
	return turret;
}

// Static factory method to create new turret objects
SpaceTurret *SpaceTurret::Create(const std::string & code)
{
	// Invoke the spawn function using a template ship corresponding to this definition (if one exists)
	return (SpaceTurret::Create(D::Turrets.Get(code)));
}

// Static factory method to create new turret objects
SpaceTurret *SpaceTurret::Create(SpaceTurret *template_turret)
{
	// If we are passed an invalid class pointer then return NULL immediately
	if (template_turret == NULL) return NULL;

	// Create a new instance of the turret from this template and return it
	return (template_turret->Copy());
}

// Full-simulation method for the turret when it is under ship computer control.  Tracks towards targets 
// and fires when possible.  Accepts a reference to an array of ENEMY contacts in the immediate area; 
// this cached array is used for greater efficiency when processing multiple turrets per object.  Array 
// should be filtered by the parent before passing it, and also sorted to prioritise targets if required.  
// Turret will select the first target in the vector that it can engage
void SpaceTurret::Update(std::vector<ObjectReference<iSpaceObject>> & enemy_contacts)
{
	// Update the turret position and orientation based on its parent
	UpdatePositioning();

	// Take different action depending on whether we are in manual or automatic mode
	if (m_mode == SpaceTurret::ControlMode::ManualControl)
	{
		// Update status
		m_turretstatus = TurretStatus::UnderManualControl;

		// (Move turret to point at user target)
	}
	else
	{
		// We are in automatic mode
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
			// Determine any pitch/yaw required to keep the target in view.  Implements
			// target leading ahead of the target object
			XMFLOAT2 pitch_yaw = DetermineCannonYawAndPitchToTarget(*m_target); 
			
			// If pitch and yaw are very close to target, we can begin firing
			float ayaw = fabs(pitch_yaw.y), apitch = fabs(pitch_yaw.x);
			if (ayaw < m_firing_region_threshold && apitch < m_firing_region_threshold)
			{
				// We are within the firing threshold.  This will only fire if the turret is ready to do so
				Fire();

				// Update status to indicate that the turret is engaging
				m_turretstatus = TurretStatus::EngagingTarget;
			}
			else
			{
				// We are tracking the target but do not yet have a shot
				m_turretstatus = TurretStatus::TrackingTarget;
			}
		
			// Attempt to yaw and pitch to keep the target in view
			if (ayaw > Game::C_EPSILON)		Yaw(pitch_yaw.y * m_yawrate * Game::TimeFactor);
			if (apitch > Game::C_EPSILON)	Pitch(pitch_yaw.x * m_pitchrate * Game::TimeFactor);
		}
		else
		{
			// We have no target; return the turret to resting position
			// If we are already at rest then there is nothing further to do
			if (fabs(m_yaw) < Game::C_EPSILON && fabs(m_pitch) < Game::C_EPSILON)
			{
				if (!m_atrest)
				{
					// If we have oriented back to ID and are not current flagged as 'at rest', set the flag now and quit
					ResetOrientation();
				}

				//  The turret is at rest
				m_turretstatus = TurretStatus::Idle;
			}
			else
			{
				// We are trying to rotate back to the resting position
				Yaw((m_yaw < 0.0f ? min(-m_yaw, m_yawrate) : -min(m_yaw, m_yawrate)) * Game::TimeFactor);
				Pitch((m_pitch < 0.0f ? min(-m_pitch, m_pitchrate) : -min(m_pitch, m_pitchrate)) * Game::TimeFactor);

				// Update status accordingly
				m_turretstatus = TurretStatus::ReturningToIdle;
			}
		}
	}
}

// Analyse all potential targets in the area and change target if necessary/preferred
void SpaceTurret::EvaluateTargets(std::vector<ObjectReference<iSpaceObject>> & enemy_contacts)
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
	Result result = SetArticulatedModelInternal(model);

	InstanceFlags.SetFlagState(InstanceFlags::INSTANCE_FLAG_SHADOW_CASTER, m_articulatedmodel != NULL);

	return result;
}

Result SpaceTurret::SetArticulatedModelInternal(ArticulatedModel *model)
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
	ProjectileLauncher & launcher = m_launchers[m_nextlauncher];
	if (!launcher.CanLaunchProjectile()) return;
	
	// We only need to determine the turret firing position if we are ready to fire a projectile
	// We use the position and orientation of the model component tagged as "turret_cannon", and each
	// launcher holds an offset to be applied from that parent component
	ArticulatedModelComponent *cannon = m_articulatedmodel->GetComponent(m_component_cannon); 
	if (!cannon) return;	

	// Launch a new projectile.  Use turret (rather than cannon-component) orientation in world space
	// TODO: in future, may want to keep track of last projectile(s) fired?
	launcher.LaunchProjectile(cannon->GetPosition(), XMQuaternionMultiply(m_turretrelativeorient, m_parent->GetOrientation()));

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
void SpaceTurret::SetParent(Ship *parent)
{
	// Store the parent reference
	m_parent = parent;

	// Also update the parent object of all launchers within this turret
	for (int i = 0; i < m_launchercount; ++i)
		m_launchers[i].SetParent(parent);
}


// Sets the relative orientation of the turret on its parent object.  This will be the 'resting' orientation
void SpaceTurret::SetBaseRelativeOrientation(const FXMVECTOR orient)
{
	// Store the new base orientation
	m_baserelativeorient = orient;

	// Reset orientation of the turret cannon given that we have just changed the base turret state
	ResetOrientation();
}

// Yaws the turret by the specified angle
void SpaceTurret::Yaw(float angle)
{
	// This will move us from our resting position
	m_atrest = false;

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
	
	// Update the orientation of the turret cannon (baseorient * pitch_yaw_delta)
	m_turretrelativeorient = XMQuaternionMultiply(	
		m_baserelativeorient, 
		XMQuaternionRotationRollPitchYaw(m_pitch, m_yaw, 0.0f));

	// Update the model rotation about its yaw constraint
	m_articulatedmodel->SetConstraintRotation(m_constraint_yaw, m_yaw);
}

// Pitches the turret by the specified angle
void SpaceTurret::Pitch(float angle)
{
	// This will move us from our resting position
	m_atrest = false;

	// Determine the new pitch value that would result
	float pitch = fmod((m_pitch + angle), TWOPI);

	// Limit to the turrent pitch bounds
	if		(pitch < m_pitchmin)	pitch = m_pitchmin;
	else if (pitch > m_pitchmax)	pitch = m_pitchmax;

	// Early-exit if this means we can't make any further movement
	if (fabs(m_pitch - pitch) < Game::C_EPSILON) return;
	
	// Update the yaw value
	m_pitch = pitch;

	// Update the orientation of the turret cannon (baseorient * pitch_yaw_delta)
	m_turretrelativeorient = XMQuaternionMultiply(
		m_baserelativeorient,
		XMQuaternionRotationRollPitchYaw(m_pitch, m_yaw, 0.0f));

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

	// Mark the turret as 'at rest' since it is at its base orientation
	m_atrest = true;
}

// Returns the pitch/yaw required to align the turret cannon with the specified target
XMFLOAT2 SpaceTurret::DetermineCannonYawAndPitchToTarget(const iActiveObject & target) const
{
	// Call the overloaded method with a target vector of (target_pos - cannon_pos)
	return DetermineCannonYawAndPitchToVector(XMVectorSubtract(m_parent->DetermineTargetLeadingPosition(target), CannonPosition()));
}

// Returns the pitch/yaw required to align the turret cannon with the specified vector
XMFLOAT2 SpaceTurret::DetermineCannonYawAndPitchToVector(const XMVECTOR target_vector) const
{
	return DetermineYawAndPitchToWorldVector(target_vector, m_invcannonorient);
}

// Determines whether the cannon is currently aligned to fire along the specified vector (with 
// tolerance specified by the turret 'firing region threshold')
bool SpaceTurret::IsCannonAlignedToVector(const FXMVECTOR target_vector) const
{
	// Calculate the pitch/yaw difference between our current cannon orientation
	// and the specified target vector
	XMFLOAT2 pitch_yaw = DetermineCannonYawAndPitchToVector(target_vector);

	// Test whether this differential is within the turret firing threshold
	return (fabs(pitch_yaw.y) < m_firing_region_threshold && fabs(pitch_yaw.x) < m_firing_region_threshold);
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

// Determines an appropriate firing region for the turret based on the accuracy of its component launchers, 
// plus (TODO:) any contribution from pilot skill, ship computer effectiveness etc.
void SpaceTurret::DetermineFiringRegion(void)
{
	// Use a default value if we are missing any required parameters
	if (m_launchercount == 0)
	{
		m_firing_region_threshold = Game::C_DEFAULT_FIRING_REGION_THRESHOLD;
		return;
	}

	// Iterate through the launcher collection and determine the max spread from any launcher
	float maxspread = 0.0f;
	for (int i = 0; i < m_launchercount; ++i)
	{
		if (m_launchers[i].GetProjectileSpread() > maxspread) maxspread = m_launchers[i].GetProjectileSpread();
	}

	/* Apply any other modifiers to the spread as required */

	// Apply the turret multiplier on launcher spread
	maxspread *= m_firing_spread_multiplier;

	// Operator/computer targeting skill etc...

	/* Store the adjusted figure as the radius of the acceptable firing region */
	m_firing_region_threshold = maxspread;
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
	// Determine turret position in world space (parentpos + rotate(relativepos))
	m_position = XMVectorAdd(m_parent->GetPosition(), XMVector3Rotate(m_relativepos, m_parent->GetOrientation()));

	// Compose our base orientation with the parent object to get the turret resting orientation, 
	// then derive the inverse
	m_orientation = XMQuaternionMultiply(m_baserelativeorient, m_parent->GetOrientation());
	m_invorient = XMQuaternionInverse(m_orientation);

	// Also determine the cannon orientation in world space & its inverse
	m_cannonorient = XMQuaternionMultiply(m_turretrelativeorient, m_parent->GetOrientation());
	m_invcannonorient = XMQuaternionInverse(m_cannonorient);
}

// Returns a flag indicating whether the target is within the firing arc of this turret.  We do not 
// test range since turrets should only be passed potential targets that are within their range
bool SpaceTurret::CanHitTarget(iSpaceObject *target)
{ 
	// First make sure the target exists
	if (!target) return false;

	// Check whether the target is in range
	if (!TargetIsInRange(target)) return false;

	// Check that this turret can maneuver to bring the target in its sights
	return (TargetIsWithinFiringArc(target));
}

// Returns the range to a specified target
float SpaceTurret::RangeSqToTarget(const iSpaceObject *target)
{
	return XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(target->GetPosition(), m_position)));
}

// Indicates whether a target at the specified range is valid
bool SpaceTurret::TargetRangeSqIsValid(float range_sq)
{
	return (range_sq <= m_maxrangesq && range_sq >= m_minrangesq);
}

// Indicates whether the specified target is in range
bool SpaceTurret::TargetIsInRange(const iSpaceObject *target)
{
	return TargetRangeSqIsValid(RangeSqToTarget(target));
}

// Indicates whether the specified target is within this turret's possible firing arc
bool SpaceTurret::TargetIsWithinFiringArc(const iSpaceObject *target)
{
	// Transform the target vector by turret inverse orientation to get a target vector in local space
	XMVECTOR tgt_local = XMVector3Rotate(
		XMVectorSubtract(target->GetPosition(), m_position),	// Get difference vector from turret to target
		m_invorient);											// Transform this target vector into local space

	// Test whether the vector lies within both our yaw & pitch firing arcs; it must lie in both to be valid
	return (
		(m_yaw_limited == false || m_yaw_arc.VectorWithinArc(XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Z>(tgt_local))) &&	// (x, z)
		(m_pitch_arc.VectorWithinArc(XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Z>(tgt_local))));	// (y, z)
}

// Searches for a new target in the given vector of enemy contacts and returns the first valid one
iSpaceObject * SpaceTurret::FindNewTarget(std::vector<ObjectReference<iSpaceObject>> & enemy_contacts)
{
	// Make sure we have required data
	if (!m_parent) return NULL;

	// Consider each candidate in turn
	iSpaceObject *obj; iSpaceObject *target = NULL; float target_dist_sq = 1e9;
	std::vector<ObjectReference<iSpaceObject>>::iterator it_end = enemy_contacts.end();
	for (std::vector<ObjectReference<iSpaceObject>>::iterator it = enemy_contacts.begin(); it != it_end; ++it)
	{
		// Make sure the object is valid
		obj = (*it)(); if (!obj) continue;

		// Make sure we are only targeting hostile objects
		if (m_parent->GetDispositionTowardsObject(obj) != Faction::FactionDisposition::Hostile) continue;

		/* Note: components of CanHitTarget() are called separately here for efficiency, and because we want to use the range information ourselves */

		// Make sure the target is in range.  Even if it is, reject the target if it is further away than a previous valid target
		float range_sq = RangeSqToTarget(obj);
		if (!TargetRangeSqIsValid(range_sq) || range_sq > target_dist_sq) continue;

		// Make sure the turret can bring this target to bear within its firing arc
		if (!TargetIsWithinFiringArc(obj)) continue;

		// The object has passed all tests so it is now our best potential target; record that fact and move to the next object
		target = obj;
		target_dist_sq = range_sq;
	}

	// Return the best target we could find, or NULL if none were suitable
	return target;
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
	m_launchers = new ProjectileLauncher[launcher_count];

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

	// Determine an appropriate firing region for the turret, based on accuracy of all launchers
	DetermineFiringRegion(); 

	// Initialise the firing readiness for each launcher and recalculate its statistics
	for (int i = 0; i < m_launchercount; ++i)
	{
		m_launchers[i].RecalculateLauncherStatistics();
		m_launchers[i].ForceReload();
	}
}

// Retrieve a reference to one of the launchers within the turrent
ProjectileLauncher * SpaceTurret::GetLauncher(int index)
{
	if (index < 0 || index > m_launcherubound)	return NULL;
	else										return &(m_launchers[index]);
}

// Sets a launcher to the specified object
Result SpaceTurret::SetLauncher(int index, const ProjectileLauncher *launcher)
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

	// Assign a new unique ID to this turret
	t->AssignNewUniqueTurretID();

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

// Static method to translate a turret status code to its string representation
std::string SpaceTurret::TranslateTurretStatusToString(SpaceTurret::TurretStatus status)
{
	switch (status)
	{
		case TurretStatus::Idle:					return "Idle";
		case TurretStatus::UnderManualControl:		return "Under Manual Control";
		case TurretStatus::TrackingTarget:			return "Tracking Target";
		case TurretStatus::EngagingTarget:			return "Engaging Target";
		case TurretStatus::ReturningToIdle:			return "Returning to Idle";
		default:									return "(Unknown)";
	}
}

