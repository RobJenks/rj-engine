#pragma once

#ifndef __BasicProjectileSetH__
#define __BasicProjectileSetH__

#include <vector>
#include "BasicProjectile.h"

// Extends the vector class
// This class has no special alignment requirements
class BasicProjectileSet
{
public:

	// Constants used by this projectile set
	static const std::vector<BasicProjectile>::size_type	DEFAULT_INITIAL_CAPACITY = 256;		// Starting item capacity
	static const std::vector<BasicProjectile>::size_type	MAXIMUM_CAPACITY = 16384;			// Maximum collection size
	static const unsigned int								DEALLOCATION_TIME = 60000U;			// If we have been below the half-threshold for this length of 
																								// time (ms) we can shrink the item collection
	static const float										COLLISION_SEARCH_RADIUS;			// Distance within which we will perform projectile collision queries
	static const float										COLLISION_COMMON_OBJECT_RADIUS_SQ;	// If projectiles are within this same sq-distance of each
	
	// Primary item collection
	std::vector<BasicProjectile>							Items;

	// Flag that indicates whether this collection is active
	bool													Active;

	// Index of the last 'live' projectile, i.e. if we have 10 active projectiles this will == 9
	// Whenever a projectile 'dies' it will then be swapped with the last live projectile, and --index == 8
	std::vector<BasicProjectile>::size_type					LiveIndex;

	// Records the current capacity (vector.size()) of this collection
	std::vector<BasicProjectile>::size_type					Capacity;

	// Initialisation method; can optionally specify estimated initial capacity for memory reservation
	void													Initialise(void);
	void													Initialise(std::vector<BasicProjectile>::size_type initial_capacity);

	// Adds a new projectile to the collection
	void													AddProjectile(	const BasicProjectileDefinition *def, Game::ID_TYPE owner, const FXMVECTOR position,
																			const FXMVECTOR orientation, unsigned int lifetime, const FXMVECTOR base_world_velocity);

	// Removes the projectile at the specified index
	void													RemoveProjectile(std::vector<BasicProjectile>::size_type index);

	// Extends the size of the collection to allow more elements to be added, as long as we are not at the limit
	// Returns a flag indicating whether the collection could be extended
	bool													ExtendCollection(void);

	// Shrinks the collection to half of its original size.  Resets the threshold counter
	void													ShrinkCollection(void);

	// Simulate all projectiles.  Remove any expired projectiles, handle collisions and mark any in-view projectiles for rendering
	// Accepts a pointer to the spatial partitioning tree for the current area as input
	void													SimulateProjectiles(Octree<iSpaceObject*> *sp_tree);

	// Returns the number of projectiles currently active in the set
	CMPINLINE std::vector<BasicProjectile>::size_type		GetActiveProjectileCount(void) const
	{
		return (Active ? (LiveIndex + 1U) : 0U);
	}

protected:

	// Record the half-item threshold that we use to determine when to possibly shrink the item vector
	std::vector<BasicProjectile>::size_type					m_half_threshold;

	// Record the time at which we dipped and stayed below the half-capacity threshold
	unsigned int											m_threshold_crossed;
};




#endif




