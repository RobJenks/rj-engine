#pragma once

#ifndef __iStaticObjectH__
#define __iStaticObjectH__

#include "DX11_Core.h"

#include "iObject.h"


class iStaticObject : public iObject
{
public:

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate.  Nothing to do at this point.
	void										SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate) { }

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void										InitialiseCopiedObject(iStaticObject *source);
};



#endif
