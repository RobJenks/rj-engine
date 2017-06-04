#pragma once

#ifndef __HardpointsH__
#define __HardpointsH__

#include <string>
#include <vector>
#include <unordered_map>
#include "CompilerSettings.h"
#include "Equip.h"
class iObject;
class iContainsHardpoints;
class Hardpoint;

// This class has no special alignment requirements
class Hardpoints
{
public:

	// Define each type of indexed hardpoint collection
	typedef std::vector<Hardpoint*>									HardpointCollection;
	typedef std::unordered_map<std::string, Hardpoint*>		IndexedHardpointCollection;
	typedef std::vector<HardpointCollection>						HardpointGroups;
	
	// Default constructor
	Hardpoints(void);

	// Returns a reference to a hardpoint based on the supplied string code
	CMPINLINE Hardpoint* operator[] (const std::string & code) 	{ return (m_items.count(code) != 0 ? m_items[code] : NULL); }
	CMPINLINE Hardpoint* Get(const std::string & code) 			{ return (m_items.count(code) != 0 ? m_items[code] : NULL); }

	// Add a new hardpoint to the collection.  Raises a HardpointChanged event to be handled by the parent
	void												AddHardpoint(Hardpoint *hp);

	// Recalculate internal hardpoint data based on new/removed hardpoints.  Will be automatically called
	// when a hardpoint is added/removed at runtime, or after updates are resumed following a series of changes
	void												RecalculateHardpoints(void);

	// Method to return all hardpoints of a particular type (i.e. one HP group)
	CMPINLINE Hardpoints::HardpointCollection &			GetHardpointsOfType(Equip::Class type)	{ return m_hpgroups[(int)type]; }

	// Returns a reference to all hardpoints maintained by the interface
	CMPINLINE Hardpoints::IndexedHardpointCollection &	GetAllHardpoints(void)					{ return m_items; }

	// Returns the parent object of this hardpoint collection
	CMPINLINE iObject *						GetParent(void) const					{ return m_parent; }
	CMPINLINE iContainsHardpoints *			GetHPParent(void) const					{ return m_hpparent; }

	// Sets a new parent object for this collection.  Templated method that will separate the parent object into its
	// two required base classes.  Parent object must be of a type deriving from { iObject, iContainsHardpoints }
	template <typename T>
	void												SetParent(T *parent)	
	{ 
		m_parent = (iObject*)parent; 
		m_hpparent = (iContainsHardpoints*)parent;
	}

	// Suspend updates based on any hardpoint changes until the updates are resumed again
	CMPINLINE void										SuspendUpdates(void)					{ m_suspendupdates = true; }

	// Resume updates based on hardpoint changes.  Invokes an initial update upon resume
	void												ResumeUpdates(void);

	// Clones the contents of another hardpoints collection into this one.  Will create new copies of all
	// hardpoints within the collection (rather than shared pointers)
	void												Clone(Hardpoints & source);

	// Clears all hardpoint data.  Hardpoints themselves are unaffected; this just clears the collections
	void												ClearData(void);

	// Clears all hardpoint data, deallocating all hardpoints held within the collection
	void												ShutdownAllHardpoints(void);

	// Default destructor
	~Hardpoints(void);

private:

	// Hardpoints are stored in two collections; one linear vector, and in type-specific groups
	Hardpoints::IndexedHardpointCollection				m_items;
	Hardpoints::HardpointGroups							m_hpgroups;

	// Parent object inheriting from iContainsHardpoints
	iObject *											m_parent;
	iContainsHardpoints *								m_hpparent;	

	// Flag indicating whether updates based on hardpoint changes should be suspended, for example during 
	// first-time initialisation where we do not want to react to population of of each hardpoint in turn
	bool												m_suspendupdates;
	
};

#endif