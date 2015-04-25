#pragma once

#ifndef __ScheduledObjectH__
#define __ScheduledObjectH__

#include "CompilerSettings.h"

class ScheduledObject
{
public:

	// Update methods for frequent and infrequent evaluation.  Should be overriden by subclasses otherwise will do nothing
	virtual void					Update(void) = 0				{ }
	virtual void					UpdateInfrequent(void) = 0		{ }

};





#endif