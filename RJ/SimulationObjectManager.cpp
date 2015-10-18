#include "Octree.h"
#include "iSpaceObject.h"
#include "CapitalShipPerimeterBeacon.h"

#include "SimulationObjectManager.h"


// Static constants
const float SimulationObjectManager::SEARCH_DISTANCE_MULTIPLIER = 3.0f;		// Multiplier attached to all search queries to allow more efficient caching
const float SimulationObjectManager::MINIMUM_SEARCH_DISTANCE = 1000.0f;		// Minimum distance on searches, for caching purposes
const float SimulationObjectManager::MAXIMUM_SEARCH_DISTANCE = 100000.0f;	// Maximum search distance when increasing by the multiplier

// Default constructor
SimulationObjectManager::SimulationObjectManager(void)
{
	// Pre-allocate space for the frequently used internal vectors, for runtime efficiency
	search_candidate_nodes.reserve(1024);
	search_large_objects.reserve(64);

	// Initialise the search cache and preallocate to the maximum size
	m_cache_enabled = true;
	m_searchcache = std::vector<CachedSearchResults>(SimulationObjectManager::MAXIMUM_SEARCH_CACHE_SIZE, CachedSearchResults());
	m_nextcacheindex = 0;
	m_cachesize = 0;

	// Initialise the cache hit / miss counters, if debug logging is enabled
#	ifdef OBJMGR_DEBUG_MODE
		CACHE_HITS = CACHE_MISSES = 0;
#	endif
}

// Initialises the object manager at the start of a frame
void SimulationObjectManager::InitialiseFrame(void)
{
	// Reset the search cache for this frame (TODO: allow some persistence across frames for efficiency?)
	m_nextcacheindex = m_cachesize = 0;

	// Reset the cache hit / miss counters for this frame, if debug logging is enabled
#	ifdef OBJMGR_DEBUG_MODE
		CACHE_HITS = CACHE_MISSES = 0;
	
#		ifdef OBJMGR_LOG_DEBUG_OUTPUT
		OutputDebugString("\nObjMgr: Initialising cache for new frame\n");
#		endif
#	endif
}

// Primary internal object search method.  Searches for all items within the specified distance of a position.  Accepts the 
// appropriate Octree node as an input; this is derived or supplied by the various publicly-invoked methods
int SimulationObjectManager::_GetAllObjectsWithinDistance(	Octree<iSpaceObject*> *node, const D3DXVECTOR3 & position, float distance,
															std::vector<iSpaceObject*> & outResult, int options)
{
	/* For now, we wil take one of two approaches.  If the search distance fits entirely within this item's node (the very likely case)
	we can efficiently iterate over the items in this node and return them.  If it does not, we will keep moving up the tree until
	we find a parent node that does emcompass the search area, locate all items within this node & it's children, and then consider
	each of those in turn.  This could be made more efficient (and complex) in future by only considering specific nodes rather than
	just moving up the tree.  e.g. if we need to search further in +ve x direction then locate the relevant node(s) to the right of
	this one.
	*/

	// We a pointer to the relevant spatial partitioning Octree to search for objects.  If we don't have one, return nothing immediately
	if (!node) return 0;

	// Clear the fdcal object boundary flag.  If it was required, it has already been applied in the public method.  We want to remove it
	// here since it has no bearing on cache eligibility
	ClearBit(options, ObjectSearchOptions::IgnoreFocalObjectBoundary);

	// Make a record of whether we want to include target object boundaries, and then remove from the options string as well for the same reason
	bool ignore_target_boundaries = CheckBit_Single(options, ObjectSearchOptions::IgnoreTargetObjectBoundaries);
	ClearBit(options, ObjectSearchOptions::IgnoreTargetObjectBoundaries);

	// All comparisons will be based on squared distance to avoid costly sqrt operations
	float distsq = (distance * distance);

	/* Test whether a cached search result can be used without searching */
	if (m_cache_enabled)
	{
		// Test whether any cached search result contains our desired search as a subset; if so, we can return it immediately
		float cacheposdistsq; D3DXVECTOR3 cacheposdiff;
		for (std::vector<CachedSearchResults>::size_type i = 0; i < m_cachesize; ++i)
		{
			// We can only used cached results that were obtained using the same search options
			if (m_searchcache[i].Options != options) continue;

			// Determine squared distance between our current search object and the centre of the cached search
			cacheposdiff = (position - m_searchcache[i].Position);
			cacheposdistsq = (cacheposdiff.x*cacheposdiff.x) + (cacheposdiff.y*cacheposdiff.y) + (cacheposdiff.z*cacheposdiff.z);

			// We can use these cached results if (cachedsearchdistSQ > (searchdistSQ + posdifferenceSQ))
			if (m_searchcache[i].SearchDistanceSq > (distsq + cacheposdistsq))
			{
				// Clear the results vector 
				outResult.clear();
				int matches = 0;
				iObject *obj;

				// Add any items within the adjusted threshold
				float threshold = (distsq - cacheposdistsq);
				std::vector<CachedSearchResult>::const_iterator it_end = m_searchcache[i].Results.end();
				for (std::vector<CachedSearchResult>::const_iterator it = m_searchcache[i].Results.begin(); it != it_end; ++it)
				{
					// Add any items within the threshold
					if ((*it).DistanceSquared < threshold)
					{
						obj = Game::GetObjectByID((*it).ObjectID);
						if (obj)
						{
							outResult.push_back((iSpaceObject*)obj);
							++matches;
						}
					}
				}

				// Record this as a cache hit, if debug logging is enabled
#				ifdef OBJMGR_DEBUG_MODE
					++CACHE_HITS;

#					ifdef OBJMGR_LOG_DEBUG_OUTPUT
						OutputDebugString(concat("ObjMgr: CACHE HIT { Search(")(distance)(" of [")(pos.x)(",")(pos.y)(",")(pos.z)("], opt=")(options)
							(") met by cache ")(i)(" (")(sqrtf(m_searchcache[i].SearchDistanceSq))(" of [")(m_searchcache[i].Position.x)(",")
							(m_searchcache[i].Position.y)(",")(m_searchcache[i].Position.z)("], opt=")(m_searchcache[i].Options)(") }\n").str().c_str());
#					endif
#				endif

				// Return the number of matches and quit
				return matches;
			}
		}
	}

	/* We cannot use a cached search result so perform a proximity search */

	// Record this as a cache miss, if debug logging is enabled
#	ifdef OBJMGR_DEBUG_MODE
		++CACHE_MISSES;

#		ifdef OBJMGR_LOG_DEBUG_OUTPUT
			OutputDebugString(concat("ObjMgr: Cache Miss { Search(")(distance)(" of [")(pos.x)(",")(pos.y)(",")(pos.z)("], opt=")(options)
				(") could not be met\n").str().c_str());
#		endif
#	endif

	// Increase actual search distance to enable more effective use of caching
	float search_distance = (distance * SimulationObjectManager::SEARCH_DISTANCE_MULTIPLIER);
	if (search_distance < SimulationObjectManager::MINIMUM_SEARCH_DISTANCE)
	{
		search_distance = SimulationObjectManager::MINIMUM_SEARCH_DISTANCE;
	}
	else if (search_distance > SimulationObjectManager::MAXIMUM_SEARCH_DISTANCE)
	{
		search_distance = max(distance, SimulationObjectManager::MAXIMUM_SEARCH_DISTANCE);
	}

	// We are ready to perform a full search; initialise the cache that we will be creating
	CachedSearchResults & cache = m_searchcache[m_nextcacheindex];
	cache.Position = position;
	cache.SearchDistanceSq = (search_distance * search_distance);
	cache.Options = options;
	cache.Results.clear();

	// Record the new cache that was created if in debug mode
#	if defined(OBJMGR_DEBUG_MODE) && defined(OBJMGR_LOG_DEBUG_OUTPUT)
		OutputDebugString(concat("ObjMgr: Creating cache ")(m_nextcacheindex)(" (")(search_distance)(" of [")(pos.x)(",")(pos.y)(",")(pos.z)
			("], opt=")(options)("\n").str().c_str());
#	endif

	// Increment the cache counter for the next one that will be used
	if (++m_nextcacheindex == SimulationObjectManager::MAXIMUM_SEARCH_CACHE_SIZE)
		m_nextcacheindex = 0;
	else
		++m_cachesize;

	// Determine the octree bounds that we need to consider
	float xmin = (position.x - search_distance), ymin = (position.y - search_distance), zmin = (position.z - search_distance),
		  xmax = (position.x + search_distance), ymax = (position.y + search_distance), zmax = (position.z + search_distance);

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

	// We know that node contains our full search area (or is the root, i.e. all objects) so we want to get a vector 
	// of all items in its bounds.  Use a non-recursive vector substitute instead of normal recursive search for efficiency
	search_candidate_nodes.clear();
	search_candidate_nodes.push_back(node);

	// Initialise the results vector before starting
	int found = 0;
	outResult.clear();

	// Initialise any other variables needed for the analysis phase
	iSpaceObject *obj;
	float diffx, diffy, diffz, obj_dist_sq;
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
				if (!obj) continue;
				const D3DXVECTOR3 & objpos = obj->GetPosition();

				// We can ignore the object if it is non-colliding and we are only interested in colliding objects.  Or, if it IS
				// colliding, we can also ignore it if it is a passive collider and we only want active collider objects
				if ( (CheckBit_Single(options, ObjectSearchOptions::OnlyCollidingObjects) && (obj->GetCollisionMode() == Game::CollisionMode::NoCollision)) ||
					 (CheckBit_Single(options, ObjectSearchOptions::OnlyActiveColliders) && (obj->GetColliderType() != Game::ColliderType::ActiveCollider)) ) continue;

				// Ignore the object if we are only interested in objects of a certain disposition (hostile/friendly etc)

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
					std::vector<iSpaceObject*>::size_type n = (search_large_objects.size());
					if (n >= 2)		// We don't need to check if n==0 (obviously) or n==1 (since we are ignoring the last item)
					{
						--n;		// Ignore the final item
						search_found_large_object = false;
						for (std::vector<iSpaceObject*>::size_type i = 0; i < n; ++i) if (search_large_objects[i] == obj) { search_found_large_object = true; break; }
						if (search_found_large_object) continue;
					}

					// We have not recorded the object; add it to the list now
					search_large_objects.push_back(obj);
					search_last_large_obj = obj;

					// Get the squared distance from our position to the object
					diffx = (position.x - objpos.x), diffy = (position.y - objpos.y), diffz = (position.z - objpos.z);
					diffx *= diffx; diffy *= diffy; diffz *= diffz;
					obj_dist_sq = diffx + diffy + diffz;

					// Add the object to the search cache before culling items in the exact search range
					cache.Results.push_back(CachedSearchResult(obj->GetID(), obj_dist_sq));

					// The object is invalid if it is outside (distsq + collisionradiussq) of the position
					if (obj_dist_sq > (distsq + obj->GetCollisionSphereRadiusSq())) continue;
				}

				// Method 2 - normal objects: any other object type can be treated as a point object, and we use a simple dist ^ 2 test
				else
				{
					// Calculated the squared distance between ourself and this object
					diffx = (position.x - objpos.x), diffy = (position.y - objpos.y), diffz = (position.z - objpos.z);
					diffx *= diffx; diffy *= diffy; diffz *= diffz;
					obj_dist_sq = diffx + diffy + diffz;

					// Add the object to the search cache before culling items in the exact search range
					cache.Results.push_back(CachedSearchResult(obj->GetID(), obj_dist_sq));

					// Test object validity based upon its squared distance to the target position.  Optionally ignore 
					// the target object collision radius as well
					if (ignore_target_boundaries)
					{
						if (obj_dist_sq > distsq) continue; 
					}
					else
					{
						if (obj_dist_sq > (distsq + obj->GetCollisionSphereRadiusSq())) continue;
					}
				}

				// The object is valid; add it to the output vector
				outResult.push_back(obj);
			}
		}
	}

	// If in debug logging mode, determine and record how many objects were added to the last cache
#	if defined(OBJMGR_DEBUG_MODE) && defined(OBJMGR_LOG_DEBUG_OUTPUT)
		int debug_cache_i = (m_nextcacheindex - 1);
		if (debug_cache_i < 0) debug_cache_i = (SimulationObjectManager::MAXIMUM_SEARCH_CACHE_SIZE - 1); 
		OutputDebugString(concat("ObjMgr: Cache ")(debug_cache_i)(" populated with ")(m_searchcache[debug_cache_i].Results.size())(" objects\n").str().c_str());
#	endif

	// Return the total number of items that were located
	return ((int)outResult.size());
}