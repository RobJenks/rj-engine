#pragma once

#ifndef __TurretControllerH__
#define __TurretControllerH__

#include <vector>
#include "CompilerSettings.h"
#include "iObject.h"
#include "ObjectReference.h"
#include "SpaceTurret.h"

// Represents a collection of turrets, with methods for managing, simulating and rendering them
// This class has no special alignment requirements
class TurretController
{
public:

	// Define turret collection type
	typedef std::vector<SpaceTurret*>				TurretCollection;

	// Collection of turret objects
	CMPINLINE const TurretCollection &				GetTurrets(void) const					{ return m_turrets; }

	// Returns a pointer to the specified turret, or NULL if no such turret exists
	CMPINLINE SpaceTurret *							GetTurret(TurretCollection::size_type index)
	{
		if (index < m_turretcount)					return m_turrets[index];
		else										return NULL;
	}

	// Modifiable reference to the turret collection, if absolutely required
	CMPINLINE TurretCollection &					Turrets(void)							{ return m_turrets; }

	// Default constructor
	TurretController(void);

	// Perform a full update of the turret collection
	void											Update(std::vector<ObjectReference<iSpaceObject>> & enemy_contacts);

	// Indicates whether the turret controller is active, based on whether it is managing any turrets
	CMPINLINE bool									IsActive(void) const					{ return m_active; }

	// Return or set a refernce to the parent object that owns this turret controller
	CMPINLINE Ship *								GetParent(void)							{ return m_parent; }
	void											SetParent(Ship *parent);

	// Sets the control mode of all turrets
	void											SetControlModeOfAllTurrets(SpaceTurret::ControlMode mode);

	// Add, remove or retrieve turret objects in the collection
	bool											AddTurret(SpaceTurret *turret);
	bool											RemoveTurret(SpaceTurret *turret);
	void											RemoveAllTurrets(void);
	bool											HaveTurret(SpaceTurret *turret);
	CMPINLINE TurretCollection::size_type			TurretCount(void)						{ return m_turretcount; }

	// Clears all controller contents without affecting any of the turrets themselves.  Generally used post-clone
	// to reset the controller without resetting the parent pointer of the (old) turrets being removed
	void											ForceClearContents(void);

	// Refreshes the turret controller status following a change to internal (e.g. turret removed) or external (e.g.
	// parent object) changes
	void											RefreshTurretCollection(void);

	// Sets the current target for all ship turrets 
	void											SetTarget(iSpaceObject *target);

	// Changes the current target for all ship turrets targeting current_target to new_target
	void											ChangeTarget(iSpaceObject *current_target, iSpaceObject *new_target);

	// Sets the designated target for all ship turrets 
	void											SetDesignatedTarget(iSpaceObject *designated_target);

	// Changes the designated target for all ship turrets targeting current_designation to new_designation
	void											ChangeDesignatedTarget(iSpaceObject *current_designation, iSpaceObject *new_designation);

	// Returns the average projectile velocity for all turrets managed by this controller
	CMPINLINE float									GetAverageProjectileVelocity(void) const	{ return m_avg_projectile_velocity; }

	// Default destructor
	~TurretController(void);


protected:

	// Collection of turrets managed by the controller
	TurretCollection								m_turrets;

	// Reference to the parent object
	Ship *											m_parent;

	// Maintain a record of the number of turret objects, to avoid recalculating via size() each time
	TurretCollection::size_type						m_turretcount;

	// Indicates whether the turret controller is active, based on whether it is managing any turrets
	bool											m_active;

	// Turret controller keeps track of the average projectile velocity in its turret collection, 
	// to enable more accurate target leading (velocity /sec)
	float											m_avg_projectile_velocity;

};



#endif


