#include "Octree.h"
#include "iSpaceObject.h"
#include "CapitalShipPerimeterBeacon.h"

#include "SimulationObjectManager.h"


// Searches for all items within the specified distance of an object.  Returns the number of items located
// Allows use of certain flags to limit results during the search; more efficient that returning everything and then removing items later
int SimulationObjectManager::GetAllObjectsWithinDistance(const iSpaceObject *focalobject, float distance, std::vector<iSpaceObject*> & outResult,
														 int options)
{
	/* For now, we wil take one of two approaches.  If the search distance fits entirely within this item's node (the very likely case)
	we can efficiently iterate over the items in this node and return them.  If it does not, we will keep moving up the tree until
	we find a parent node that does emcompass the search area, locate all items within this node & it's children, and then consider
	each of those in turn.  This could be made more efficient (and complex) in future by only considering specific nodes rather than
	just moving up the tree.  e.g. if we need to search further in +ve x direction then locate the relevant node(s) to the right of
	this one.
	*/
	//unsigned int start_time = (unsigned int)timeGetTime(); // DBG

	// We must be a member of the spatial partitioning Octree to search for objects.  If not, return nothing immediately
	Octree<iSpaceObject*> * node = focalobject->GetSpatialTreeNode();
	if (!node) return 0;

	// If we want to test from the focal object's boundary then add that boundary to the search distance now
	if (CheckBit_Single(options, ObjectSearchOptions::IncludeFocalObjectBoundary)) distance += focalobject->GetCollisionSphereRadius();

	// Get the position of this object.  All comparisons will be based on squared distance to avoid costly sqrt operations
	const D3DXVECTOR3 & pos = focalobject->GetPosition();
	float distsq = (distance * distance);
	float xmin = (pos.x - distance), ymin = (pos.y - distance), zmin = (pos.z - distance),
		  xmax = (pos.x + distance), ymax = (pos.y + distance), zmax = (pos.z + distance);

	// Starting point for the search will be our current node: "node" is currently set to the focal object treenode

	// Determine whether we can take approach 1, i.e. considering only the contents of our current node
	if ((xmin >= node->m_xmin) && (xmax < node->m_xmax) && (ymin >= node->m_ymin) &&
		(ymax < node->m_ymax) && (zmin >= node->m_zmin) && (zmax < node->m_zmax))
	{
		// The search area does fit within this node, so we can take approach 1.  Make sure we have any items besides ourself; 
		// if not, we can stop here
		if (node->m_itemcount <= 1) return 0;

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
	search_candidate_nodes.clear();
	search_candidate_nodes.push_back(node);

	// Initialise the results vector before starting
	int found = 0;
	outResult.clear();

	// Initialise any other variables needed for the analysis phase
	iSpaceObject *obj;
	float diffx, diffy, diffz;
	search_last_large_obj = NULL;
	int C;

	// Search nodes from the left ("index"), and add new search candidates to the right.  Only node in place at the start
	// will be the selected "node" that encompasses the entire search area
	int index = -1, count = 1;
	while (++index < count)
	{
		// Process the node.  Different action for branch vs leaf nodes
		node = search_candidate_nodes[index]; if (!node) continue;
		if (node->IsBranchNode())
		{
			// If this is a branch node, work out which of its children are actually relevant to us
			const D3DXVECTOR3 & centre = node->m_centre; 
			C = (NodeSubdiv::X_BOTH | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_BOTH);
			if		(xmin >= centre.x)	ClearBit(C, NodeSubdiv::X_NEG_SEG);
			else if (xmax < centre.x)	ClearBit(C, NodeSubdiv::X_POS_SEG);
			if		(ymin >= centre.y)	ClearBit(C, NodeSubdiv::Y_NEG_SEG);
			else if (ymax < centre.y)	ClearBit(C, NodeSubdiv::Y_POS_SEG);
			if		(zmin >= centre.z)	ClearBit(C, NodeSubdiv::Z_NEG_SEG);
			else if (zmax < centre.z)	ClearBit(C, NodeSubdiv::Z_POS_SEG);

			// Now add only those child nodes which are still considered relevant.  Unrolled as switch/lookup table for efficiency
			switch (C)
			{
				case (NodeSubdiv::X_NEG_SEG | NodeSubdiv::Y_NEG_SEG | NodeSubdiv::Z_NEG_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_DOWN]); ++count; break;
				case (NodeSubdiv::X_NEG_SEG | NodeSubdiv::Y_NEG_SEG | NodeSubdiv::Z_BOTH) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_DOWN]);
					count += 2; break;
				case (NodeSubdiv::X_NEG_SEG | NodeSubdiv::Y_NEG_SEG | NodeSubdiv::Z_POS_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_DOWN]); ++count; break;
				case (NodeSubdiv::X_NEG_SEG | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_NEG_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_DOWN]);
					count += 2; break;
				case (NodeSubdiv::X_NEG_SEG | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_BOTH) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_DOWN]);
					count += 4; break;
				case (NodeSubdiv::X_NEG_SEG | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_POS_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_DOWN]);
					count += 2; break;
				case (NodeSubdiv::X_NEG_SEG | NodeSubdiv::Y_POS_SEG | NodeSubdiv::Z_NEG_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_UP]); ++count; break;
				case (NodeSubdiv::X_NEG_SEG | NodeSubdiv::Y_POS_SEG | NodeSubdiv::Z_BOTH) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_UP]);
					count += 2; break;
				case (NodeSubdiv::X_NEG_SEG | NodeSubdiv::Y_POS_SEG | NodeSubdiv::Z_POS_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_UP]); ++count; break;
				case (NodeSubdiv::X_BOTH | NodeSubdiv::Y_NEG_SEG | NodeSubdiv::Z_NEG_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_DOWN]);
					count += 2; break;
				case (NodeSubdiv::X_BOTH | NodeSubdiv::Y_NEG_SEG | NodeSubdiv::Z_BOTH) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_DOWN]);
					count += 4; break;
				case (NodeSubdiv::X_BOTH | NodeSubdiv::Y_NEG_SEG | NodeSubdiv::Z_POS_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_DOWN]);
					count += 2; break;
				case (NodeSubdiv::X_BOTH | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_NEG_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_DOWN]);
					count += 4; break;
				case (NodeSubdiv::X_BOTH | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_BOTH) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_DOWN]);
					count += 8; break;
				case (NodeSubdiv::X_BOTH | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_POS_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_DOWN]);
					count += 4; break;
				case (NodeSubdiv::X_BOTH | NodeSubdiv::Y_POS_SEG | NodeSubdiv::Z_NEG_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_UP]);
					count += 2; break;
				case (NodeSubdiv::X_BOTH | NodeSubdiv::Y_POS_SEG | NodeSubdiv::Z_BOTH) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SW_UP]);
					count += 4; break;
				case (NodeSubdiv::X_BOTH | NodeSubdiv::Y_POS_SEG | NodeSubdiv::Z_POS_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NW_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_UP]);
					count += 2; break;
				case (NodeSubdiv::X_POS_SEG | NodeSubdiv::Y_NEG_SEG | NodeSubdiv::Z_NEG_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_DOWN]); ++count; break;
				case (NodeSubdiv::X_POS_SEG | NodeSubdiv::Y_NEG_SEG | NodeSubdiv::Z_BOTH) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_DOWN]);
					count += 2; break;
				case (NodeSubdiv::X_POS_SEG | NodeSubdiv::Y_NEG_SEG | NodeSubdiv::Z_POS_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_DOWN]); ++count; break;
				case (NodeSubdiv::X_POS_SEG | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_NEG_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_DOWN]);
					count += 2; break;
				case (NodeSubdiv::X_POS_SEG | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_BOTH) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_DOWN]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_DOWN]);
					count += 4; break;
				case (NodeSubdiv::X_POS_SEG | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_POS_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_DOWN]);
					count += 2; break;
				case (NodeSubdiv::X_POS_SEG | NodeSubdiv::Y_POS_SEG | NodeSubdiv::Z_NEG_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_UP]); ++count; break;
				case (NodeSubdiv::X_POS_SEG | NodeSubdiv::Y_POS_SEG | NodeSubdiv::Z_BOTH) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_UP]);
					search_candidate_nodes.push_back(node->m_children[OCTREE_SE_UP]);
					count += 2; break;
				case (NodeSubdiv::X_POS_SEG | NodeSubdiv::Y_POS_SEG | NodeSubdiv::Z_POS_SEG) :
					search_candidate_nodes.push_back(node->m_children[OCTREE_NE_UP]); ++count; break;
			}
		}
		else
		{
			// Otherwise, this is a leaf node.  Process each item within it
			search_obj_end = node->m_items.end();
			for (search_obj_it = node->m_items.begin(); search_obj_it != search_obj_end; ++search_obj_it)
			{
				// We can ignore this entry if it does not represent an item, or if it is us
				obj = (iSpaceObject*)(*search_obj_it);
				if (!obj || obj == focalobject) continue;
				const D3DXVECTOR3 & objpos = obj->GetPosition();

				// We can also ignore it if the item is non-colliding and we are only interested in colliding objects
				if (CheckBit_Single(options, ObjectSearchOptions::OnlyCollidingObjects) && (obj->GetCollisionMode() == Game::CollisionMode::NoCollision)) continue;

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
					if (search_last_large_obj == NULL)		search_large_objects.clear();
					else if (search_last_large_obj == obj)	continue;

					// Check whether we have already recorded this object; ignore the final entry since this was "last_large_obj"
					int n = (search_large_objects.size() - 1);
					search_found_large_object = false;
					for (int i = 0; i < n; ++i) if (search_large_objects[i] == obj) { search_found_large_object = true; break; }
					if (search_found_large_object) continue;

					// We have not recorded the object; add it to the list now
					search_large_objects.push_back(obj);
					search_last_large_obj = obj;

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
					if (CheckBit_Single(options, ObjectSearchOptions::IncludeTargetObjectBoundaries))
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

	/*static int dbgc = 0;
	dbgc += outResult.size();
	OutputDebugString(concat("Obj \"")(focalobject->GetName())("\" (")(focalobject->GetID())(") - NodeSearch: ")(search_candidate_nodes.size())
		(", ObjResults: ")(outResult.size())(", Time = ")(((unsigned int)timeGetTime() - start_time))("ms\n").str().c_str());
*/
	// Return the total number of items that were located
	return (outResult.size());
}