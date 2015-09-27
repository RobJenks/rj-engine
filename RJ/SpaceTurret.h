#pragma once

#ifndef __SpaceTurretH__
#define __SpaceTurretH__

#include "DX11_Core.h"
#include "FiringArc.h"
class iSpaceObject;
class SpaceProjectileLauncher;

class SpaceTurret
{
public:

	// Default constructor
	SpaceTurret(void);

	// Primary simulation method for the turret.  Tracks towards targets and fires when possible.  Accepts
	// a reference to an array of ENEMY contacts in the immediate area; this cached array is used for greater
	// efficiency when processing multiple turrets per object.  Array should be filtered by the parent
	// before passing it, and also sorted to prioritise targets if required.  Turret will select the first 
	// target in the vector that it can engage
	void							Update(std::vector<iSpaceObject*> & enemy_contacts);

	// Analyse all potential targets in the area and change target if necessary/preferred
	void							EvaluateTargets(std::vector<iSpaceObject*> & enemy_contacts);

	// Fires a projectile from the turret
	void							Fire(void);

	// Gets/sets the parent object to this turret
	CMPINLINE iSpaceObject *		GetParent(void) const									{ return m_parent; }
	void							SetParent(iSpaceObject *parent);

	// Retrieve or return a pointer to the articulated model for this turret
	CMPINLINE ArticulatedModel *	GetArticulatedModel(void)								{ return m_articulatedmodel; }
	CMPINLINE void					SetArticulatedModel(ArticulatedModel *model)			{ m_articulatedmodel = model; }

	// Gets/sets the relative position of the turret on its parent object
	CMPINLINE D3DXVECTOR3			GetRelativePosition(void) const							{ return m_relativepos; }
	CMPINLINE void					SetRelativePosition(const D3DXVECTOR3 & relativepos)	{ m_relativepos = relativepos; }

	// Gets/sets the relative orientation of the turret on its parent object.  This will be the 'resting' orientation
	CMPINLINE D3DXQUATERNION		GetBaseRelativeOrientation(void) const						{ return m_baserelativeorient; }
	void							SetBaseRelativeOrientation(const D3DXQUATERNION & orient);

	// Specifies whether the turret is yaw-limited, or can rotate freely about its local up axis
	CMPINLINE bool					IsYawLimited(void) const								{ return m_yaw_limited; }
	CMPINLINE void					SetYawLimitFlag(bool limit)								{ m_yaw_limited = limit; }

	// Specify the yaw limits for this turret, in radians.  These only have an effect if the yaw limit flag is set
	CMPINLINE float					GetMinYawLimit(void) const								{ return m_yawmin; }
	CMPINLINE float					GetMaxYawLimit(void) const								{ return m_yawmin; }
	void							SetYawLimits(float y_min, float y_max);

	// Specify the pitch limits for this turret, in radians
	CMPINLINE float					GetMinPitchLimit(void) const								{ return m_pitchmin; }
	CMPINLINE float					GetMaxPitchLimit(void) const								{ return m_pitchmin; }
	void							SetPitchLimits(float p_min, float p_max);

	// Sets or retrieves the max yaw/pitch rate of the turret, in radians/sec
	CMPINLINE float					GetYawRate(void) const										{ return m_yawrate; }
	CMPINLINE float					GetPitchRate(void) const									{ return m_pitchrate; }
	CMPINLINE void					SetYawRate(float rps)										{ m_yawrate = rps; }
	CMPINLINE void					SetPitchRate(float rps)										{ m_pitchrate = rps; }

	// Retrieve or manually set the max range of the turret
	CMPINLINE float					GetMaxRange(void) const										{ return m_maxrange; }
	CMPINLINE void					SetMaxRange(float r)
	{
		m_maxrange = r;
		m_maxrangesq = (r * r);
	}


	// Determines the max range of the turret based on its component launchers & projectiles.  Is only
	// an approximation since the projectiles may have linear velocity degradation or in-flight orientation 
	// changes that we cannot simulate accurately here (without actually firing a projectile)
	void							DetermineApproxRange(void);

	// Yaws or pitches the turret by the specified angle
	void							Yaw(float angle);
	void							Pitch(float angle);

	// Sets the current target for the turret
	void							SetTarget(iSpaceObject *target);

	// Designates a target for the turret; will be engaged if possible, otherwise alternative targets
	// will be engaged in the meantime
	void							DesignateTarget(iSpaceObject *target);

	// Returns a flag indicating whether the target is within range, and within the firing arc of this turret
	bool							CanHitTarget(iSpaceObject *target);

	// Searches for a new target in the given vector of enemy contacts and returns the first valid one
	iSpaceObject *					FindNewTarget(std::vector<iSpaceObject*> & enemy_contacts);

	// Allocates space for the required number of launcher objects
	void							InitialiseLaunchers(int launcher_count);

	// Retrieve a reference to one of the launchers within the turrent
	SpaceProjectileLauncher *		GetLauncher(int index);

	// Returns the number of launchers maintained within this turrent
	int								GetLauncherCount(void) const		{ return m_launchercount; }

	// Recalculates the object world matrix
	CMPINLINE void					DetermineWorldMatrix(void)
	{
		// Derive a new relative world matrix from the turret position & orientation
		D3DXMATRIX trans, rot;
		D3DXMatrixRotationQuaternion(&rot, &m_orientation);
		D3DXMatrixTranslation(&trans, m_position.x, m_position.y, m_position.z);
		m_worldmatrix = (rot * trans);
	}

	// Perform an update of the turret position and orientation in world space
	void							UpdatePositioning(void);

	// Retrieve the absolute spatial data for this turret, derived during update method
	CMPINLINE D3DXVECTOR3			GetPosition(void) const				{ return m_position; }
	CMPINLINE D3DXQUATERNION		GetOrientation(void) const			{ return m_orientation; }
	CMPINLINE D3DXQUATERNION		GetInverseOrientation(void) const	{ return m_invorient; }
	CMPINLINE const D3DXMATRIX *	GetWorldMatrix(void) const			{ return &m_worldmatrix; }

	// Shut down the turret object, deallocating all resources
	void							Shutdown(void);


protected:

	// Parent object that this turret is attached to
	iSpaceObject *					m_parent;

	// Articulated model for this turret
	ArticulatedModel *				m_articulatedmodel;

	// Array of projectile launchers attached to this turret
	SpaceProjectileLauncher *		m_launchers;				// Array of launcher objects arrached to the turret
	int								m_launchercount;			// The total count of launcher objects
	int								m_launcherubound;			// Efficiency measure; the upper bound of the launcher array.  (LauncherCount - 1)
	int								m_nextlauncher;				// The next launcher that will fire a projectile

	// Current target that the turret is tracking
	iSpaceObject *					m_target;

	// Target that the turret has been manually assigned, and which it will try to reacquire if it is lost
	iSpaceObject *					m_designatedtarget;

	// The times & intervals for when the turret was last updated
	static const unsigned int		TARGET_ANALYSIS_INTERVAL = 2000U;
	unsigned int					m_nexttargetanalysis;


	// Turrets have a relative position and orientation from their parent object.  The latter is the base
	// orientation of the turret, before any pitch or yaw is applied
	D3DXVECTOR3						m_relativepos;
	D3DXQUATERNION					m_baserelativeorient;
	D3DXQUATERNION					m_relativeorient;

	// Store world position, orientation and inverse orientation.  Derived during update by turret controller
	D3DXVECTOR3						m_position;
	D3DXQUATERNION					m_orientation, m_invorient;

	// Pitch and yaw of the turret, and resulting relative relative orientation.  Specified in the range [0 2PI]
	float							m_yaw, m_pitch;

	// Object world matrix, relative to its parent object
	D3DXMATRIX						m_worldmatrix;

	// Values indicating the yaw & pitch capabilities of the turret
	float							m_yawrate, m_pitchrate;							// Rad/sec
	bool							m_yaw_limited;									// Flag indicating whether the turret has yaw extents, or can rotate 360-deg
	float							m_yawmin, m_yawmax;								// Min/max yaw extents, radians.  Only required if m_yaw_limited == true
																					// Specified in the range [0 2PI]
	float							m_pitchmin, m_pitchmax;							// Min/max pitch extents, radians.  Specified in the range [0 2PI]

	// Turret maintains two unit circle arcs, in the yaw & pitch dimensions, to represent its firing cone
	FiringArc						m_yaw_arc, m_pitch_arc;

	// Turret has a maximum range
	float							m_maxrange, m_maxrangesq;

};



#endif