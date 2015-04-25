#pragma once

#ifndef __HardpointsH__
#define __HardpointsH__

#include <cstdlib>
#include <string>
#include <unordered_map>
#include "DX11_Core.h"
#include "iHardpoints.h"
#include "Hardpoint.h"
#include "iContainsHardpoints.h"
#include "CompilerSettings.h"
using namespace std;
using namespace std::tr1;

class Hardpoint;

class Hardpoints : public iHardpoints 
{
public:

	Hardpoints(void);
	~Hardpoints(void);
	
	//typedef unordered_map<string, Hardpoint*> HardpointItems;
	//HardpointItems				Items;
	//vector<Hardpoint*>		Items;
	//int *HPCount;							// A count of the number of each hardpoint class in the collection
	//Hardpoint ***HPGroup;					// Array of arrays of hardpoint category pointers
	// Helper function to retrieve the group and count of hardpoints of a particular class
	//void GetHardpointsOfClass(Equip::Class type, Hardpoint*** pOutHardpoints, int *pOutCount);

	// Returns a reference to a hardpoint based on the supplied string code
	CMPINLINE Hardpoint* operator[] (const string &code) { return m_items[code]; }
	CMPINLINE Hardpoint* Get(const string &code) { return m_items[code]; }

	void AddHardpoint(Hardpoint *hp);
	void RecalculateHardpoints(void);
	void PerformHardpointChangeRefresh(Hardpoint *hp);

	// Method to return all hardpoints of a particular type (i.e. one HP group)
	CMPINLINE iHardpoints::HardpointCollection *		GetHardpointsOfType(Equip::Class type)	{ return &(m_hpgroups[(int)type]); }

	// Returns a reference to all hardpoints maintained by the interface
	CMPINLINE iHardpoints::IndexedHardpointCollection *	GetAllHardpoints(void)					{ return &(m_items); }


	iObject *						GetParent(void);
	iContainsHardpoints *			GetHPParent(void);
	void							SetParent(iObject *parent, iContainsHardpoints *hp_parent);

	Hardpoints *Copy(void);

private:

	iHardpoints::IndexedHardpointCollection		m_items;
	iHardpoints::HardpointGroups				m_hpgroups;

	iObject *									m_parent;		// Parent object should inherit from both iObject and iContainsHardpoints
	iContainsHardpoints *						m_hp_parent;	// Split and store them separately here to enforce that, without defining a common subclass
	
};

CMPINLINE iObject *Hardpoints::GetParent()					{ return m_parent; }
CMPINLINE iContainsHardpoints *Hardpoints::GetHPParent()	{ return m_hp_parent; }

CMPINLINE void Hardpoints::SetParent(iObject *parent, iContainsHardpoints *hp_parent) 
{
	// Set the pointers to our new parent objects (or NULL, potentially)
	this->m_parent = parent; 
	this->m_hp_parent = hp_parent;

	// Run through every HP-type-specific update to ensure the parent is updated based on its new hardpoint collection
	if (m_hp_parent) this->m_hp_parent->PerformHardpointChangeRefresh(NULL);
}


#endif