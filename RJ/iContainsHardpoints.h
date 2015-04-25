#pragma once

#ifndef __iContainsHardpointsH__
#define __iContainsHardpointsH__

#include <string>
#include "GameVarsExtern.h"
#include "iSpaceObject.h"
class Hardpoint;
using namespace std;

class iContainsHardpoints
{
public:

	// Virtual interface method: refreshes the parent object based on a given hardpoint, or all hardpoints if NULL is provided
	virtual void					PerformHardpointChangeRefresh(Hardpoint *hp)			= 0;

	// Virtual interface method: objects must expose a way back to their base iSpaceObject class (since this interface is not derived 
	// from it).  This saves virtual inheritance from iSpaceObject
	virtual iSpaceObject *			GetSpaceObjectReference(void)							= 0;

};


#endif