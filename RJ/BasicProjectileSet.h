#pragma once

#ifndef __BasicProjectileSetH__
#define __BasicProjectileSetH__

#include <vector>
#include "BasicProjectile.h"

// Extends the vector class
class BasicProjectileSet
{
public:

	// Constants used by this projectile set
	static const std::vector<BasicProjectile>::size_type	DEFAULT_INITIAL_CAPACITY = 256;		// Starting item capacity
	static const std::vector<BasicProjectile>::size_type	MAXIMUM_CAPACITY = 16384;			// Maximum collection size
	static const unsigned int								DEALLOCATION_TIME = 60000U;			// If we have been below the half-threshold for this length of 
																								// time (ms) we can shrink the item collection

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
	void													AddProjectile(	const BasicProjectileDefinition *def, const D3DXVECTOR3 & position,
																			const D3DXVECTOR3 & velocity, unsigned int lifetime);

	// Removes the projectile at the specified index
	void													RemoveProjectile(std::vector<BasicProjectile>::size_type index);

	// Extends the size of the collection to allow more elements to be added, as long as we are not at the limit
	// Returns a flag indicating whether the collection could be extended
	bool													ExtendCollection(void);

	// Shrinks the collection to half of its original size.  Resets the threshold counter
	void													ShrinkCollection(void);


protected:

	// Record the half-item threshold that we use to determine when to possibly shrink the item vector
	std::vector<BasicProjectile>::size_type					m_half_threshold;

	// Record the time at which we dipped and stayed below the half-capacity threshold
	unsigned int											m_threshold_crossed;
};




#endif




