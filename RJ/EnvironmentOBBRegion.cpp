#include "EnvironmentOBBRegion.h"

// Initialise static region collection
EnvironmentOBBRegion::RegionCollection EnvironmentOBBRegionBuilder::m_obb_regions;

// Static mapping of { current region state, individual child state } -> { new region state }
const EnvironmentOBBRegion::RegionState EnvironmentOBBRegion::m_child_state_transitions[4][4] = {

	// Current region state = Unknown, child state = { Unknown, Empty, Partial, Complete }
	{ EnvironmentOBBRegion::RegionState::Unknown, EnvironmentOBBRegion::RegionState::Empty, EnvironmentOBBRegion::RegionState::Partial, EnvironmentOBBRegion::RegionState::Complete }, 

	// Current region state = Empty, child state = { Unknown, Empty, Partial, Complete }
	{ EnvironmentOBBRegion::RegionState::Empty, EnvironmentOBBRegion::RegionState::Empty, EnvironmentOBBRegion::RegionState::Partial, EnvironmentOBBRegion::RegionState::Partial },
	
	// Current region state = Partial, child state = { Unknown, Empty, Partial, Complete }
	{ EnvironmentOBBRegion::RegionState::Partial, EnvironmentOBBRegion::RegionState::Partial, EnvironmentOBBRegion::RegionState::Partial, EnvironmentOBBRegion::RegionState::Partial },

	// Current region state = Complete, child state = { Unknown, Empty, Partial, Complete }
	{ EnvironmentOBBRegion::RegionState::Complete, EnvironmentOBBRegion::RegionState::Partial, EnvironmentOBBRegion::RegionState::Partial, EnvironmentOBBRegion::RegionState::Complete }
};

// Initialise the region builder.  Accepts a reference to the region that needs
// to be analysed
void EnvironmentOBBRegionBuilder::Initialise(const EnvironmentOBBRegion & region)
{
	// Clear any existing region data in advance of the next calculation
	m_obb_regions.clear();

	// Ensure that the vector has enough memory allocated for 2*region.element_size.  This ensures that 
	// an Octree-based partitioning of the region will never exceed the available capacity and require
	// a reallocation, even in the worst-case scenario where every 1x1x1 element represents its own region
	m_obb_regions.reserve(region.size.x * region.size.y * region.size.z * 2);
}


// Adds a new child region with the specified details
void EnvironmentOBBRegion::AddChild(int el, const INTVECTOR3 & region_size)
{
	//children[childcount++] = EnvironmentOBBRegionBuilder::Add(el, region_size);

	// Use the statements below instead of the above, to ensure operations are atomic and 
	// not affected if the vector::push_back() method invalidates our "this" reference 
	// part way through execution.
	// Note: This may no longer be required since the vector is now reserve()-ed to a 
	// capacity that ensures no reallocations are required, however we keep this method
	// since it is still safer to maintain the two separate operations that cannot be
	// affected by an invalidation mid-execution
	children[childcount++] = (int)EnvironmentOBBRegionBuilder::NextRegionIndex();
	EnvironmentOBBRegionBuilder::Add(el, region_size);
}




