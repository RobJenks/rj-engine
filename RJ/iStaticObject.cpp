#include "iStaticObject.h"

// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void iStaticObject::InitialiseCopiedObject(iStaticObject *source)
{
	// Pass control to all base classes
	iObject::InitialiseCopiedObject((iObject*)source);
}

// Shutdown method to remove this object from the simulation
void iStaticObject::Shutdown(void)
{
	// Pass control back to the base class
	iObject::Shutdown();
}