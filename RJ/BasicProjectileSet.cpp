#include "GameVarsExtern.h"

#include "BasicProjectileSet.h"

// Initialisation method
void BasicProjectileSet::Initialise(void)
{
	Initialise(BasicProjectileSet::DEFAULT_INITIAL_CAPACITY);
}

// Initialisation method; can specify estimated initial capacity for memory reservation
void BasicProjectileSet::Initialise(std::vector<BasicProjectile>::size_type initial_capacity)
{
	// Pre-allocate elements for efficiency at runtime
	Items = std::vector<BasicProjectile>(initial_capacity, BasicProjectile());

	// Initial capacity should be divisible by two for convenience
	Capacity = ((initial_capacity % 2 == 0) ? initial_capacity : initial_capacity + 1);
	m_half_threshold = ((Capacity / 2) - 1);

	// Collection will start as inactive.  'Live index' indicates the positon of the first 'live' projectile and can be set 
	// to anything at initialisation since we are not active
	Active = false;
	LiveIndex = 0;
}

// Adds a new projectile to the collection
void BasicProjectileSet::AddProjectile(	const BasicProjectileDefinition *def, const D3DXVECTOR3 & position,
										const D3DXVECTOR3 & velocity, unsigned int lifetime)
{
	// If we are not active then initialise now; first projectile will be created at [0]
	if (!Active)
	{
		Active = true;
		LiveIndex = 0;
		m_threshold_crossed = (0U - 1);		// UINT_MAX
	}
	else
	{
		// Otherwise, if we were active, make sure we aren't now going over the collection capacity by adding a new element
		if (++LiveIndex == Capacity)
		{
			// This method will return false if we cannot extend the capacity; if this is the case quit here since we cannot add the projectile
			if (ExtendCollection() == false)
			{
				--LiveIndex;
				return;
			}
		}
	}

	// The "LiveIndex" now points to the next suitable element for creating a projectile.  Set the projectile details
	Items[LiveIndex].Definition = def;
	Items[LiveIndex].Position = position;
	Items[LiveIndex].Velocity = velocity;
	Items[LiveIndex].Lifetime = lifetime;
}

// Extends the size of the collection to allow more elements to be added, as long as we are not at the limit
// Returns a flag indicating whether the collection could be extended
bool BasicProjectileSet::ExtendCollection(void)
{
	// Determine the desired new capacity and make sure it is within allowable limits
	std::vector<BasicProjectile>::size_type new_capacity = Capacity * 2;
	if (new_capacity >= BasicProjectileSet::MAXIMUM_CAPACITY) return false;

	// We can proceed, so extend the item vector by doubling its current size
	Items.insert(Items.end(), Capacity, BasicProjectile());

	// Store the new capacity
	Capacity = new_capacity;

	// Determine the new half-capacity threshold and reset the counter for how long we have been below the threshold
	m_half_threshold = ((Capacity / 2) - 1);
	m_threshold_crossed = (0U - 1);				// UINT_MAX
	
	// Return success
	return true;
}

void BasicProjectileSet::RemoveProjectile(std::vector<BasicProjectile>::size_type index)
{
	// If we now have zero elements (i.e. this was our only active one) we are no longer active and can quit
	if (LiveIndex == 0)
	{
		Active = false;
		return;
	}

	// Overwrite the item with our last live projectile
	Items[index] = Items[LiveIndex]; 

	// Check if we are currently at the half-threshold.  If so, decrementing the item count now will take us below
	// the threshold and we should set the threshold counter
	if (LiveIndex == m_half_threshold)
	{
		m_threshold_crossed = Game::ClockMs;
	}

	// Now decrement the live index to remove the last item
	--LiveIndex;

	// Check if we have been below the threshold for the required amount of time to shrink the item collection
	if ((Game::ClockMs > m_threshold_crossed) && (Game::ClockMs - m_threshold_crossed) > BasicProjectileSet::DEALLOCATION_TIME)
	{
		ShrinkCollection();
	}
}


// Shrinks the collection to half of its original size.  Resets the threshold counter
void BasicProjectileSet::ShrinkCollection(void)
{
	// Reduce the capacity to half-capacity (i.e. halve it) and calculate a new half-capacity
	Capacity = m_half_threshold;
	m_half_threshold = (Capacity % 2 == 0 ? ((Capacity / 2) - 1) : ((Capacity - 1) / 2));

	// Reset the threshold-shrinking timer since we just shrunk the collection
	m_threshold_crossed = (0U - 1);			// UINT_MAX

	// Now actually shrink the item collection
	Items.resize(Capacity);
}


CREATE METHOD(S?) TO RUN THROUGH THE COLLECTION AND
a) Remove any projectiles that are now dead(lifetime < 0).  Cast Game::ClockMs to int once and then use int from that point so we can test for -ve
b) Test for any collisions with objects using line/sphere test
c) Render any objects within the frustum(check using sphere test, with speedSq radius [stored in proj def?  No because of pointer dereference?])






