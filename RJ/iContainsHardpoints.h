#pragma once

#ifndef __iContainsHardpointsH__
#define __iContainsHardpointsH__

class Hardpoint;
#include "Hardpoints.h"
#include "CompilerSettings.h"
#include "TurretController.h"

// Class has no special alignment requirements
class iContainsHardpoints
{
public:

	// Default constructor
	iContainsHardpoints(void);

	// Returns a reference to the hardpoints collection
	CMPINLINE Hardpoints &			GetHardpoints(void)							{ return m_hardpoints; }

	// Assigns a new hardpoints collection to this object
	CMPINLINE void					AssignHardpoints(Hardpoints & hardpoints)
	{
		m_hardpoints.Clone(hardpoints);			// Clone from the source collection.  This will clone all hardpoints within it as well
		m_hardpoints.SetParent(this);			// Make sure that the parent pointer is still pointing at this object
	}

	// Pure virtual event handler to be implemented by inheriting class.  Called by this class when the hardpoint 
	// configuration of the object is changed.  Provides a reference to the hardpoint that was changed, or NULL
	// if a more general update based on all hardpoints is required (e.g. after first-time initialisation)
	virtual void					HardpointChanged(Hardpoint *hp)				= 0;

	// This object contains a turret controller to manage simulation and AI of all its turrets
	TurretController				TurretController;

	// Default destructor
	~iContainsHardpoints(void);

protected:

	// Hardpoints collection
	Hardpoints  					m_hardpoints;

};


#endif

