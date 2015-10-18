#pragma once

#ifndef __ObjectManagerH__
#define __ObjectManagerH__

#include "Utility.h"
#include "Octree.h"
#include "iSpaceObject.h"

// Debug options that will store or report additional data.  Only available in debug builds
#ifdef _DEBUG
#	define OBJMGR_DEBUG_MODE			// Records additional debug data if enabled
//#	define OBJMGR_LOG_DEBUG_OUTPUT		// Logs detailed cache data to the debug output each frame. Requires OBJMGR_DEBUG_MODE
#endif


class SimulationObjectManager
{
public:

	// Static constants
	static const float					SEARCH_DISTANCE_MULTIPLIER;
	static const float					MINIMUM_SEARCH_DISTANCE;
	static const float					MAXIMUM_SEARCH_DISTANCE;

	// Bit flags specifying object search options
	enum ObjectSearchOptions
	{
		NoSearchOptions					= 0x00,
		OnlyCollidingObjects			= 0x01,
		IgnoreFocalObjectBoundary		= 0x02,				// Removed from search caching
		IgnoreTargetObjectBoundaries	= 0x04,				// Removed from search caching
		OnlyActiveColliders				= 0x08
	};

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
	struct CachedSearchResult
	{
		Game::ID_TYPE 						ObjectID;
		float								DistanceSquared;

		CachedSearchResult(Game::ID_TYPE obj_id, float distsq) : ObjectID(obj_id), DistanceSquared(distsq) { }
	};

	// Structure holding cached object search information
	struct CachedSearchResults
	{
		D3DXVECTOR3							Position;
		float								SearchDistanceSq;
		SearchOptions						Options;
		std::vector<CachedSearchResult>		Results;

		CachedSearchResults(void) : Position(NULL_VECTOR), SearchDistanceSq(0.0f), Options(0) { }
	};

	// Maximum accepted search cache size
	static const std::vector<CachedSearchResults>::size_type		MAXIMUM_SEARCH_CACHE_SIZE = 32;

	// Default constructor
	SimulationObjectManager(void);

	// Initialises the object manager at the start of a frame
	void InitialiseFrame(void);

	// Searches for all items within the specified distance of an object.  Returns the number of items located
	// Allows use of certain flags to limit results during the search; more efficient that returning everything and then removing items later
	CMPINLINE int GetAllObjectsWithinDistance(const iSpaceObject *focalobject, float distance, std::vector<iSpaceObject*> & outResult, int options)
	{
		if (focalobject) return _GetAllObjectsWithinDistance(focalobject->GetSpatialTreeNode(), focalobject->GetPosition(), 
			(CheckBit_Single(options, ObjectSearchOptions::IgnoreFocalObjectBoundary) ? distance : distance + focalobject->GetCollisionSphereRadius()),
			outResult, options);
		else return 0;
	}

	// Searches for all items within the specified distance of a position.  Returns the number of items located
	// Allows use of certain flags to limit results during the search; more efficient that returning everything and then removing items later
	// Requires us to locate the most relevant Octree node, so less efficient than the method that supplies a space object
	CMPINLINE int GetAllObjectsWithinDistance(	const D3DXVECTOR3 & position, Octree<iSpaceObject*> *sp_tree, float distance, 
												std::vector<iSpaceObject*> & outResult, int options)
	{
		if (sp_tree) return _GetAllObjectsWithinDistance(sp_tree->GetNodeContainingPoint(position), position, distance, outResult, options);
		else return 0;
	}

	// Flag determining whether the object manager will maintain and return cached object search results where possible
	CMPINLINE bool								SearchCacheEnabled(void) const	{ return m_cache_enabled; }
	CMPINLINE void								DisableSearchCache(void)		{ m_cache_enabled = false; }
	CMPINLINE void								EnableSearchCache(void)			
	{
		// Enable caching and clear any previously-cached data so we have a clean starting point
		m_cache_enabled = true; 
		InitialiseFrame();
	}

	// Returns the current size of the search cache
	std::vector<CachedSearchResults>::size_type GetCurrentCacheSize(void) const { return m_cachesize; }


	// Debug output of number cache hits/missed
#ifdef OBJMGR_DEBUG_MODE
	int CACHE_HITS, CACHE_MISSES;
#endif

protected:

	// Primary internal object search method.  Searches for all items within the specified distance of a position.  Accepts the 
	// appropriate Octree node as an input; this is derived or supplied by the various publicly-invoked methods
	int _GetAllObjectsWithinDistance(Octree<iSpaceObject*> *node, const D3DXVECTOR3 & position, float distance, 
										std::vector<iSpaceObject*> & outResult, int options);

	// Data used in the process of searching for objects
	std::vector<Octree<iSpaceObject*>*> search_candidate_nodes;
	std::vector<iSpaceObject*>::const_iterator search_obj_end, search_obj_it;
	std::vector<iSpaceObject*> search_large_objects;
	iSpaceObject *search_last_large_obj;
	bool search_found_large_object;

	// Flag determining whether search caching is enabled or not
	bool											m_cache_enabled;

	// Collection of cached search results
	std::vector<CachedSearchResults>				m_searchcache;

	// Index of the next cached search result to be added
	std::vector<CachedSearchResults>::size_type		m_nextcacheindex;
	std::vector<CachedSearchResults>::size_type		m_cachesize;

};




#endif