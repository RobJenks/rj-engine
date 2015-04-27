#include "GameVarsExtern.h"
#include "GameSpatialPartitioningTrees.h"
#include "CapitalShipPerimeterBeacon.h"
#include "SpaceSystem.h"
#include "iActiveObject.h"
#include "Player.h"

#include "iSpaceObject.h"


// Constructor; assigns a unique ID to this object
iSpaceObject::iSpaceObject(void)
{
	// Initialise key fields to their default values
	m_treenode = NULL;
	m_spaceenvironment = NULL;
}

// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void iSpaceObject::InitialiseCopiedObject(iSpaceObject *source)
{
	// Initialise the spatial tree node to null, since our position is likely different from the copy source
	m_treenode = NULL;

	// Pass control to all base classes
	iActiveObject::InitialiseCopiedObject((iActiveObject*)source);
}

// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
void iSpaceObject::SimulationStateChanged(iObject::ObjectSimulationState prevstate, iObject::ObjectSimulationState newstate)
{
	// If we were not being simulated, and we now are, then we may need to change how the object is operating
	// TODO: this will not always be true in future when we have more granular simulation states 
	if (prevstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// Take action
	}

	// Conversely, if we are no longer going to be simulated, we may be able to stop simulating certain things
	if (newstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// Take action
	}
}

// Moves the space object into a new environment
void iSpaceObject::MoveIntoSpaceEnvironment(SpaceSystem *system, const D3DXVECTOR3 & location)
{
	// First, remove the object from its current environment (if applicable)
	if (m_spaceenvironment)
	{
		this->RemoveFromEnvironment();
	}

	// Now add to the new environment, assuming a valid environment has been specified
	if (system)
	{
		system->AddObjectToSystem(this, location);
	}

}

// Removes the object from its current space environment
void iSpaceObject::RemoveFromEnvironment(void)
{
	// If we are located within a system, call the removal method which will also reset all pointers etc. to NULL
	if (m_spaceenvironment)
	{
		m_spaceenvironment->RemoveObjectFromSystem(this);
	}
}


// Destructor; removes the item from the space object register, if it exists
iSpaceObject::~iSpaceObject(void)
{
	// Remove from any environment we currently exist in
	RemoveFromEnvironment();
}


// Searches for all items within the specified distance of this object.  Returns the number of items located
// Allows use of certain flags to limit results during the search; more efficient that returning everything and then removing items later
//		only_colliding_objects - only return items that have m_collides flag == true.  Allows us to use this method for efficient collision detection
int iSpaceObject::GetAllObjectsWithinDistance(float distance, std::vector<iSpaceObject*> &outResult, bool only_colliding_objects,
	bool include_focal_object_boundary, bool include_target_object_boundaries)
{
	/* For now, we wil take one of two approaches.  If the search distance fits entirely within this item's node (the very likely case)
	we can efficiently iterate over the items in this node and return them.  If it does not, we will keep moving up the tree until
	we find a parent node that does emcompass the search area, locate all items within this node & it's children, and then consider
	each of those in turn.  This could be made more efficient (and complex) in future by only considering specific nodes rather than
	just moving up the tree.  e.g. if we need to search further in +ve x direction then locate the relevant node(s) to the right of
	this one.
	*/
	iSpaceObject **pObj;
	int numobjects;
	std::vector<iSpaceObject*> items;
	Game::ID_TYPE objid = 0, lastcapship = 0;
	static vector<Game::ID_TYPE> capshipsrecorded;
	bool foundcapship;


	// We must be a member of the spatial partitioning Octree to search for objects.  If not, return nothing immediately
	if (!m_treenode) return 0;

	// Get the position of this object.  All comparisons will be based on squared distance to avoid costly sqrt operations
	D3DXVECTOR3 pos = this->GetPosition();
	float distsq = (distance * distance);

	// If we want to test from the focal object's boundary then add that boundary to the search distance now
	if (include_focal_object_boundary) distsq += m_collisionsphereradiussq;

	// Clear the output vector ready to be populated with results
	int found = 0;
	outResult.clear();

	// Determine whether we can take approach 1, i.e. considering only the contents of our current node
	if ((pos.x - distance >= m_treenode->m_xmin) && (pos.x + distance < m_treenode->m_xmax) && (pos.y - distance >= m_treenode->m_ymin) &&
		(pos.y + distance < m_treenode->m_ymax) && (pos.z - distance >= m_treenode->m_zmin) && (pos.z + distance < m_treenode->m_zmax))
	{
		// The search area does fit within this node, so we can take approach 1.  Make sure we have any items besides ourself; if not, we can stop here
		if (m_treenode->m_itemcount <= 1) return 0;

		// We will simply consider the items in this node
		pObj = &(m_treenode->m_items[0]);
		numobjects = m_treenode->m_itemcount;
	}
	else
	{
		// We cannot take the more efficient approach since the search area is not fully contained within this node.  We therefore need
		// to take approach 2 and traverse up the tree until we find a node that fully contains the search area, or we reach the root
		Octree<iSpaceObject*> *node = m_treenode;
		while (node->m_parent)
		{
			// Move up to the parent node
			node = node->m_parent;

			// Test whether this now fully contains the search area
			if ((pos.x - distance >= node->m_xmin) && (pos.x + distance < node->m_xmax) && (pos.y - distance >= node->m_ymin) &&
				(pos.y + distance < node->m_ymax) && (pos.z - distance >= node->m_zmin) && (pos.z + distance < node->m_zmax))
			{
				// It does, so break out of the loop and use this node for locating objects
				break;
			}
		}

		// We now know that node contains our full search area (or is the root, i.e. all objects) so get a vector of all items in its bounds
		node->GetItems(items);
		pObj = &(items.at(0));
		numobjects = items.size();
	}

	// We can now use the same logic in each case, by starting at a pointer to item[0] and incrementing our way through the collection.  Works
	// because both the local node storage (iSpaceObject*[]) and result of the GetItems method (vector<iSpaceObject*>) use contiguous storage


	// Iterate through each item in turn
	iSpaceObject *obj;
	D3DXVECTOR3 objpos, diff;
	for (int i = 0; i<numobjects; i++, pObj++)
	{
		// We can ignore this entry if it does not represent an item, or if it is us
		obj = *pObj;
		if (!obj || obj == this) continue;

		// We can also ignore it if the item is non-colliding and we are only interested in colliding objects
		if (only_colliding_objects && (obj->GetCollisionMode() == Game::CollisionMode::NoCollision)) continue;

		// Special case: if this is a capital ship beacon then we want to instead treat it like the parent capital ship
		if (obj->GetObjectType() == iObject::ObjectType::CapitalShipPerimeterBeaconObject)
			obj = ((CapitalShipPerimeterBeacon*)obj)->GetParentShip();
		if (!obj || obj == this) continue;

		/*
		Test approach 1; if this is a capital ship or other large object, then
		a) We want to make sure we only add it once (e.g. if it has beacons deployed)
		b) We want to test distance using a world-matrix-transformed point test, rather than treating as a point object
		*/
		if (obj->GetObjectType() == iObject::ObjectType::ComplexShipObject)
		{
			objid = obj->GetID();

			// If this is the first cap ship we have found in this check, reset the search vector
			if (lastcapship == 0)
				capshipsrecorded.clear();
			else
			{
				// Otherwise, for efficiency, test if this is the very last capital ship we recorded.  If so we can skip it	
				if (objid == lastcapship) continue;
			}

			// Record this ship as the last cap ship we have found
			lastcapship = objid;

			// We also then need to check if it is linked to *any* capital ship we have recorded (if there is >1).  If so we can skip it
			int n = capshipsrecorded.size();
			foundcapship = false;
			for (int i = 0; i<n; i++) if (capshipsrecorded.at(i) == objid) { foundcapship = true; break; }
			if (foundcapship) continue;

			// This is a new capital ship we haven't seen before; transform the viewer point into the local space of this object
			D3DXVECTOR3 transformedpoint;
			D3DXVec3TransformCoord(&transformedpoint, &(this->GetPosition()), obj->GetInverseWorldMatrix());

			// Now test whether the viewer sits within the ship bounds +/- the search distance; if not, we can skip the object
			if (transformedpoint.x < (-distance) || transformedpoint.y < (-distance) || transformedpoint.z < (-distance) ||
				transformedpoint.x >(m_size.x + distance) || transformedpoint.y >(m_size.y + distance) || transformedpoint.z >(m_size.z + distance))
				continue;
		}
		else
		{
			/*
			Test approach 2; any other object type can be treated as a point object, and we use a simple dist^2 test
			*/

			// Get the position of this object
			objpos = obj->GetPosition();

			// Calculated the squared distance between ourself and this object
			diff.x = (pos.x - objpos.x); diff.y = (pos.y - objpos.y); diff.z = (pos.z - objpos.z);
			diff.x *= diff.x; diff.y *= diff.y; diff.z *= diff.z;

			// If we want to inlcude the target boundary distance, add to one component now so it is incorporated in the comparison
			if (include_target_object_boundaries) diff.x -= obj->GetCollisionSphereRadiusSq();

			// If the object is outside our threshold distance, including collision radius of the target, then continue.
			if ((diff.x + diff.y + diff.z) > distsq) continue;
		}


		/* The item is within range and we want to include it */

		// Add this item to the collection
		outResult.push_back(obj); ++found;
	}

	// We can return at this point, having considered each item in the node in turn
	return found;
}

