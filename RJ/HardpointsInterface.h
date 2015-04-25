#pragma once

#ifndef __HardpointsInterfaceH__
#define __HardpointsInterfaceH__

#include <vector>
#include "Equip.h"
#include "iHardpoints.h"
class Hardpoint;
class Hardpoints;

class HardpointsInterface : public iHardpoints 
{
public:
	typedef vector<Hardpoints*>							HardpointsLinkCollection;

	// Methods to add to, remove from and clear the collection of hardpoint links
	void												AddHardpointsLink(Hardpoints *hardpoints);
	void												RemoveHardpointsLink(Hardpoints *hardpoints);
	void												RemoveHardpointsLink(int index);
	int													FindHardpointsLink(Hardpoints *hardpoints);
	bool												HasHardpointsLink(Hardpoints *hardpoints);
	void												ClearHardpointsLinks(void);
	Hardpoints *										GetHardpointsLink(int index);
	CMPINLINE int										GetHardpointsLinkCount(void) { return m_hplinks.size(); }

	// Method which rebuilds the interface to hardpoint collections based on the set of hardpoints links
	void												RebuildHardpointsInterface(void);

	// Returns a hardpoint matching the supplied string code
	CMPINLINE Hardpoint *								Get(const string &code)					{ return m_hardpoints[code]; }	

	// Method to return all hardpoints of a particular type (i.e. one HP group)
	CMPINLINE iHardpoints::HardpointCollection *		GetHardpointsOfType(Equip::Class type)	{ return &(m_hpgroups[(int)type]); }

	// Returns a reference to all hardpoints maintained by the interface
	CMPINLINE iHardpoints::IndexedHardpointCollection *	GetAllHardpoints(void)					{ return &(m_hardpoints); }

	// Creates an identical copy of the 

	// Shutdown method which clears all the collections (without deallocating, since this just an interface)
	void												Shutdown(void);

	// Constructor/destructor
	HardpointsInterface(void);
	~HardpointsInterface(void);

private:

	// Initialises the interface on startup to be ready for operation
	void										InitialiseHardpointsInterface(void);

	// Key collections maintained by the interface
	HardpointsLinkCollection					m_hplinks;			// The set of links to other 'Hardpoints' collections
	iHardpoints::HardpointGroups				m_hpgroups;			// A collection of vectors, each containing hardpoints of a particular type
	iHardpoints::IndexedHardpointCollection		m_hardpoints;		// A collection of all hardpoints maintained by the interface, across all types

};



#endif