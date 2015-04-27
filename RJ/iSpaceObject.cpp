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
	
	static vector<iSpaceObject*> large_objects_recorded;
	iSpaceObject *last_large_obj = NULL; 
	bool found_large_object;


	// We must be a member of the spatial partitioning Octree to search for objects.  If not, return nothing immediately
	if (!m_treenode) return 0;

	// If we want to test from the focal object's boundary then add that boundary to the search distance now
	if (include_focal_object_boundary) distance += m_collisionsphereradius;

	// Get the position of this object.  All comparisons will be based on squared distance to avoid costly sqrt operations
	const D3DXVECTOR3 & pos = this->GetPosition();
	float distsq = (distance * distance);
	float xmin = (pos.x - distance), ymin = (pos.y - distance), zmin = (pos.z - distance),
		  xmax = (pos.x + distance), ymax = (pos.y + distance), zmax = (pos.z + distance);

	// Starting point for the search will be our current node
	Octree<iSpaceObject*> *node = m_treenode;

	// Determine whether we can take approach 1, i.e. considering only the contents of our current node
	if ((xmin >= m_treenode->m_xmin) && (xmax < m_treenode->m_xmax) && (ymin >= m_treenode->m_ymin) &&
		(ymax < m_treenode->m_ymax) && (zmin >= m_treenode->m_zmin) && (zmax < m_treenode->m_zmax))
	{
		// The search area does fit within this node, so we can take approach 1.  Make sure we have any items besides ourself; 
		// if not, we can stop here
		if (m_treenode->m_itemcount <= 1) return 0;

		// We will simply consider the items in this node; no need to assign "node" since it already points to our tree node
		// node = m_treenode;
	}
	else
	{
		// We cannot take the more efficient approach since the search area is not fully contained within this node.  We therefore need
		// to take approach 2 and traverse up the tree until we find a node that fully contains the search area, or we reach the root
		while (node->m_parent)
		{
			// Move up to the parent node
			node = node->m_parent;

			// Test whether this now fully contains the search area
			if ((xmin >= node->m_xmin) && (xmax < node->m_xmax) && (ymin >= node->m_ymin) &&
				(ymax < node->m_ymax) && (zmin >= node->m_zmin) && (zmax < node->m_zmax))
			{
				// It does, so break out of the loop and use this node for locating objects
				break;
			}
		}
	}
	
	// We now know that node contains our full search area (or is the root, i.e. all objects) so we want to get a vector 
	// of all items in its bounds.  Use a non-recursive vector substitute instead of normal recursive search for efficiency
	std::vector<Octree<iSpaceObject*>*> search;
	search.push_back(node);

	// Initialise the results vector before starting
	int found = 0;
	outResult.clear();

	// Initialise any other variables needed for the analysis phase
	std::vector<iSpaceObject*>::const_iterator obj_end, obj_it;
	iSpaceObject *obj;
	float diffx, diffy, diffz;

	// Search nodes from the left ("index"), and add new search candidates to the right.  Only node in place at the start
	// will be the selected "node" that encompasses the entire search area
	int index = -1, count = 1; 
	while (++index < count)
	{
		node = search[index]; if (!node) continue;

		if (node->m_children[0] != NULL)
		{
			// If this is a branch node, add its children to the search vector
			search.insert(search.end(), node->m_children, (node->m_children + 8));
			count += 8;
		}
		else
		{
			// Otherwise, this is a leaf node.  Process each item within it
			obj_end = node->m_items.end();
			for (obj_it = node->m_items.begin(); obj_it != obj_end; ++obj_it)
			{
				// We can ignore this entry if it does not represent an item, or if it is us
				obj = (iSpaceObject*)(*obj_it);
				if (!obj || obj == this) continue;
				const D3DXVECTOR3 & objpos = obj->GetPosition();

				// We can also ignore it if the item is non-colliding and we are only interested in colliding objects
				if (only_colliding_objects && (obj->GetCollisionMode() == Game::CollisionMode::NoCollision)) continue;

				// Special case: if this is a capital ship beacon then we want to instead treat it like the parent capital ship
				if (obj->GetObjectType() == iObject::ObjectType::CapitalShipPerimeterBeaconObject)
					obj = ((CapitalShipPerimeterBeacon*)obj)->GetParentShip();

				// Method 1 - Large objects: if this is a large object, avoid multiple-counting the objects based on any
				// beacons it may have deployed.  Test distance using a conservative test of (distsq + collradiussq) < targetdistsq, 
				// which may yield some false positives but which is much quicker at this stage than a proper distance test
				if (obj->GetObjectType() == iObject::ObjectType::ComplexShipSectionObject ||
					obj->GetObjectType() == iObject::ObjectType::ComplexShipObject)
				{
					// Check whether we have already found this object; continue to the next object if we have
					// Also clear the static large object vector if this is the first time we are using it
					if (last_large_obj == NULL) large_objects_recorded.clear();
					else						if (last_large_obj == obj) continue;

					// Check whether we have already recorded this object; ignore the final entry since this was "last_large_obj"
					int n = (large_objects_recorded.size() - 1);
					found_large_object = false;
					for (int i = 0; i < n; ++i) if (large_objects_recorded[i] == obj) { found_large_object = true; break; }
					if (found_large_object) continue;

					// We have not recorded the object; add it to the list now
					large_objects_recorded.push_back(obj);
					last_large_obj = obj;

					// Get the squared distance from our position to the object
					diffx = (pos.x - objpos.x), diffy = (pos.y - objpos.y), diffz = (pos.z - objpos.z);
					diffx *= diffx; diffy *= diffy; diffz *= diffz;

					// The object is invalid if it is outside (distsq + collisionradiussq) of the position
					if ((diffx + diffy + diffz) > (distsq + obj->GetCollisionSphereRadiusSq())) continue;
				}

				// Method 2 - normal objects: any other object type can be treated as a point object, and we use a simple dist ^ 2 test
				else
				{
					// Calculated the squared distance between ourself and this object
					diffx = (pos.x - objpos.x), diffy = (pos.y - objpos.y), diffz = (pos.z - objpos.z);
					diffx *= diffx; diffy *= diffy; diffz *= diffz;

					// Test object validity based upon its squared distance to the target position.  Optionally account for
					// the target object collision radius as well
					if (include_target_object_boundaries)
					{
						if ((diffx + diffy + diffz) > (distsq + obj->GetCollisionSphereRadiusSq())) continue;
					}
					else
					{
						if ((diffx + diffy + diffz) > distsq) continue;
					}
				}

				// The object is valid; add it to the output vector
				outResult.push_back(obj);
			}
		}
	}

static int dbgc = 0;
dbgc += outResult.size();
OutputDebugString(concat("Obj \"")(m_name)("\" (")(m_id)(") - NodeSearch: ")(search.size())(", ObjResults: ")(outResult.size())(", TotalObj = ")(dbgc)("\n").str().c_str());

	// Return the total number of items that were located
	return (outResult.size());
}

