#pragma once

#ifndef __iStaticObjectH__
#define __iStaticObjectH__

#include "DX11_Core.h"

#include "iObject.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class iStaticObject : public iObject
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(iStaticObject)

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate.  Nothing to do at this point.
	void										SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate) { }

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void										InitialiseCopiedObject(iStaticObject *source);

	// Shutdown method to remove this object from the simulation
	void										Shutdown(void);
};



#endif
