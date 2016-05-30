#pragma once

#ifndef __ObjectSearchH__
#define __ObjectSearchH__

#include "Utility.h"
#include "Octree.h"
#include "ObjectReference.h"
#include "iObject.h"
#include "ComplexShipSection.h"
#include "CapitalShipPerimeterBeacon.h"

// Debug options that will store or report additional data.  Only available in debug builds
#ifdef _DEBUG
#	define OBJMGR_DEBUG_MODE			// Records additional debug data if enabled
//#	define OBJMGR_LOG_DEBUG_OUTPUT		// Logs detailed cache data to the debug output each frame. Requires OBJMGR_DEBUG_MODE
#endif

namespace Game
{
	// Bit flags specifying object search options
	enum ObjectSearchOptions
	{
		NoSearchOptions = 0x00,
		OnlyCollidingObjects = 0x01,
		IgnoreFocalObjectBoundary = 0x02,					// Removed from search caching
		IgnoreTargetObjectBoundaries = 0x04,				// Removed from search caching
		OnlyActiveColliders = 0x08
	};

	// Search instance, specialised for a specific node in the iObject hierarchy
	// Class is 16-bit aligned to allow use of SIMD member variables, via the use of ALIGN16
	// specification and AX... members, since can't use declspec on template class declaration
	template <class T>
	class ObjectSearch : public ALIGN16 <ObjectSearch<T>>
	{
	public:

		// Static constants
		static const float					SEARCH_DISTANCE_MULTIPLIER;
		static const float					MINIMUM_SEARCH_DISTANCE;
		static const float					MAXIMUM_SEARCH_DISTANCE;
		static const AXMVECTOR				SEARCH_DISTANCE_MULTIPLIER_V;
		static const AXMVECTOR				MINIMUM_SEARCH_DISTANCE_V;
		static const AXMVECTOR				MAXIMUM_SEARCH_DISTANCE_V;

		// Type definition for the search options bitstring
		typedef int SearchOptions;

		// Bit flags specifying the conditions for partitioning an Octree node during object search
		enum NodeSubdiv
		{
			X_NEG_SEG = 0x01,
			X_POS_SEG = 0x02,
			X_BOTH = (X_NEG_SEG | X_POS_SEG),
			Y_NEG_SEG = 0x04,
			Y_POS_SEG = 0x08,
			Y_BOTH = (Y_NEG_SEG | Y_POS_SEG),
			Z_NEG_SEG = 0x10,
			Z_POS_SEG = 0x20,
			Z_BOTH = (Z_NEG_SEG | Z_POS_SEG)
		};

		// Sturcture holding information on one cached search result
		// Class has no special alignment requirements
		struct CachedSearchResult
		{
			ObjectReference<T>					Object;
			float								DistanceSquared;

			CachedSearchResult(Game::ID_TYPE obj_id, float distsq) : Object(obj_id), DistanceSquared(distsq) { }
		};

		// Structure holding cached object search information
		// Class has no special alignment requirements
		struct CachedSearchResults
		{
			XMFLOAT3												Position;
			float													SearchDistanceSq;
			SearchOptions											Options;
			std::vector<CachedSearchResult>							Results;

			CachedSearchResults(void) : Position(NULL_FLOAT3), SearchDistanceSq(0.0f), Options(0) { }
		};

		// Maximum accepted search cache size
		static const typename std::vector<CachedSearchResults>::size_type	MAXIMUM_SEARCH_CACHE_SIZE = 32;

		// Static initialisation method
		static void Initialise(void);

		// Initialises the object manager at the start of a frame
		static void InitialiseFrame(void);

		// Searches for all items within the specified distance of an object.  Returns the number of items located
		// Allows use of certain flags to limit results during the search; more efficient that returning everything and then removing items later
		CMPINLINE static int GetAllObjectsWithinDistance(const T *focalobject, float distance, std::vector<T*> & outResult, SearchOptions options)
		{
			if (focalobject) return _GetAllObjectsWithinDistance(focalobject->GetSpatialTreeNode(), focalobject->GetPosition(),
				(CheckBit_Single(options, ObjectSearchOptions::IgnoreFocalObjectBoundary) ? distance : distance + focalobject->GetCollisionSphereRadius()),
				outResult, options);
			else return 0;
		}

		// Searches for all items within the specified distance of a position.  Returns the number of items located
		// Allows use of certain flags to limit results during the search; more efficient that returning everything and then removing items later
		// Requires us to locate the most relevant Octree node, so less efficient than the method that supplies a space object
		CMPINLINE static int GetAllObjectsWithinDistance(const FXMVECTOR position, Octree<T*> *sp_tree, float distance,
			std::vector<T*> & outResult, SearchOptions options)
		{
			if (sp_tree) return _GetAllObjectsWithinDistance(sp_tree->GetNodeContainingPoint(position), position, distance, outResult, options);
			else return 0;
		}

		// Performs a custom (non-cached) search with a specified radius, based on the supplied predicate
		template <typename UnaryPredicate>
		CMPINLINE static int	CustomSearch(const T *focalobject, float distance, std::vector<T*> outResult, UnaryPredicate Predicate)
		{
			if (focalobject)	return _CustomSearch<UnaryPredicate>(focalobject->GetSpatialTreeNode(), focalobject->GetPosition(), distance, outResult, Predicate);
			else				return 0;
		}

		// Performs a custom (non-cached) search with a specified radius, based on the supplied predicate
		template <typename UnaryPredicate>
		CMPINLINE static int	CustomSearch(const FXMVECTOR position, Octree<T*> *sp_tree, float distance, std::vector<T*> outResult, UnaryPredicate Predicate)
		{
			if (sp_tree)		return _CustomSearch<UnaryPredicate>(sp_tree->GetNodeContainingPoint(position), position, distance, outResult, Predicate);
			else				return 0;
		}

		// Flag determining whether the object manager will maintain and return cached object search results where possible
		CMPINLINE static bool						SearchCacheEnabled(void) 		{ return m_cache_enabled; }
		CMPINLINE static void						DisableSearchCache(void)		{ m_cache_enabled = false; }
		CMPINLINE static void						EnableSearchCache(void)
		{
			// Enable caching and clear any previously-cached data so we have a clean starting point
			m_cache_enabled = true;
			InitialiseFrame();
		}

		// Returns the current size of the search cache
		CMPINLINE static typename std::vector<CachedSearchResults>::size_type GetCurrentCacheSize(void) { return m_cachesize; }

		// Unary predicate for searching based on object type
		class ObjectIsOfType : public std::unary_function<T*, bool>
		{
		public:
			ObjectIsOfType(iObject::ObjectType _type) : type(_type){ }
			bool operator()(T* obj) const { return (obj->GetObjectType() == type); }

		protected:
			iObject::ObjectType type;
		};

		// Debug output of number cache hits/missed
#	ifdef OBJMGR_DEBUG_MODE
		static int CACHE_HITS, CACHE_MISSES;
#	endif

	protected:

		// Primary internal object search method.  Searches for all items within the specified distance of a position.  Accepts the 
		// appropriate Octree node as an input; this is derived or supplied by the various publicly-invoked methods
		static int _GetAllObjectsWithinDistance(Octree<T*> *node, const FXMVECTOR position, float distance,
												std::vector<T*> & outResult, SearchOptions options);

		// Primary custom search method.  Searches for all items within the specified distance of a position, using a custom
		// predicate to select results.  Accepts the appropriate Octree node as an input; this is derived or supplied 
		// by the various publicly-invoked methods
		template <typename UnaryPredicate>
		static int _CustomSearch(	Octree<T*> *node, const FXMVECTOR position, float distance,
									std::vector<T*> & outResult, UnaryPredicate Predicate)
		{
			// We a pointer to the relevant spatial partitioning Octree to search for objects.  If we don't have one, return nothing immediately
			if (!node) return 0;

			// All comparisons will be based on squared distance to avoid costly sqrt operations
			float distsq = (distance * distance);
			XMVECTOR vdistsq = XMVectorReplicate(distsq);

			// Determine the octree bounds that we need to consider
			XMVECTOR sdist = XMVectorReplicate(distance);
			XMVECTOR vmin = XMVectorSubtract(position, sdist);
			XMVECTOR vmax = XMVectorAdd(position, sdist);
			XMFLOAT3 fmin; XMStoreFloat3(&fmin, vmin);
			XMFLOAT3 fmax; XMStoreFloat3(&fmax, vmax);

			// Determine whether we can take approach 1, i.e. considering only the contents of our current node
			if (XMVector3GreaterOrEqual(vmin, node->m_min) && XMVector3Less(vmax, node->m_max))
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
					if (XMVector3GreaterOrEqual(vmin, node->m_min) && XMVector3Less(vmax, node->m_max))
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
			T *obj;	XMFLOAT3 centre; int C; float obj_dist_sq;

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
					XMStoreFloat3(&centre, node->m_centre);
					C = (NodeSubdiv::X_BOTH | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_BOTH);
					if (fmin.x >= centre.x)	ClearBit(C, NodeSubdiv::X_NEG_SEG);
					else if (fmax.x < centre.x)		ClearBit(C, NodeSubdiv::X_POS_SEG);
					if (fmin.y >= centre.y)	ClearBit(C, NodeSubdiv::Y_NEG_SEG);
					else if (fmax.y < centre.y)		ClearBit(C, NodeSubdiv::Y_POS_SEG);
					if (fmin.z >= centre.z)	ClearBit(C, NodeSubdiv::Z_NEG_SEG);
					else if (fmax.z < centre.z)		ClearBit(C, NodeSubdiv::Z_POS_SEG);

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
						obj = (*search_obj_it);
						if (!obj) continue;

						// Test whether the object is within our search distance
						obj_dist_sq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(position, obj->GetPosition())));
						if (obj_dist_sq > (distsq + obj->GetCollisionSphereRadiusSq())) continue;

						// Also test whether it meets the custom predicate condition
						if (!Predicate(obj)) continue;

						// The object is valid; add it to the output vector
						outResult.push_back(obj);
					}
				}
			}

			// Return the total number of items that were located
			return ((int)outResult.size());
		}


		// Data used in the process of searching for objects
		static std::vector<Octree<T*>*>									search_candidate_nodes;
		static typename std::vector<T*>::const_iterator					search_obj_end;
		static typename std::vector<T*>::const_iterator					search_obj_it;
		static std::vector<T*>											search_large_objects;
		static T *														search_last_large_obj;
		static bool														search_found_large_object;

		// Flag determining whether search caching is enabled or not
		static bool														m_cache_enabled;

		// Collection of cached search results
		static std::vector<CachedSearchResults>							m_searchcache;

		// Index of the next cached search result to be added
		typename static std::vector<CachedSearchResults>::size_type		m_nextcacheindex;
		typename static std::vector<CachedSearchResults>::size_type		m_cachesize;
	};


	// Static constants
	template <class T> const float ObjectSearch<T>::SEARCH_DISTANCE_MULTIPLIER = 3.0f;		// Multiplier attached to all search queries to allow more efficient caching
	template <class T> const float ObjectSearch<T>::MINIMUM_SEARCH_DISTANCE = 1000.0f;		// Minimum distance on searches, for caching purposes
	template <class T> const float ObjectSearch<T>::MAXIMUM_SEARCH_DISTANCE = 100000.0f;	// Maximum search distance when increasing by the multiplier
	template <class T> const AXMVECTOR	ObjectSearch<T>::SEARCH_DISTANCE_MULTIPLIER_V = XMVectorReplicate(ObjectSearch<T>::SEARCH_DISTANCE_MULTIPLIER);
	template <class T> const AXMVECTOR	ObjectSearch<T>::MINIMUM_SEARCH_DISTANCE_V = XMVectorReplicate(ObjectSearch<T>::MINIMUM_SEARCH_DISTANCE);
	template <class T> const AXMVECTOR	ObjectSearch<T>::MAXIMUM_SEARCH_DISTANCE_V = XMVectorReplicate(ObjectSearch<T>::MAXIMUM_SEARCH_DISTANCE);

	// Static debug constants
#	ifdef OBJMGR_DEBUG_MODE
	template <class T> int ObjectSearch<T>::CACHE_HITS = 0;
	template <class T> int ObjectSearch<T>::CACHE_MISSES = 0;
#	endif

	// Static member initialisation
	template <class T> std::vector<Octree<T*>*> ObjectSearch<T>::search_candidate_nodes;
	template <class T> typename std::vector<T*>::const_iterator ObjectSearch<T>::search_obj_end;
	template <class T> typename std::vector<T*>::const_iterator ObjectSearch<T>::search_obj_it;
	template <class T> std::vector<T*> ObjectSearch<T>::search_large_objects = std::vector<T*>();
	template <class T> T* ObjectSearch<T>::search_last_large_obj = NULL;
	template <class T> bool ObjectSearch<T>::search_found_large_object = false;
	template <class T> bool ObjectSearch<T>::m_cache_enabled = true;
	template <class T> std::vector<typename ObjectSearch<T>::CachedSearchResults> ObjectSearch<T>::m_searchcache;
	template <class T> typename std::vector<typename ObjectSearch<T>::CachedSearchResults>::size_type ObjectSearch<T>::m_nextcacheindex = 0;
	template <class T> typename std::vector<typename ObjectSearch<T>::CachedSearchResults>::size_type ObjectSearch<T>::m_cachesize = 0;


	// Static initialisation method
	template <class T>
	void ObjectSearch<T>::Initialise(void)
	{
		// Pre-allocate space for the frequently used internal vectors, for runtime efficiency
		search_candidate_nodes.reserve(1024);
		search_large_objects.reserve(64);

		// Initialise the search cache and preallocate to the maximum size
		m_cache_enabled = true;
		m_searchcache = std::vector<CachedSearchResults>(MAXIMUM_SEARCH_CACHE_SIZE, CachedSearchResults());
		m_nextcacheindex = 0;
		m_cachesize = 0;

		// Initialise the cache hit / miss counters, if debug logging is enabled
#	ifdef OBJMGR_DEBUG_MODE
		CACHE_HITS = CACHE_MISSES = 0;
#	endif
	}

	// Initialises the object manager at the start of a frame
	template <class T>
	void ObjectSearch<T>::InitialiseFrame(void)
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
	template <class T>
	int ObjectSearch<T>::_GetAllObjectsWithinDistance(Octree<T*> *node, const FXMVECTOR position, float distance,
		std::vector<T*> & outResult, int options)
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

		// Clear the focal object boundary flag.  If it was required, it has already been applied in the public method.  We want to remove it
		// here since it has no bearing on cache eligibility
		ClearBit(options, ObjectSearchOptions::IgnoreFocalObjectBoundary);

		// Make a record of whether we want to include target object boundaries, and then remove from the options string as well for the same reason
		bool ignore_target_boundaries = CheckBit_Single(options, ObjectSearchOptions::IgnoreTargetObjectBoundaries);
		ClearBit(options, ObjectSearchOptions::IgnoreTargetObjectBoundaries);

		// All comparisons will be based on squared distance to avoid costly sqrt operations
		float distsq = (distance * distance);
		XMVECTOR vdistsq = XMVectorReplicate(distsq);

		/* Test whether a cached search result can be used without searching */
		if (m_cache_enabled)
		{
			// Test whether any cached search result contains our desired search as a subset; if so, we can return it immediately
			XMVECTOR cacheposdiff, cacheposdistsq;
			for (std::vector<CachedSearchResults>::size_type i = 0; i < m_cachesize; ++i)
			{
				// We can only used cached results that were obtained using the same search options
				if (m_searchcache[i].Options != options) continue;

				// Determine squared distance between our current search object and the centre of the cached search
				cacheposdiff = XMVectorSubtract(position, XMLoadFloat3(&m_searchcache[i].Position));
				cacheposdistsq = XMVector3LengthSq(cacheposdiff);

				// We can use these cached results if (cachedsearchdistSQ > (searchdistSQ + posdifferenceSQ))
				// Use a vector2 comparison since the distance values are replicated across all elements anyway
				if (m_searchcache[i].SearchDistanceSq > XMVectorGetX(XMVectorAdd(vdistsq, cacheposdistsq)))
				{
					// Clear the results vector 
					outResult.clear();
					int matches = 0;

					// Add any items within the adjusted threshold
					float threshold = XMVectorGetX(XMVectorSubtract(vdistsq, cacheposdistsq));
					std::vector<CachedSearchResult>::const_iterator it_end = m_searchcache[i].Results.end();
					for (std::vector<CachedSearchResult>::const_iterator it = m_searchcache[i].Results.begin(); it != it_end; ++it)
					{
						// Add any items within the threshold
						const CachedSearchResult & result = (*it);
						if (result.DistanceSquared < threshold && result.Object())
						{
							outResult.push_back(result.Object());
							++matches;
						}
					}

					// Record this as a cache hit, if debug logging is enabled
#				ifdef OBJMGR_DEBUG_MODE
					++CACHE_HITS;

#					ifdef OBJMGR_LOG_DEBUG_OUTPUT
					OutputDebugString(concat("ObjMgr: CACHE HIT { Search(")(distance)(" of [")(XMVectorGetX(position))(",")(XMVectorGetY(position))(",")(XMVectorGetZ(position))("], opt=")(options)
						(") met by cache ")(i)(" (")(sqrtf(XMVectorGetX(m_searchcache[i].SearchDistanceSq)))(" of [")(XMVectorGetX(m_searchcache[i].Position))(",")
						(XMVectorGetY(m_searchcache[i].Position))(",")(XMVectorGetZ(m_searchcache[i].Position))("], opt=")(m_searchcache[i].Options)(") }\n").str().c_str());
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
		OutputDebugString(concat("ObjMgr: Cache Miss { Search(")(distance)(" of [")(position)(",")(position)(",")(position)("], opt=")(options)
			(") could not be met\n").str().c_str());
#		endif
#	endif

		// Increase actual search distance to enable more effective use of caching (distance *= SEARCH_DIST_MULTIPLIER)
		// Use vector2 comparisons for efficiency since all components are replicated anyway
		float search_distance = distance * ObjectSearch::SEARCH_DISTANCE_MULTIPLIER;
		if (search_distance < ObjectSearch::MINIMUM_SEARCH_DISTANCE)
		{
			search_distance = ObjectSearch::MINIMUM_SEARCH_DISTANCE;
		}
		else if (search_distance > ObjectSearch::MAXIMUM_SEARCH_DISTANCE)
		{
			search_distance = max(distance, ObjectSearch::MAXIMUM_SEARCH_DISTANCE);
		}

		// We are ready to perform a full search; initialise the cache that we will be creating
		CachedSearchResults & cache = m_searchcache[m_nextcacheindex];
		XMStoreFloat3(&cache.Position, position);
		cache.SearchDistanceSq = search_distance * search_distance;
		cache.Options = options;
		cache.Results.clear();

		// Record the new cache that was created if in debug mode
#	if defined(OBJMGR_DEBUG_MODE) && defined(OBJMGR_LOG_DEBUG_OUTPUT)
		OutputDebugString(concat("ObjMgr: Creating cache ")(m_nextcacheindex)(" (")(search_distance)(" of [")(XMVectorGetX(position))(",")
			(XMVectorGetY(position))(",")(XMVectorGetZ(position))("], opt=")(options)("\n").str().c_str());
#	endif

		// Increment the cache counter for the next one that will be used
		if (++m_nextcacheindex == ObjectSearch::MAXIMUM_SEARCH_CACHE_SIZE)
			m_nextcacheindex = 0;
		else
			++m_cachesize;

		// Determine the octree bounds that we need to consider
		XMVECTOR sdist = XMVectorReplicate(search_distance);
		XMVECTOR vmin = XMVectorSubtract(position, sdist);
		XMVECTOR vmax = XMVectorAdd(position, sdist);
		XMFLOAT3 fmin; XMStoreFloat3(&fmin, vmin);
		XMFLOAT3 fmax; XMStoreFloat3(&fmax, vmax);

		// Starting point for the search will be our current node: "node" is currently set to the focal object treenode

		// Determine whether we can take approach 1, i.e. considering only the contents of our current node
		if (XMVector3GreaterOrEqual(vmin, node->m_min) && XMVector3Less(vmax, node->m_max))
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
				if (XMVector3GreaterOrEqual(vmin, node->m_min) && XMVector3Less(vmax, node->m_max))
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
		T *obj;
		XMFLOAT3 centre;
		float obj_dist_sq;
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
				XMStoreFloat3(&centre, node->m_centre);
				C = (NodeSubdiv::X_BOTH | NodeSubdiv::Y_BOTH | NodeSubdiv::Z_BOTH);
				if (fmin.x >= centre.x)	ClearBit(C, NodeSubdiv::X_NEG_SEG);
				else if (fmax.x < centre.x)		ClearBit(C, NodeSubdiv::X_POS_SEG);
				if (fmin.y >= centre.y)	ClearBit(C, NodeSubdiv::Y_NEG_SEG);
				else if (fmax.y < centre.y)		ClearBit(C, NodeSubdiv::Y_POS_SEG);
				if (fmin.z >= centre.z)	ClearBit(C, NodeSubdiv::Z_NEG_SEG);
				else if (fmax.z < centre.z)		ClearBit(C, NodeSubdiv::Z_POS_SEG);

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
					obj = (*search_obj_it);
					if (!obj) continue;

					// We can ignore the object if it is non-colliding and we are only interested in colliding objects.  Or, if it IS
					// colliding, we can also ignore it if it is a passive collider and we only want active collider objects
					if ((CheckBit_Single(options, ObjectSearchOptions::OnlyCollidingObjects) && (obj->GetCollisionMode() == Game::CollisionMode::NoCollision)) ||
						(CheckBit_Single(options, ObjectSearchOptions::OnlyActiveColliders) && (obj->GetColliderType() != Game::ColliderType::ActiveCollider))) continue;

					// Ignore the object if we are only interested in objects of a certain disposition (hostile/friendly etc)

					// Special cases: if this is a capital ship beacon or ship section then we want to instead treat it like the parent capital ship
					if (obj->GetObjectType() == iObject::ObjectType::CapitalShipPerimeterBeaconObject)
					{
						obj = (T*)((CapitalShipPerimeterBeacon*)obj)->GetParentShip();
					}
					else if (obj->GetObjectType() == iObject::ObjectType::ComplexShipSectionObject)
					{
						obj = (T*)(((ComplexShipSection*)obj)->GetParent());
					}

					// Method 1 - Large objects: if this is a large object, avoid multiple-counting the objects based on any
					// beacons it may have deployed.  Test distance using a conservative test of (distsq + collradiussq) < targetdistsq, 
					// which may yield some false positives but which is much quicker at this stage than a proper distance test
					if (obj->GetObjectType() == iObject::ObjectType::ComplexShipObject)
					{
						// Check whether we have already found this object; continue to the next object if we have
						// Also clear the static large object vector if this is the first time we are using it
						if (search_last_large_obj == NULL)		search_large_objects.clear();
						else if (search_last_large_obj == obj)	continue;

						// Check whether we have already recorded this object; ignore the final entry since this was "last_large_obj"
						std::vector<T*>::size_type n = (search_large_objects.size());
						if (n >= 2)		// We don't need to check if n==0 (obviously) or n==1 (since we are ignoring the last item)
						{
							--n;		// Ignore the final item
							search_found_large_object = false;
							for (std::vector<T*>::size_type i = 0; i < n; ++i) if (search_large_objects[i] == obj) { search_found_large_object = true; break; }
							if (search_found_large_object) continue;
						}

						// We have not recorded the object; add it to the list now
						search_large_objects.push_back(obj);
						search_last_large_obj = obj;

						// Get the squared distance from our position to the object
						obj_dist_sq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(position, obj->GetPosition())));

						// Add the object to the search cache before culling items in the exact search range
						cache.Results.push_back(CachedSearchResult(obj->GetID(), obj_dist_sq));

						// The object is invalid if it is outside (distsq + collisionradiussq) of the position
						// Use vector2 comparison since values are replicated anyway
						if (obj_dist_sq > (distsq + obj->GetCollisionSphereRadiusSq())) continue;
					}

					// Method 2 - normal objects: any other object type can be treated as a point object, and we use a simple dist ^ 2 test
					else
					{
						// Calculated the squared distance between ourself and this object
						obj_dist_sq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(position, obj->GetPosition())));

						// Add the object to the search cache before culling items in the exact search range
						cache.Results.push_back(CachedSearchResult(obj->GetID(), obj_dist_sq));

						// Test object validity based upon its squared distance to the target position.  Optionally ignore 
						// the target object collision radius as well.  Use vector2 comparison since all components are replicated
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
		if (debug_cache_i < 0) debug_cache_i = (ObjectSearch::MAXIMUM_SEARCH_CACHE_SIZE - 1);
		OutputDebugString(concat("ObjMgr: Cache ")(debug_cache_i)(" populated with ")(m_searchcache[debug_cache_i].Results.size())(" objects\n").str().c_str());
#	endif

		// Return the total number of items that were located
		return ((int)outResult.size());
	}




	// Object search manager; coordinates the action of multiple ObjectSearch instances so that only one
	// method needs to be called externally for actions such as initialisation or shutdown
	// Class has no special alignment requirements
	class ObjectSearchManager
	{
	public:

		// Initialises object search capabilities
		CMPINLINE static void				Initialise(void)
		{
			// Update each search instance in turn
			ObjectSearch<iObject>::Initialise();
		}

		// Initialises object search instances at the start of a frame
		CMPINLINE static void				InitialiseFrame(void)
		{
			// Update each search instance in turn
			ObjectSearch<iObject>::InitialiseFrame();
		}
		
		// Enables the search cache for all object search instances
		CMPINLINE static void				EnableSearchCache(void)
		{
			// Update each search instance in turn
			ObjectSearch<iObject>::EnableSearchCache();
		}

		// Disables the search cache for all object search instances
		CMPINLINE static void				DisableSearchCache(void)
		{
			// Update each search instance in turn
			ObjectSearch<iObject>::DisableSearchCache();
		}

		// Returns the current size of the search cache across all search instances
		CMPINLINE static int DetermineTotalCurrentCacheSize(void) 
		{ 
			return (int)(
				ObjectSearch<iObject>::GetCurrentCacheSize() /* +
	   			ObjectSearch<...>::GetCurrentCacheSize() */
			);
		}

#ifdef OBJMGR_DEBUG_MODE		
		// Returns the current number of cache hits across all search instances
		CMPINLINE static int DetermineTotalCurrentCacheHits(void)
		{
			return (ObjectSearch<iObject>::CACHE_HITS /* + ObjectSearch<...>::... */);
		}

		// Returns the current size of the search cache across all search instances
		CMPINLINE static int DetermineTotalCurrentCacheMisses(void)
		{
			return (ObjectSearch<iObject>::CACHE_MISSES /* + ObjectSearch<...>::... */);
		}
#endif

	};

}


#endif