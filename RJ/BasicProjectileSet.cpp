#include "GameVarsExtern.h"
#include "ViewFrustrum.h"
#include "ObjectSearch.h"

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
void BasicProjectileSet::AddProjectile(	const BasicProjectileDefinition *def, Game::ID_TYPE owner, const FXMVECTOR position,
										const FXMVECTOR orientation, const FXMVECTOR base_world_velocity)
{
	// If we are not active then initialise now; first projectile will be created at [0]
	if (!Active)
	{
		Active = true;
		LiveIndex = 0;
		m_threshold_crossed = (0U - 1U);		// UINT_MAX
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
	Items[LiveIndex] = BasicProjectile(def, owner, position, orientation, base_world_velocity);
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

// Simulate all projectiles.  (1) Remove any expired projectiles, (2) handle collisions and (3) move the projectiles
// along their velocity vector.  Accepts a pointer to the spatial partitioning tree for the current area as input
void BasicProjectileSet::SimulateProjectiles(Octree<iObject*> *sp_tree)
{
	// Make sure this collection is active and that we have been passed a valid SP tree
	if (!Active || !sp_tree) return;

	// Define variables required later in the method
	Octree<iObject*> *leaf = NULL;
	std::vector<iObject*> contacts;
	XMVECTOR delta_pos;
	bool collision;

	// Loop through the projectile collection until we reach the LiveIndex (the last active projectile)
	std::vector<BasicProjectile>::size_type i = 0;
	while (i <= LiveIndex && Active)
	{
		// Get a reference to this projectile
		BasicProjectile & proj = Items[i];

		/* 1. Test whether the projectile has expired */
		if (Game::ClockMs > proj.Expiration)
		{
			// Projectile has expired; remove it from the collection and do not increment i, since the last 'live'
			// projectile has now been swapped into this position
			RemoveProjectile(i); continue;
		}
		
		// Slight hack; if the current clock time equals the projectile launch time this is the first frame in which it
		// has been active.  Don't do any simulation in this first frame so it is rendered from its starting location (and not 
		// one velocity step ahead) and move onto the next projectile
		if (Game::ClockMs == proj.LaunchTime) { ++i;  continue; }


		/* 2. Otherwise if the projectile is still active, test collision for any objects in the path it is about to take */
			
		// Determine the relevant octree leaf node for this projectile.  Test the last node that was used as a
		// first approximation, since many projectiles may exist in the same general area
		if (!leaf || !leaf->ContainsPoint(proj.Position))
		{
			leaf = sp_tree->GetNodeContainingPoint(proj.Position);
		}

		// Determine the position delta vector for this frame; this is the velocity/sec * the timefactor
		delta_pos = XMVectorMultiply(proj.Velocity, Game::TimeFactorV);

		// Get all contacts potentially within the path of this object
		int count = Game::ObjectSearch<iObject>::GetAllObjectsWithinDistance(proj.Position, leaf, proj.Speed,
						contacts, Game::ObjectSearchOptions::OnlyCollidingObjects);

		// Test each contact to see if it is actually intersected by the projectile path
		if (count != 0)
		{
			collision = false;
			std::vector<iObject*>::iterator it_end = contacts.end();
			for (std::vector<iObject*>::iterator it = contacts.begin(); it != it_end; ++it)
			{
				// Prevent the projectile from colliding with its owner
				iObject *obj = (*it);
				if (obj->GetID() == proj.Owner) continue;

				// Test for bounding sphere intersection along the projectile path
				if (Game::PhysicsEngine.TestLineVectorvsSphereIntersection(proj.Position, delta_pos,
					obj->GetPosition(), obj->GetCollisionSphereRadius()))
				{
					// This is a potential (broadphase) collision; perform a precise collision test 
					// against the object OBB hierarchy to confirm
					if (Game::PhysicsEngine.DetermineLineVectorVsOBBHierarchyIntersection(proj.Position, delta_pos, obj->CollisionOBB))
					{
						// The projectile is impacting this object; handle and render the impact accordingly
						obj->HandleProjectileImpact(proj, Game::PhysicsEngine.OBBIntersectionResult);

						// Record the collision and quit here; we cannot collide with more than one object
						collision = true; break;
					}
				}
			}

			// If we collided with an object then remove the projectile and move to the next one, since there will be nothing to render
			if (collision) { RemoveProjectile(i); continue; }
		}

		/* 3. Since the projectile hasn't collided we can move it along its velocity vector to its new position */
		// TODO: If velocity vector is only re-tranformed from basis vector by orientation once at start, changes in orientation mid-flight
		// will have no effect on the projectile trajectory.  This may be fine for basic projectiles
		proj.Position = XMVectorAdd(proj.Position, delta_pos);

		// Finally, increment the collection index to move onto the next projectile
		++i;
	}
}




