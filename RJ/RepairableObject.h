#pragma once

#ifndef __RepairableObjectH__
#define __RepairableObjectH__

#include "CompilerSettings.h"

class RepairableObject
{
public:

	// Constructor
	RepairableObject(void) : m_is_destroyed(false) { }

	// Check whether this object has been destroyed
	CMPINLINE bool			IsDestroyed(void) const						{ return m_is_destroyed; }

	// Change the destruction state of this object
	CMPINLINE void			MarkObjectAsDestroyed(void)					{ m_is_destroyed = true; }
	CMPINLINE void			MarkObjectAsRepaired(void)					{ m_is_destroyed = false; }
	CMPINLINE void			SetObjectDestroyedState(bool destroyed)		{ m_is_destroyed = destroyed; }

protected:

	// Flag indicating whether the object has been "destroyed".  Leaving the object in this 
	// state means that it can be repaired in the future 
	bool					m_is_destroyed;

};



#endif