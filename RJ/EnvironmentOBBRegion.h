#pragma once

#ifndef __EnvironmentOBBRegion__
#define __EnvironmentOBBRegion__

#include <vector>
#include "Utility.h"
#include "FastMath.h"

// Structure used to hold one region of an environment OBB hierarchy
struct EnvironmentOBBRegion 
{
	typedef std::vector<EnvironmentOBBRegion> RegionCollection;
	typedef std::vector<EnvironmentOBBRegion>::size_type RegionIndex;
	
	enum RegionState { Unknown  = 0, Empty, Partial, Complete };	// Empty = all gaps, Partial = some elements & gaps, complete = all elements

	int element; INTVECTOR3 size;		// Element ID and size of the region
	int children[8];					// Child nodes below this region
	int childcount;						// The number of children active below this region

	EnvironmentOBBRegion(void) 
	{
		element = 0; size = ONE_INTVECTOR3; childcount = 0;
	}
	
	EnvironmentOBBRegion(int el, const INTVECTOR3 & region_size)
	{
		element = el;
		size = region_size;
		childcount = 0;
	}

	// Adds a new child region with the specified details
	void AddChild(int el, const INTVECTOR3 & region_size);

	// Removes the child region at the specified index
	CMPINLINE void RemoveChild(RegionIndex region_index)
	{
		int index = (int)region_index;
		if (index < 0 || index >= childcount) return;
		for (int i = (index + 1); i < childcount; ++i)
			children[i - 1] = children[i];
		--childcount;
	}

	// Removes all children from this node
	CMPINLINE void RemoveAllChildren(void)
	{
		childcount = 0U;
	}

	// Determines the new state of a region based on its current state, and the state of one of its child regions
	CMPINLINE static EnvironmentOBBRegion::RegionState ApplyChildRegionState(EnvironmentOBBRegion::RegionState current_state, EnvironmentOBBRegion::RegionState child_state)
	{
		return m_child_state_transitions[(int)current_state][(int)child_state];
	}

private:

	// Static mapping of { current region state, individual child state } -> { new region state }
	static const EnvironmentOBBRegion::RegionState m_child_state_transitions[4][4];

};

// Class used to help construct the OBB region hierarchies.  Uses e.g. static stack allocation for 
// efficiency & to avoid many small allocations
class EnvironmentOBBRegionBuilder
{
public:

	// Initialise the region builder.  Accepts a reference to the region that needs
	// to be analysed
	static void											Initialise(const EnvironmentOBBRegion & region);

	// Returns the region with the specified region index
	CMPINLINE static EnvironmentOBBRegion &				Get(EnvironmentOBBRegion::RegionIndex id) { return m_obb_regions[id]; }

	// Adds a new region to the collection.  Index of the index can be retrieved via NextRegionIndex() 
	// or CurrentRegionIndex(), pre- or post-addition respectively
	CMPINLINE static void								Add(int element, const INTVECTOR3 & size) 
	{ 
		m_obb_regions.push_back(EnvironmentOBBRegion(element, size)); 
	}

	// Returns the index of the next region that will be created
	CMPINLINE static EnvironmentOBBRegion::RegionIndex	NextRegionIndex(void) { return (m_obb_regions.size()); }

	// Returns the index of the last region to be created
	CMPINLINE static EnvironmentOBBRegion::RegionIndex	CurrentRegionIndex(void) { return (m_obb_regions.size() - 1); }

private:

	static EnvironmentOBBRegion::RegionCollection m_obb_regions;

};



#endif
