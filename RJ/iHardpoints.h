#pragma once

#ifndef __iHardpointsH__
#define __iHardpointsH__

#include <vector>
#include <unordered_map>
#include <string>
#include "Equip.h"
class Hardpoint;
using namespace std;
using namespace std::tr1;

class iHardpoints
{
public:

	// Definition of the indexed hardpoint collection
	typedef vector<Hardpoint*>						HardpointCollection;
	typedef unordered_map<string, Hardpoint*>		IndexedHardpointCollection;
	typedef vector<HardpointCollection>				HardpointGroups;

	// Virtual interface method: return a hardpoint based on the code provided
	virtual Hardpoint *								Get(const string &code)							= 0;

	// Virtual interface method: return all hardpoints of a particular type (i.e. one HP group)
	virtual HardpointCollection *					GetHardpointsOfType(Equip::Class type)			= 0;

	// Virtual interface method: returns a reference to all hardpoints maintained by the interface
	virtual IndexedHardpointCollection *			GetAllHardpoints(void)							= 0;

};



#endif