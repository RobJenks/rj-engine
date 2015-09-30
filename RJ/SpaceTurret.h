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
	
	// Get/set the string name or code of this turret object
	CMPINLINE const std::string &	GetCode(void) const										{ return m_code; }
	CMPINLINE void					SetCode(const std::string & code)						{ m_code = code; }
	CMPINLINE const std::string &	GetName(void) const										{ return m_name; }
	CMPINLINE void					SetName(const std::string & name)						{ m_name = name; }

	// Primary simulation method for the turret.  Tracks towards targets and fires when possible.  Accepts
	// a reference to an array of ENEMY contacts in the immediate area; this cached array is used for greater
	// efficiency when processing multiple turrets per object.  Array should be filtered by the parent
	// before passing it, and also sorted to prioritise targets if required.  Turret will select the first 
	// target in the vector that it can engage
	void							Update(std::vector<iSpaceObject*> & enemy_contacts);

	// Analyse all potential targets in the area and change target if necessary/preferred
	void							EvaluateTargets(std::vector<iSpaceObject*> & enemy_contacts);

	// Force new target analysis next frame
	void							ForceNewTargetAnalysis(void);

	// Fires a projectile from the turret
	void							Fire(void);

	// Gets/sets the parent object to this turret
	CMPINLINE iSpaceObject *		GetParent(void) const									{ return m_parent; }
	void							SetParent(iSpaceObject *parent);

	// Retrieve a pointer to the articulated model for this turret
	CMPINLINE ArticulatedModel *	GetArticulatedModel(void)								{ return m_articulatedmodel; }

	// Set the articulated model to be used by this turret.  Performs validation; if the model is not suitable, 
	// returns an errorcode and the model is defaulted to NULL
	Result							SetArticulatedModel(ArticulatedModel *model);

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
	CMPINLINE float					GetMinRange(void) const										{ return m_minrange; }
	CMPINLINE float					GetMaxRange(void) const										{ return m_maxrange; }
	CMPINLINE void					SetRange(float minrange, float maxrange)
	{
		m_minrange = clamp(minrange, Game::C_MIN_TURRET_RANGE       , Game::C_MAX_TURRET_RANGE - 1.0f);	
		m_maxrange = clamp(maxrange, Game::C_MIN_TURRET_RANGE + 1.0f, Game::C_MAX_TURRET_RANGE       );

		m_minrangesq = (m_minrange * m_minrange);
		m_maxrangesq = (m_maxrange * m_maxrange);
	}


	// Determines the max range of the turret based on its component launchers & projectiles.  Is only
	// an approximation since the projectiles may have linear velocity degradation or in-flight orientation 
	// changes that we cannot simulate accurately here (without actually firing a projectile)
	void							DetermineApproxRange(void);

	// Yaws or pitches the turret by the specified angle
	void							Yaw(float angle);
	void							Pitch(float angle);

	// Reset the orientation of the turret back to its base (instantly)
	void							ResetOrientation(void);

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
	CMPINLINE int					GetLauncherCount(void) const		{ return m_launchercount; }

	// Clears the reference to all turret launcher data; used during object clone to allow deep-copy of launcher data
	void							ClearLauncherReferences(void);

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

	// Make and return a copy of this turret
	SpaceTurret *					Copy(void);

protected:

	// Unique string code, and descriptive string name
	std::string						m_code;
	std::string						m_name;

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
	//		m_baserelativeorient = resting orientation, [0 0 0 1] in many cases
	//		m_orientation = (baserelativeorient * parent_orientation), i.e. resting orientation in world space
	//		m_turretrelativeorient = (baserelativeorient * delta_from_pitch_yaw), i.e. relative turret cannon orientation
	//		m_turretorient = (relativeturretorient * parent_orientation), i.e. actual turret cannon orientation in world space
	D3DXVECTOR3						m_relativepos;
	D3DXQUATERNION					m_baserelativeorient;
	D3DXQUATERNION					m_turretrelativeorient;

	// Store world position, orientation and inverse orientation.  Derived during update by turret controller
	D3DXVECTOR3						m_position;
	D3DXQUATERNION					m_orientation, m_invorient;

	// Pitch and yaw of the turret, and resulting relative relative orientation.  Specified in the range [0 2PI]
	float							m_yaw, m_pitch;

	// Object world matrix, relative to its parent object.  Based on m_position and m_orientation
	D3DXMATRIX						m_worldmatrix;

	// Store orientation & inverse orientation of the turret heading (which will differ from the base turret orientation)
	D3DXQUATERNION					m_turretorient, m_invturretorient;

	// Values indicating the yaw & pitch capabilities of the turret
	float							m_yawrate, m_pitchrate;							// Rad/sec
	bool							m_yaw_limited;									// Flag indicating whether the turret has yaw extents, or can rotate 360-deg
	float							m_yawmin, m_yawmax;								// Min/max yaw extents, radians.  Only required if m_yaw_limited == true
																					// Specified in the range [0 2PI]
	float							m_pitchmin, m_pitchmax;							// Min/max pitch extents, radians.  Specified in the range [0 2PI]

	// Turret maintains two unit circle arcs, in the yaw & pitch dimensions, to represent its firing cone
	FiringArc						m_yaw_arc, m_pitch_arc;

	// Turret has a maximum range, and usually at least a nominal minimum range within which it cannot hit targets
	float							m_minrange, m_minrangesq;
	float							m_maxrange, m_maxrangesq;

	// Model indices for key components and constraints
	int								m_constraint_yaw;
	int								m_constraint_pitch;
	int								m_component_cannon;

};



#endif