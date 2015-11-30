#pragma once

#ifndef __TurretControllerH__
#define __TurretControllerH__

#include <vector>
#include "CompilerSettings.h"
#include "iObject.h"
#include "SpaceTurret.h"

// Represents a collection of turrets, with methods for managing, simulating and rendering them
// This class has no special alignment requirements
class TurretController
{
public:

	// Define turret collection type
	typedef std::vector<SpaceTurret*>				TurretCollection;

	// Collection of turret objects
	TurretCollection								Turrets;

	// Default constructor
	TurretController(void);

	// Perform a full update of the turret collection
	void											Update(std::vector<iSpaceObject*> & enemy_contacts);

	// Indicates whether the turret controller is active, based on whether it is managing any turrets
	CMPINLINE bool									IsActive(void) const					{ return m_active; }

	// Return or set a refernce to the parent object that owns this turret controller
	CMPINLINE iSpaceObject *						GetParent(void)							{ return m_parent; }
	void											SetParent(iSpaceObject *parent);

	// Sets the control mode of all turrets
	void											SetControlModeOfAllTurrets(SpaceTurret::ControlMode mode);

	// Add, remove or retrieve turret objects in the collection
	bool											AddTurret(SpaceTurret *turret);
	bool											RemoveTurret(SpaceTurret *turret);
	void											RemoveAllTurrets(void);
	bool											HaveTurret(SpaceTurret *turret);
	CMPINLINE int									TurretCount(void)						{ return m_turretcount; }

	// Clears all controller contents without affecting any of the turrets themselves.  Generally used post-clone
	// to reset the controller without resetting the parent pointer of the (old) turrets being removed
	void											ForceClearContents(void);

	// Refreshes the turret controller status following a change to internal (e.g. turret removed) or external (e.g.
	// parent object) changes
	void											RefreshTurretCollection(void);

	// Default destructor
	~TurretController(void);


protected:

	// Reference to the parent object
	iSpaceObject *									m_parent;

	// Maintain a record of the number of turret objects, to avoid recalculating via size() each time
	int												m_turretcount;

	// Indicates whether the turret controller is active, based on whether it is managing any turrets
	bool											m_active;

};



#endif


