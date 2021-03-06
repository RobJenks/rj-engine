#pragma once

#ifndef __SpaceTurretH__
#define __SpaceTurretH__

#include "DX11_Core.h"
#include "FiringArc.h"
#include "ArticulatedModel.h"
#include "ObjectReference.h"
#include "InstanceFlags.h"
#include "iSpaceObject.h"
class ProjectileLauncher;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class SpaceTurret : public ALIGN16<SpaceTurret>
{
public:

	// Index used to assign a unique incrementing ID to each new turret
	typedef unsigned long			TurretID;
	static TurretID					TurretIDCounter;
	static const TurretID			NULL_TURRET;

	// Define the possible turret control modes
	enum ControlMode				{ ManualControl = 0, AutomaticControl };

	// Define the possible turret status codes
	enum TurretStatus				{ Idle = 0, UnderManualControl, TrackingTarget, EngagingTarget, ReturningToIdle };

	// Static factory method to create new turret objects
	static SpaceTurret *			Create(void);
	static SpaceTurret *			Create(const std::string & code);
	static SpaceTurret *			Create(SpaceTurret *template_turret);

	// Default constructor
	SpaceTurret(void);
	
	// Return the unique turret ID for this instance
	CMPINLINE TurretID				GetTurretID(void) const									{ return m_turret_id; }

	// Assigns a new unique turret ID to this object
	CMPINLINE void					AssignNewUniqueTurretID(void)							{ m_turret_id = (++SpaceTurret::TurretIDCounter); }

	// Get/set the string name or code of this turret object
	CMPINLINE const std::string &	GetCode(void) const										{ return m_code; }
	CMPINLINE void					SetCode(const std::string & code)						{ m_code = code; }
	CMPINLINE const std::string &	GetName(void) const										{ return m_name; }
	CMPINLINE void					SetName(const std::string & name)						{ m_name = name; }


	// Primary update method for the turret.  Will take appropriate action depending on the control
	// mode currently set for the turret
	void							Update(std::vector<ObjectReference<iSpaceObject>> & enemy_contacts);

	// Analyse all potential targets in the area and change target if necessary/preferred
	void							EvaluateTargets(std::vector<ObjectReference<iSpaceObject>> & enemy_contacts);

	// Force new target analysis next frame
	void							ForceNewTargetAnalysis(void);

	// Fires a projectile from the turret
	void							Fire(void);

	// Gets/sets the parent object to this turret
	CMPINLINE Ship *				GetParent(void) const									{ return m_parent; }
	void							SetParent(Ship *parent);

	// Retrieve a pointer to the articulated model for this turret
	CMPINLINE ArticulatedModel *	GetArticulatedModel(void)								{ return m_articulatedmodel; }

	// Instance rendering flags for this object
	InstanceFlags					InstanceFlags;

	// Set the articulated model to be used by this turret.  Performs validation; if the model is not suitable, 
	// returns an errorcode and the model is defaulted to NULL
	Result							SetArticulatedModel(ArticulatedModel *model);
	Result							SetArticulatedModelInternal(ArticulatedModel *model);

	// Gets/sets the relative position of the turret on its parent object
	CMPINLINE XMVECTOR				GetRelativePosition(void) const							{ return m_relativepos; }
	CMPINLINE void					SetRelativePosition(const FXMVECTOR relativepos)		{ m_relativepos = relativepos; }

	// Gets/sets the relative resting orientation of the turret on its parent object.  This will be the 'resting' orientation
	CMPINLINE XMVECTOR				GetBaseRelativeOrientation(void) const					{ return m_baserelativeorient; }
	void							SetBaseRelativeOrientation(const FXMVECTOR orient);

	// Return the orientation of the turret cannon relative to its parent object (NOT relative
	// to the turret base).  This accounts for pitch & yaw
	CMPINLINE XMVECTOR				GetTurretRelativeOrientation(void) const				{ return m_turretrelativeorient; }

	// Return the world orientation of the turret cannon
	CMPINLINE XMVECTOR				GetCannonOrientation(void) const						{ return m_cannonorient; }

	// Return the inverse world orientation of the turret cannon
	CMPINLINE XMVECTOR				GetInverseCannonOrientation(void) const					{ return m_invcannonorient; }

	// Returns the pitch/yaw required to align the turret cannon with the specified vector
	XMFLOAT2						DetermineCannonYawAndPitchToVector(const XMVECTOR target_vector) const;

	// Returns the pitch/yaw required to align the turret cannon with the specified target
	XMFLOAT2						DetermineCannonYawAndPitchToTarget(const iActiveObject & target) const;

	// Determines whether the cannon is currently aligned to fire along the specified vector (with 
	// tolerance specified by the turret 'firing region threshold')
	bool							IsCannonAlignedToVector(const FXMVECTOR target_vector) const;

	// Returns the current status of this turret
	CMPINLINE TurretStatus			GetTurretStatus(void) const								{ return m_turretstatus; }

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

	// Retrieve a reference to the firing arcs for this turret
	CMPINLINE const FiringArc &		GetYawFiringArc(void)										{ return m_yaw_arc; }
	CMPINLINE const FiringArc &		GetPitchFiringArc(void)										{ return m_pitch_arc; }

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

	// Get or set the fire delay; the minimum delay between shots regardless of launcher readiness
	CMPINLINE unsigned int			GetTurretFireDelay(void) const								{ return m_firedelay; }
	CMPINLINE void					SetTurretFireDelay(unsigned int ms)							{ m_firedelay = ms; }

	// Determines the max range of the turret based on its component launchers & projectiles.  Is only
	// an approximation since the projectiles may have linear velocity degradation or in-flight orientation 
	// changes that we cannot simulate accurately here (without actually firing a projectile)
	void							DetermineApproxRange(void);

	// Multiplier on turret launcher spread, so that turret will fire when the target is within x launchers-spreads of 
	// centre.  Can be tuned per turret based on types of weapon mounted (e.g. we allow a greater multiple for smaller, 
	// faster-firing weapons than large projectile-firing cannons which should be more careful not to miss and not as
	// suitable for engaging fast-moving targets anyway)
	CMPINLINE float					GetFiringSpreadMultiplier(void) const						{ return m_firing_spread_multiplier; }
	CMPINLINE void					SetFiringSpreadMultiplier(float m)							
	{ 
		m_firing_spread_multiplier = m; 
		RecalculateTurretStatistics(); 
	}

	// Determines an appropriate firing region for the turret based on the accuracy of its component launchers, 
	// plus (TODO:) any contribution from pilot skill, ship computer effectiveness etc.
	void							DetermineFiringRegion(void);

	// Yaws or pitches the turret by the specified angle
	void							Yaw(float angle);
	void							Pitch(float angle);

	// Retrieve the current pitch and yaw values for this turret
	CMPINLINE float					GetPitch(void) const										{ return m_pitch; }
	CMPINLINE float					GetYaw(void) const											{ return m_yaw; }

	// Reset the orientation of the turret back to its base (instantly)
	void							ResetOrientation(void);

	// Indicates whether this is a fixed or rotating turret
	CMPINLINE bool					IsFixed(void) const											{ return m_isfixed; }
	CMPINLINE void					SetFixState(bool is_fixed)									{ m_isfixed = is_fixed; }

	// Indicates whether the turret is under manual or automatic (ship computer) control
	CMPINLINE ControlMode			GetControlMode(void) const									{ return m_mode; }
	CMPINLINE void					SetControlMode(ControlMode mode)							{ m_mode = mode; }

	// Sets the current target for the turret
	void							SetTarget(iSpaceObject *target);

	// Designates a target for the turret; will be engaged if possible, otherwise alternative targets
	// will be engaged in the meantime
	void							DesignateTarget(iSpaceObject *target);

	// Returns a flag indicating whether the target is within range, and within the firing arc of this turret
	bool							CanHitTarget(iSpaceObject *target);

	// Searches for a new target in the given vector of enemy contacts and returns the first valid one
	iSpaceObject *					FindNewTarget(std::vector<ObjectReference<iSpaceObject>> & enemy_contacts);

	// Returns a value indicating whether the turret currently has a target
	CMPINLINE bool					HasTarget(void) const										{ return (m_target != NULL); }

	// Returns a value indicating whether the turret has been given a designated target
	CMPINLINE bool					HasDesignatedTarget(void) const								{ return (m_designatedtarget != NULL); }

	// Retrieves the current target of this turret, or NULL if the turret currently has no target
	CMPINLINE iSpaceObject *		GetTarget(void) const										{ return m_target; }

	// Retrieves the current target designation, or NULL if no target has been designated
	CMPINLINE iSpaceObject *		GetDesignatedTarget(void) const								{ return m_designatedtarget; }

	// Allocates space for the required number of launcher objects
	void							InitialiseLaunchers(int launcher_count);

	// Retrieve a reference to one of the launchers within the turrent
	ProjectileLauncher *			GetLauncher(int index);

	// Returns the number of launchers maintained within this turret
	CMPINLINE int					GetLauncherCount(void) const		{ return m_launchercount; }

	// Sets a launcher to the specified object
	Result							SetLauncher(int index, const ProjectileLauncher *launcher);

	// Clears the reference to all turret launcher data; used during object clone to allow deep-copy of launcher data
	void							ClearLauncherReferences(void);

	// Returns a constant reference to the cannon position in world space, for use in calculating the firing trajectory
	CMPINLINE XMVECTOR				CannonPosition(void) const			{ return m_articulatedmodel->GetComponents()[m_component_cannon]->GetPosition(); }

	// Recalculates the object world matrix
	CMPINLINE void					DetermineWorldMatrix(void)
	{
		// Derive a new relative world matrix from the turret position & orientation (World = Rotation * Translation)
		m_worldmatrix = XMMatrixMultiply(
			XMMatrixRotationQuaternion(m_orientation), 
			XMMatrixTranslationFromVector(m_position));
	}

	// Recalculates all turret statistics based on contents; used post-initialisation to prepare turret for use
	void							RecalculateTurretStatistics(void); 

	// Perform an update of the turret position and orientation in world space
	void							UpdatePositioning(void);

	// Retrieve the absolute spatial data for this turret, derived during update method
	CMPINLINE XMVECTOR			GetPosition(void) const				{ return m_position; }
	CMPINLINE XMVECTOR			GetOrientation(void) const			{ return m_orientation; }
	CMPINLINE XMVECTOR			GetInverseOrientation(void) const	{ return m_invorient; }
	CMPINLINE XMMATRIX			GetWorldMatrix(void) const			{ return m_worldmatrix; }

	// Shut down the turret object, deallocating all resources
	void							Shutdown(void);

	// Make and return a copy of this turret
	SpaceTurret *					Copy(void);

	// Static method to translate a turret status code to its string representation
	static std::string				TranslateTurretStatusToString(TurretStatus status);

protected:

	// Unique turret ID, unique across every space turret instance
	TurretID						m_turret_id;

	// Class-unique string code, and descriptive string name
	std::string						m_code;
	std::string						m_name;

	// Parent object that this turret is attached to
	Ship *							m_parent;

	// Indicates whether this is a fixed weapon or rotating turret
	bool							m_isfixed;

	// Articulated model for this turret
	ArticulatedModel *				m_articulatedmodel;

	// Array of projectile launchers attached to this turret
	ProjectileLauncher *			m_launchers;				// Array of launcher objects arrached to the turret
	int								m_launchercount;			// The total count of launcher objects
	int								m_launcherubound;			// Efficiency measure; the upper bound of the launcher array.  (LauncherCount - 1)
	int								m_nextlauncher;				// The next launcher that will fire a projectile
	unsigned int					m_firedelay;				// The minimum period between shots, regardless of launcher readiness

	// Turret control mode indicates whether this turret is under manual or automatic (ship computer) control
	ControlMode						m_mode;

	// Current target that the turret is tracking
	iSpaceObject *					m_target;

	// Target that the turret has been manually assigned, and which it will try to reacquire if it is lost
	iSpaceObject *					m_designatedtarget;

	// Current status of the turret
	SpaceTurret::TurretStatus		m_turretstatus;

	// The threshold yaw/pitch within which this turret will begin firing.  Adjusted based on accuracy of weapons
	// and effectiveness of pilot/turret control computer
	float							m_firing_region_threshold;

	// Multiplier on turret launcher spread, so that turret will fire when the target is within x launchers-spreads of 
	// centre.  Can be tuned per turret based on types of weapon mounted (e.g. we allow a greater multiple for smaller, 
	// faster-firing weapons than large projectile-firing cannons which should be more careful not to miss and not as
	// suitable for engaging fast-moving targets anyway)
	float							m_firing_spread_multiplier;

	// The times & intervals for when the turret was last updated
	static const unsigned int		TARGET_ANALYSIS_INTERVAL = 2000U;
	unsigned int					m_nexttargetanalysis;


	// Turrets have a relative position and orientation from their parent object.  The latter is the base
	// orientation of the turret, before any pitch or yaw is applied
	//		m_baserelativeorient = resting orientation, [0 0 0 1] in many cases
	//		m_orientation = (baserelativeorient * parent_orientation), i.e. resting orientation in world space
	//		m_turretrelativeorient = (baserelativeorient * delta_from_pitch_yaw), i.e. relative turret cannon orientation
	AXMVECTOR						m_relativepos;
	AXMVECTOR						m_baserelativeorient;
	AXMVECTOR						m_turretrelativeorient;

	// Store world position, orientation and inverse orientation.  Derived during (full simulation) update by turret controller
	AXMVECTOR						m_position;
	AXMVECTOR						m_orientation, m_invorient;

	// Store cannon orientation and inverse.  Derived during (full simulation) update by turret controller
	AXMVECTOR						m_cannonorient;
	AXMVECTOR						m_invcannonorient;

	// Pitch and yaw of the turret, and resulting relative relative orientation.  Specified in the range [0 2PI]
	float							m_yaw, m_pitch;

	// Flag that indicates whether the turret is 'at rest', i.e. at the ID orientation
	bool							m_atrest;

	// Object world matrix, relative to its parent object.  Based on m_position and m_orientation
	AXMMATRIX						m_worldmatrix;

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

	// Returns the range to a specified target
	float							RangeSqToTarget(const iSpaceObject *target);

	// Indicates whether a target at the specified range is valid
	bool							TargetRangeSqIsValid(float range_sq);

	// Indicates whether the specified target is in range
	bool							TargetIsInRange(const iSpaceObject *target);

	// Indicates whether the specified target is within this turret's possible firing arc
	bool							TargetIsWithinFiringArc(const iSpaceObject *target);

};



#endif