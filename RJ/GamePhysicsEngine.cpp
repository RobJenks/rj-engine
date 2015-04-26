#include <vector>
#include "DX11_Core.h"

#include "Utility.h"
#include "FastMath.h"
#include "GameVarsExtern.h"
#include "Octree.h"
#include "iActiveObject.h"
#include "iSpaceObject.h"
#include "Ray.h"
#include "AABB.h"
#include "OrientedBoundingBox.h"
#include "iEnvironmentObject.h"
#include "Actor.h"
#include "Player.h"
#include "iSpaceObjectEnvironment.h"
#include "ComplexShipSection.h"
#include "StaticTerrain.h"

#include "GamePhysicsEngine.h"

// Compiler settings that, if defined, will cause all collision details to be logged to the debug output.  Only available in debug builds
#ifdef _DEBUG
//#	define RJ_LOG_COLLISION_DETAILS
//#	define RJ_LOG_PLAYER_TERRAIN_COLLISION_DETAILS
#	define RJ_NEW_COLLISION_HANDLING
//#	define RJ_OLD_COLLISION_HANDLING
//#	define RJ_OLD_COLLISION_HANDLING_2
#endif

// Default constructor
GamePhysicsEngine::GamePhysicsEngine(void)
{
	// Initialise fields to their default values wherever required
	m_static_cd_counter = 0U; m_cd_include_static = false;

	// Default physics engine flags
	m_flag_handle_diverging_collisions = false;
}

// Primary method to simulate all physics in the world.  Uses semi-fixed time step to maintain reasonably frame-rate 
// independent simulation results, with a simulation limiter to avoid the 'spiral of death' 
void GamePhysicsEngine::SimulatePhysics(void)
{
	// Initialise the clock state
	PhysicsClock.RemainingFrameTime = Game::TimeFactor;

	// Increment the counter for testing static/static object collisions, and set the flag if necessary
	if ((m_static_cd_counter += Game::ClockDelta) > Game::C_STATIC_PAIR_CD_INTERVAL)
		{ m_cd_include_static = true; m_static_cd_counter = 0U; }
	else
		m_cd_include_static = false;

	// Semi-fixed time step will be used to maintain relatively frame rate-independent simulation.  We will use a
	// target max-delta-time, but with provision to relax that limit in case it would result in the physics simulation
	// exceeding the maximum permitted cycles this frame
	PhysicsClock.FrameCycleTimeLimit = 
		((PhysicsClock.RemainingFrameTime * Game::C_MIN_PHYSICS_CYCLES_PER_SEC) <= Game::C_MAX_PHYSICS_FRAME_CYCLES ? // Would the desired physics-FPS take us over per-frame cycle limit?
		Game::C_MAX_PHYSICS_TIME_DELTA :																			  // No - so set the max time delta to that resulting from the FPS
		(PhysicsClock.RemainingFrameTime / Game::C_MAX_PHYSICS_FRAME_CYCLES));										  // Yes - so set max time delta to yield exactly the cycle limit of cycles

	// Potentially integrate the physics engine multiple times per frame, if the render time is sufficiently high
	while (PhysicsClock.RemainingFrameTime > 0.000000001f)
	{
		// Store the new physics engine delta time (this may only be a portion of the total frame time)
		PhysicsClock.TimeFactor = min(PhysicsClock.RemainingFrameTime, PhysicsClock.FrameCycleTimeLimit);

		// Perform a full cycle of physics simulation based on this delta time
		PerformPhysicsCycle();

		// Decrement the remaining frame time
		PhysicsClock.RemainingFrameTime -= PhysicsClock.TimeFactor;

		// *** We are not doing a semi-fixed timestep for now, the logic above can be removed later if it works across all FPS ***
		break;
	}
}

// Runs a full cycle of physics simulation.  Timing data for this cycle is held within the physics engine internal clock
// Physics cycle may differ in length from the full game cycles, if e.g. the physics engine fidelity is very high or if the
// render FPS gets too low
void GamePhysicsEngine::PerformPhysicsCycle(void)
{
	/*float magSq;
	bool fastmover;
	iObject *obj; iSpaceObject *sobj; iEnvironmentObject *eobj; Ship *shobj;

	// Iterate through all active game objects
	Game::ObjectRegister::iterator it_end = Game::Objects.end();
	for (Game::ObjectRegister::iterator it = Game::Objects.begin(); it != it_end; ++it)
	{
		// Get a reference to the object
		obj = (it->second); if (!obj) continue;

		// Take different action depending on the class of object
		switch (obj->GetObjectType())
		{
			case iObject::ObjectType::SimpleShipObject:
			case iObject::ObjectType::ComplexShipObject:
			{
				// Call the ship-specific update method
				shobj = (Ship*)obj;
				shobj->PerformPhysicsUpdate(PhysicsClock.TimeFactor);
				break;
			}
			/* case iObject::<OtherSpaceObjects> 
			case iObject::ObjectType::ActorObject:
			{
				eobj = (iEnvironmentObject*)obj;
				
			}
		}
*/
		
	// Simply perform collision detection for now
	PerformCollisionDetection(Game::CurrentPlayer->GetActivePlayerObject());
	
}

// Performs collision detection about the specified object.  Determines parameters for e.g. how far around the object we should
// be testing for collisions, and then passes control to the main collision detection method
void GamePhysicsEngine::PerformCollisionDetection(iObject *focalobject)
{
	// Parameter check
	if (!focalobject) return;

	// Reset the collision data logs for this frame
	CollisionDetectionResults.ClearData();

	// Use a different collision method & distance based upon the type of object being tested
	switch (focalobject->GetObjectType())
	{
		case iObject::ObjectType::SimpleShipObject:
		case iObject::ObjectType::ComplexShipObject:
		case iObject::ObjectType::ComplexShipSectionObject:

			// Perform ship-level collision detection, at a higher than average range due to increased visibility & speed in space
			PerformSpaceCollisionDetection((iSpaceObject*)focalobject, Game::C_ACTIVE_COLLISION_DISTANCE_SHIPLEVEL);
			break;


		case iObject::ObjectType::ActorObject:

			// Actor-level collisions can be performed with low active collision distance, since visibility should be much smaller
			iEnvironmentObject *envobj = (iEnvironmentObject*)focalobject;
			PerformEnvironmentCollisionDetection(	envobj->GetParentEnvironment(), envobj->GetEnvironmentPosition(), 
													Game::C_ACTIVE_COLLISION_DISTANCE_ACTORLEVEL);
			break;
	}
}


// Performs a full cycle of collision detection & collision response in a radius around the specified focal object (which is typically
// the player).  Use the object's octree to locate and test objects.  If radius < 0.0f then all objects in the tree will be considered 
// (which can be very inefficient).  We use a focal object rather than simply a position since the objects are automatically maintaining
// pointers to the relevant octree nodes during simulation.  Using a position value we would have to calculate the relevant node each frame.
void GamePhysicsEngine::PerformSpaceCollisionDetection(iSpaceObject *focalobject, float radius)
{
	std::vector<iSpaceObject*> objects;			// The list of objects being considered for collision detection
	std::vector<iSpaceObject*> candidates;		// The list of potential collisions around the object being tested
	iSpaceObject *object, *candidate;
	int numobjects, numcandidates;
	bool hasexclusions;							// Flag indicating whether the current object has any collision exclusions.  For efficiency
	bool exclude_static;						// Flag indicating whether we should exclude static/static pairs from the CD
	OrientedBoundingBox *collider0 = NULL, *collider1 = NULL;	// Oriented bounding boxes that are colliding, determined during narrowphase testing
	bool result;

	/* High-level process:
		1. Use octree to determine only those objects in the search radius about the focal object
		>  For each object in scope,
			2. Use octree to determine only those candidates close to the object
			>  For each other object in scope,
				3. Broadphase: Use bounding sphere test (with radius = max(size.x, size.y, size.z) to eliminate all but broadphase collision pairs
				               Record this item against the candidate as an object already tested.  Then when testing candidate we don't need to repeat.
				4. Narrowphase: Perform OBB narrowphase collision detection where applicable
	*/

	// Parameter check
	if (!focalobject) return;

	// Make sure this object is held within an octree, otherwise we cannot consider it for collision detection
	Octree<iSpaceObject*> *node = focalobject->GetSpatialTreeNode();
	if (!node) return;

	// 1. We want to retrieve the set of objects in scope for testing.  If radius < 0.0f, we select all objects from the root
	if (radius <= Game::C_EPSILON)
	{
		node->GetUltimateParent()->GetItems(objects);
		numobjects = objects.size();
	}
	else
	{
		// Perform a search outwards from the focal object to locate all objects within range.  Don't incorporate any object
		// boundaries at this point since we are just doing a coarse search of nearby objects
		numobjects = focalobject->GetAllObjectsWithinDistance(radius, objects, true, false, false);

		// Add the focal object (as long as it collides), since it will not be returned by the ObjectsWithinDistance method
		if (focalobject->GetCollisionMode() != Game::CollisionMode::NoCollision)
		{
			objects.push_back(focalobject);
			++numobjects;
		}
	}

	// 2. Now we want to consider each object in turn
	for (int i = 0; i < numobjects; ++i)
	{
		// Get a reference to this object
		object = objects[i]; if (!object) continue;

		// Test whether this object is moving at very high speed
		if (object->IsFastMover())
		{
			// If it is, we need to perform continuous collision detection (CDD) instead of the primary discrete method
			PerformContinuousSpaceCollisionDetection(object);
		}
		else
		{
			// Otherwise, in the majority of cases, we will handle this object via normal, discrete collision detection
			
			// Get any objects within the current object's collision margin radius; quit here if there are no objects nearby.  Don't
			// include the focal object boundary since this is already passed in the 'marginradius'.  Include all target object boundaries.
			candidates.clear();
			numcandidates = object->GetAllObjectsWithinDistance(object->GetCollisionSphereMarginRadius(), candidates, true, false, true);
			if (numcandidates == 0) continue;

			// Get basic information on the object that we will need for each comparison
			hasexclusions = object->HasCollisionExclusions();
			exclude_static = (!m_cd_include_static && object->IsStatic());

			// 3. Now consider each candidate in the broadphase collision detection, using simple collision sphere comparisons
			Game::ID_TYPE object_id = object->GetID();
			for (int c = 0; c < numcandidates; ++c)
			{
				// Get a reference to the candidate object
				candidate = candidates[c]; if (!candidate) continue;

				// We only want to test a collision between two objects once, i.e. we don't want to test (object vs candidate) and 
				// then (candidate vs object).  To do this efficiently we only test collisions where object.ID < candidate.ID.  The 
				// uniqueness and sequential nature of object IDs means this will always work
				if (object_id >= candidate->GetID()) continue;

				// Also test whether either object has an exclusion in place to prevent collision with the other
				if ((hasexclusions && object->CollisionExcludedWithObject(candidate->GetID())) ||
					(candidate->HasCollisionExclusions() && candidate->CollisionExcludedWithObject(object_id))) continue;

				// If the candidate is a 'fast-mover', don't test for a collision from this side.  All CCD collisions for this candidate
				// object will be covered in the CCD method when it is the primary object
				if (candidate->IsFastMover()) continue;

				// If the focal object is static, test whether the candidate is as well.  Only evaluate static pairs of objects on
				// a periodic basis, since the vast majority of the time they will not be colliding (unless they are somehow placed inside
				// each other
				if (exclude_static && candidate->IsStatic()) continue;

				// Record the fact that we are testing the collision pair
				++CollisionDetectionResults.SpaceCollisions.CollisionChecks;

				// Test the objects to see if there is a broadphase collision
				if (CheckBroadphaseCollision(object, candidate) == false)
				{
					// We do NOT have a collision, so simply move on to the next candidate object
					continue;
				}
				else
				{
					// We DO have a potential collision.  We do not currently store the broadphase collision parameters for future use, but 
					// if needed (From NCL physics PDF, collision detection part 1):
					//		m_penetration = r1r2 - sqrtf(distSq)
					//		m_normal = D3DXVec3Normalize(objpos - candpos)
					//		m_point = objpos - (m_normal * (obj.radius - m_penetration * 0.5f ));

					// Increment the count of broadphase collisions
					++CollisionDetectionResults.SpaceCollisions.BroadphaseCollisions;

					// 4. These two objects are potentially colliding.  We should now therefore pass them to the more computationally-expensive
					// narrowphase collision handling (if applicable) to determine if there is a true collision between components of each object
					result = CheckFullCollision(object, candidate, &collider0, &collider1);

					if (result)
					{
						// These two objects are colliding.  Determine the collision response and apply it
						HandleCollision(object, candidate, (collider0 ? &(collider0->Data) : NULL), (collider1 ? &(collider1->Data) : NULL));
						++CollisionDetectionResults.SpaceCollisions.Collisions;
					}
				}
			}
		}
	}
}

// Performs continuous collision detection (CCD) for the specified object, including potentially handling multiple collisions
// within the same execution cycle and rollback of physics time to simulate high-speed within-frame collisions.  Returns the 
// object which we collided with, if applicable, otherwise NULL
iSpaceObject * GamePhysicsEngine::PerformContinuousSpaceCollisionDetection(iSpaceObject *object)
{
	// Parameter check
	if (!object) return NULL;

	// Get all objects within a potential collision volume, based upon the current object velocity & with a buffer for ricochets
	// Quit immediately if there are no other objects in range
	std::vector<iSpaceObject*> candidates;
	int numcandidates = object->GetAllObjectsWithinDistance(GetCCDTestDistance(object), candidates, true, false, true);
	if (numcandidates == 0) return NULL;

	// The method will test for potentially multiple collisions within the same frame.  It therefore 'dials-back' the physics
	// clock to the correct within-frame time point in order to correctly handle them.  Clock state is restored at the end 
	// of the method.
	float restore_timefactor = PhysicsClock.TimeFactor;
	iSpaceObject *candidate, *collider, *exclude = NULL, *lastcollider = NULL;

	// Get basic information on the object that will be needed for each comparison
	bool hasexclusions = object->HasCollisionExclusions();

	// We will handle up to a maximum number of intra-frame collisions
	bool collision = false; float nearest = 1.01f;
	for (int i = 0; i < Game::C_MAX_INTRA_FRAME_CCD_COLLISIONS; ++i)
	{
		// Loop through each potential candidate in turn
		collider = NULL;
		for (int c = 0; c < numcandidates; ++c)
		{
			// Make sure the object is valid, and we aren't excluded from colliding with it
			candidate = candidates[c]; if (!candidate) continue;
			if (candidate == exclude || (hasexclusions && object->CollisionExcludedWithObject(candidate->GetID()))) continue;

			// We are testing against the candidate's OBB, so update it if it has been invalidated
			if (candidate->CollisionOBB.IsInvalidated()) candidate->CollisionOBB.UpdateFromObject(*candidate);

			// Test for collisions with this object; if nothing, we can move to the next candidate immediately
			++CollisionDetectionResults.SpaceCollisions.CCDCollisionChecks;
			collision = TestContinuousSphereVsOBBCollision(object, candidate);
			if (!collision) continue;

			// Otherwise, if this collision is closer than any previous one, record it as the collision to be handled
			if (m_collisiontest.ContinuousTestResult.IntersectionTime < nearest)
			{
				collider = candidate;
				nearest = m_collisiontest.ContinuousTestResult.IntersectionTime;
			}
		}

		// If we didn't collide with any of the objects then break out of the loop; there are no further collisions this frame
		if (collider == NULL) break;

		// Set the flag that enables collision handling for diverging objects; required for CCD collision handling.  Set here
		// the first time we actually need to handle a collision (in most cases we won't even get here)
		SetFlag_HandleDivergingCollisions();

		// Handle this collision between the two objects.  Physics clock has been adjusted to the correct intra-frame
		// time, so the collision response will be correct and proportionate
		HandleCollision(object, collider, NULL, &(collider->CollisionOBB.Data));
		++CollisionDetectionResults.SpaceCollisions.CCDCollisions;
		lastcollider = collider;

		// Adjust intra-frame time forwards to test for any further collision within the frame.  Prevent us from colliding
		// with the same object immediately again to prevent issues with penetration & multiple impacts
		exclude = collider;
		PhysicsClock.TimeFactor *= (1.0f - m_collisiontest.ContinuousTestResult.IntersectionTime);
		if (PhysicsClock.TimeFactor < Game::C_EPSILON) break;
	}

	// Restore the physics clock following these intra-frame test
	PhysicsClock.TimeFactor = restore_timefactor;

	// Revert the flag for testing diverging collisions back to false, now that CCD handling has been completed
	ClearFlag_HandleDivergingCollisions();

	// Return the last object that we collided with, or NULL if no collisions took place
	return lastcollider;
}

// Performs a full cycle of collision detection & collision response in a radius around the specified focal location (which is typically
// the player) in the specified environment.  Use the existing environment structure to partition & identify potential colliding pairs.  
// If radius < 0.0f then all objects in the environment will be considered (which can be inefficient).  This method is specific to 
// environment-based collision handling
void GamePhysicsEngine::PerformEnvironmentCollisionDetection(iSpaceObjectEnvironment *env, const D3DXVECTOR3 & location, float radius)
{
	// Parameter check
	if (!env) return;

	// Store the element size upper bound to ensure we can restrict to a valid area
	INTVECTOR3 envbound = env->GetElementSize(); --envbound.x; --envbound.y; --envbound.z;

	// Determine the range of elements which will need to be checked for collision around this focal location.  Swap Y and Z coords as we 
	// transition from space coordinates to environment coordinates
	INTVECTOR3 min_element = INTVECTOR3(max(0, min(envbound.x, (int)floorf((location.x - radius) * Game::C_CS_ELEMENT_SCALE_RECIP))),
										max(0, min(envbound.y, (int)floorf((location.z - radius) * Game::C_CS_ELEMENT_SCALE_RECIP))),
										max(0, min(envbound.z, (int)floorf((location.y - radius) * Game::C_CS_ELEMENT_SCALE_RECIP))));
	INTVECTOR3 max_element = INTVECTOR3(max(0, min(envbound.x, (int)floorf((location.x + radius) * Game::C_CS_ELEMENT_SCALE_RECIP))),
										max(0, min(envbound.y, (int)floorf((location.z + radius) * Game::C_CS_ELEMENT_SCALE_RECIP))),
										max(0, min(envbound.z, (int)floorf((location.y + radius) * Game::C_CS_ELEMENT_SCALE_RECIP))));

	// Peform environment collision handling on the objects in each element in turn.  All elements should be valid since we
	// clamped to the environment size
	for (int x = min_element.x; x <= max_element.x; ++x)
	{
		for (int y = min_element.y; y <= max_element.y; ++y)
		{
			for (int z = min_element.z; z <= max_element.z; ++z)
			{
				// Call the per-element collision handling method				
				PerformEnvironmentCollisionDetection(env, x, y, z);
			}
		}
	}
}

// Performs collision detection for all objects in the specified element
void GamePhysicsEngine::PerformEnvironmentCollisionDetection(iSpaceObjectEnvironment *env, int x, int y, int z)
{
	// Make sure the environment and the specified element are valid
	if (!env) return;
	ComplexShipElement *el = env->GetElement(x, y, z);
	if (!el) return;

	// Record the fact that we are checking this element around the focal object
	++CollisionDetectionResults.EnvironmentCollisions.ElementsChecked;

	// Loop through each active object in turn and test it for collisions
	iEnvironmentObject *obj;
	int n = el->Objects.size();
	for (int i = 0; i < n; ++i)
	{
		// Only check this object if the current element is its minimum element bound.  This avoids objects being
		// checked multiple times where they exist across multiple elements
		obj = el->Objects[i];
		const INTVECTOR3 & el_min = obj->GetElementRangeMin();
		if (!obj || el_min.x != x || el_min.y != y || el_min.z != z) continue;

		// Perform collision detection for this object with its surroundings
		PerformEnvironmentCollisionDetection(obj);
	}
}

// Performs collision detection for the specified object, in a particular element of the environment, with its surroundings
void GamePhysicsEngine::PerformEnvironmentCollisionDetection(iEnvironmentObject *obj)
{
	// Parameter checks
	if (!obj) return;
	iSpaceObjectEnvironment *env = obj->GetParentEnvironment();
	if (!env) return;

	// Record the fact that we are checking this object for collisions
	++CollisionDetectionResults.EnvironmentCollisions.ObjectsChecked;

	ComplexShipElement *el;
	iEnvironmentObject *candidate; StaticTerrain *terrain;
	std::vector<iEnvironmentObject*>::iterator a_it, a_it_end;
	std::vector<StaticTerrain*>::iterator t_it, t_it_end;
	OrientedBoundingBox *collider0, *collider1;

	// Store references to the key fields we will require
	const D3DXVECTOR3 & location = obj->GetEnvironmentPosition();
	const float & radius = obj->GetCollisionSphereRadius();

	// Store the object local vertical momentum before handling any collisions.  We can then test this later to determine if the 
	// object is on the 'ground'
	float pre_lmY = obj->PhysicsState.LocalMomentum.y;

	// Determine the range of elements which will need to be checked for collision around this object (typically just 1, or a couple of adjacent elements)
	// Swap Y and Z coords since we are moving from space to environment coordinates
	INTVECTOR3 min_element = INTVECTOR3((int)floorf((location.x - radius) * Game::C_CS_ELEMENT_SCALE_RECIP),
										(int)floorf((location.z - radius) * Game::C_CS_ELEMENT_SCALE_RECIP),
										(int)floorf((location.y - radius) * Game::C_CS_ELEMENT_SCALE_RECIP));
	INTVECTOR3 max_element = INTVECTOR3((int)floorf((location.x + radius) * Game::C_CS_ELEMENT_SCALE_RECIP),
										(int)floorf((location.z + radius) * Game::C_CS_ELEMENT_SCALE_RECIP),
										(int)floorf((location.y + radius) * Game::C_CS_ELEMENT_SCALE_RECIP));
	
	// Check each of these elements in turn
	for (int x = min_element.x; x <= max_element.x; ++x)
	{
		for (int y = min_element.y; y <= max_element.y; ++y)
		{
			for (int z = min_element.z; z <= max_element.z; ++z)
			{
				// Get a reference to this element and make sure it exists (e.g. we haven't left the edge of the environment)
				el = env->GetElement(x, y, z); 
				if (!el) continue;

				// Record the fact that we are checking for object collisions within this surrounding element
				++CollisionDetectionResults.EnvironmentCollisions.ElementsCheckedAroundObjects;

				// First check for collisions against terrain objects in the element
				t_it_end = el->TerrainObjects.end();
				for (t_it = el->TerrainObjects.begin(); t_it != t_it_end; ++t_it)
				{
					// Make sure this is a valid terrain object
					terrain = (*t_it); if (!terrain) continue;

					// Record the fact that we are testing this object/terrain collision pair
					++CollisionDetectionResults.EnvironmentCollisions.ObjectVsTerrainChecks;

					/* Broadphase collision check; determine whether object collision spheres intersect */
					if (CheckBroadphaseCollision(obj->GetEnvironmentPosition(), obj->GetCollisionSphereRadius(), 
												 terrain->GetPosition(), terrain->GetCollisionRadius()) == false)
					{
						continue;
					}
					else
					{
						// Broadphase collision detected, so perform full collision between these two objects.  Both are represented as OBBs
						// Replace with OBBHierarchy method if required later, but for now all colliding environment objects are represented by single OBBs
						++CollisionDetectionResults.EnvironmentCollisions.BroadphaseCollisions;

						// Update the object collision hierarchy before executing narrowphase collision detection, if it has been invalidated
						// since the last update.  Terrain OBB does not need to be updated; terrain objects store their environment-relative data
						// in their OBB, and so this is always up-to-date with the latest environment pos/orient.  The collision detection method
						// will handle transformation of object OBB data into that local coordinate frame, and performs a comparison in local
						// environment coordinates.  There is therefore no need to update the terrain OBB data here.
						if (obj->CollisionOBB.IsInvalidated()) obj->CollisionOBB.UpdateFromObject(*obj);

						// Now test the object collision against this world-space terrain data, and apply collision response if applicable
						TestAndHandleTerrainCollision(env, obj, terrain);
					}
				}

				// Now check for collisions against other active objects in the element
				a_it_end = el->Objects.end();
				for (a_it = el->Objects.begin(); a_it != a_it_end; ++a_it)
				{
					// Make sure this is a valid object
					candidate = (*a_it); if (!candidate) continue;

					// We only want to test a collision between two active objects once, i.e. we don't want to test (object vs candidate) and 
					// then (candidate vs object).  To do this efficiently we only test collisions where object.ID < candidate.ID.  The 
					// uniqueness and sequential nature of object IDs means this will always work
					if (obj->GetID() >= candidate->GetID()) continue;

					// Record the fact that we are testing this object/object collision pair
					++CollisionDetectionResults.EnvironmentCollisions.ObjectVsObjectChecks;

					/* Broadphase collision check; determine whether object collision spheres intersect */
					if (CheckBroadphaseCollision(obj, candidate) == false)
					{
						continue;
					}
					else
					{
						// We have a broadphase collision
						++CollisionDetectionResults.EnvironmentCollisions.BroadphaseCollisions;

						// Perform full collision between these two objects
						if (CheckFullCollision(obj, candidate, &collider0, &collider1))
						{
							++CollisionDetectionResults.EnvironmentCollisions.Collisions;
							HandleCollision(obj, candidate, (collider0 ? &(collider0->Data) : NULL), (collider1 ? &(collider1->Data) : NULL));
						}
					}
				}
			}
		}
	}

	// Test whether our downward momentum was cancelled out during collision detection; if so, we can be pretty confident
	// that we are on the 'ground', or at least on top of an object that is temporarily acting like ground
	const float & post_lmY = obj->PhysicsState.LocalMomentum.y;

	// On the ground if (1) our initial velocity was significantly negative (i.e. we were falling), (2) our new velocity
	// is more positive than it was before collision detection (i.e. some was cancelled out by a collision), (3) our 
	// new velocity is small (i.e. we have been stopped by the collision)
	obj->SetGroundFlag( (pre_lmY < 0.0001f && post_lmY > pre_lmY && fabs(post_lmY) < 0.001f) );
}

// Performs full SAT collision testing between the object and terrain OBBs.  If a collision is detected, applies an appropriate response
// to move the object out of the terrain collision box by the minimum separating axis
bool GamePhysicsEngine::TestAndHandleTerrainCollision(iSpaceObjectEnvironment *env, iEnvironmentObject *object, StaticTerrain *terrain)
{
	// No parameter checks; this will only ever be called by the physics engine at the point it has already confirmed all are valid objects

	// The actor OBB is in world space while the terrain OBB is in local environment space.  Convert the former to local environment
	// space for this comparison, since we will need to apply the response in local space in case of a collision
	D3DXVec3TransformCoord(&_obbdata.Centre, &object->CollisionOBB.Data.Centre, env->GetInverseZeroPointWorldMatrix());
	D3DXVec3TransformCoordArray(_obbdata.Axis, sizeof(D3DXVECTOR3), object->CollisionOBB.Data.Axis, sizeof(D3DXVECTOR3), env->GetInverseOrientationMatrix(), 3);
	_obbdata.Extent = object->CollisionOBB.Data.Extent;

	// Perform an SAT test between the object OBBs to see if there is a collision
	if (TestOBBvsOBBCollision(_obbdata, terrain->GetOBBData()))
	{
		// We have a collision
		++CollisionDetectionResults.EnvironmentCollisions.Collisions;
		
		// Use the last SAT result to determine the minimum separating axis that should be used to separate the objects
		// Also use the returned axis distance to determine whether the response should be +ve or -ve along the axis.  We test 
		// Object1 first since it is more frequently the reference frame returned by SAT
		D3DXVECTOR3 response = BASIS_VECTOR;
		if (m_collisiontest.SATResult.Object1Axis != -1)
		{
			response = (m_collisiontest.SATResult.AxisDist1[m_collisiontest.SATResult.Object1Axis] < 0.0f ? terrain->GetOBBData().Axis[m_collisiontest.SATResult.Object1Axis] :
																			  -(terrain->GetOBBData().Axis[m_collisiontest.SATResult.Object1Axis]));
		}
		else if (m_collisiontest.SATResult.Object0Axis != -1)
		{
			response = (m_collisiontest.SATResult.AxisDist0[m_collisiontest.SATResult.Object0Axis] < 0.0f ? _obbdata.Axis[m_collisiontest.SATResult.Object0Axis] :
																			  -(_obbdata.Axis[m_collisiontest.SATResult.Object0Axis]));
		}

		// Normalise the vector (although it should be normalised already since we are working with basis vectors)
		D3DXVec3Normalize(&response, &response);

		// We also want to update the object momentum; take the dot product of its current momentum along the response vector to
		// determine the equal and opposite force that should be applied
		float mom = D3DXVec3Dot(&object->PhysicsState.LocalMomentum, &response);
		float absmom = fabs(mom);
		if (absmom > Game::C_EPSILON)
		{
#			ifdef RJ_LOG_PLAYER_TERRAIN_COLLISION_DETAILS
				if (Game::CurrentPlayer && Game::CurrentPlayer->GetActor() && object->GetID() == Game::CurrentPlayer->GetActor()->GetID())
					OutputDebugString(concat("Applying momentum of ")(-mom)(" along response vector (")(response.x)(",")(response.y)(",")(response.z)
						("); world momentum change from (")(object->PhysicsState.WorldMomentum.x)(",")(object->PhysicsState.WorldMomentum.y)(",")
						(object->PhysicsState.WorldMomentum.z)(")").str().c_str());
#			endif

			// Apply a direct force (i.e. no division through by mass) in the object local frame, to nullify current momentum along the response vector
			object->ApplyLocalLinearForceDirect(response * -mom);

			// Also test whether this momentum change exceeds our 'impact threshold'.  If it does, we consider this a significant impact (vs
			// e.g. a normal floor collision) and may apply an additional response such as object or terrain damage
			if (absmom > Game::C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD)
			{
				TerrainImpact.Terrain = terrain;
				TerrainImpact.ResponseVector = response;
				TerrainImpact.ResponseVelocity = -mom;
				TerrainImpact.ImpactVelocity = absmom;
				TerrainImpact.ImpactForce = (absmom * object->GetMass());
				object->CollisionWithTerrain(TerrainImpact);
			}
			
#			ifdef RJ_LOG_PLAYER_TERRAIN_COLLISION_DETAILS
				if (Game::CurrentPlayer && Game::CurrentPlayer->GetActor() && object->GetID() == Game::CurrentPlayer->GetActor()->GetID())
					OutputDebugString(concat(" to (")(object->PhysicsState.WorldMomentum.x)(",")(object->PhysicsState.WorldMomentum.y)(",")
						(object->PhysicsState.WorldMomentum.z)(")\n").str().c_str());
#			endif
		}

		// We now want to scale the response vector by the penetration distance (since we want to move the object by the exact inverse 
		// of that distance, out of the collision) to determine the collision response
		response *= m_collisiontest.Penetration;

#		ifdef RJ_LOG_PLAYER_TERRAIN_COLLISION_DETAILS
			if (Game::CurrentPlayer && Game::CurrentPlayer->GetActor() && object->GetID() == Game::CurrentPlayer->GetActor()->GetID())
				OutputDebugString(concat("Adjusting position by penentration of ")(m_collisiontest.Penetration)(" along response vector (")(response.x)(",")(response.y)(",")(response.z)("); position change from ")
					("(")(object->GetEnvironmentPosition().x)(",")(object->GetEnvironmentPosition().y)(",")(object->GetEnvironmentPosition().z)(")").str().c_str());
#		endif

		// Apply the change in location to the object
		object->SetEnvironmentPosition(object->GetEnvironmentPosition() + response);

#		ifdef RJ_LOG_PLAYER_TERRAIN_COLLISION_DETAILS
			if (Game::CurrentPlayer && Game::CurrentPlayer->GetActor() && object->GetID() == Game::CurrentPlayer->GetActor()->GetID())
				OutputDebugString(concat(" to (")(object->GetEnvironmentPosition().x)(",")(object->GetEnvironmentPosition().y)(",")(object->GetEnvironmentPosition().z)(")\n").str().c_str());
#		endif

		// Force an immediate refresh so that the collision volume remains in sync for the remainder of this frame
		// Efficiency measure: the terrain collision is only adjusting the object position (not its axes).  If the 
		// object has a compound OBB then we will need to recalculate the world matrix & traverse it recursively.  
		// However if it is a single OBB we can directly set the position and wait until the next frame to recalc everything
		if (!object->CollisionOBB.HasChildren())
			object->CollisionOBB.Data.Centre = object->GetPosition();
		else
			object->RefreshPositionImmediate();

		// Finally, return true to indicate that a collision was detected
		return true;
	}
	else
	{
		// No collision was detected
		return false;
	}
}

// Checks for a broadphase collision between the two objects.  No parameter checking since this should only be called internally on pre-validated parameters
// Broadphase collision is tested by a bounding sphere test; objects may be colliding if (d^2 < (r1 + r2)^2), where
//		d = distance between centre of the two bounding spheres
//		r1, r2 = the radius of each object's bounding spheres
CMPINLINE bool GamePhysicsEngine::CheckBroadphaseCollision(const iObject *obj0, const iObject *obj1)
{
	// Perform a simple bounding spehere test on the objects
	_diffpos = (obj0->GetPosition() - obj1->GetPosition());
	_distsq = (_diffpos.x * _diffpos.x) + (_diffpos.y * _diffpos.y) + (_diffpos.z * _diffpos.z);
	_r1r2 = (obj0->GetCollisionSphereRadius() + obj1->GetCollisionSphereRadius());

	// Test the values to see if there is a broadphase collision
	m_collisiontest.BroadphasePenetrationSq = ((_r1r2 * _r1r2) - _distsq);
	return (m_collisiontest.BroadphasePenetrationSq > 0.0f);
}

// Checks for a broadphase collision between the two objects.  No parameter checking since this should only be called internally on pre-validated parameters
// Broadphase collision is tested by a bounding sphere test; objects may be colliding if (d^2 < (r1 + r2)^2), where
//		d = distance between centre of the two bounding spheres
//		r1, r2 = the radius of each object's bounding spheres
CMPINLINE bool GamePhysicsEngine::CheckBroadphaseCollision(const D3DXVECTOR3 & pos0, float collisionradius0, const D3DXVECTOR3 & pos1, float collisionradius1)
{
	// Perform a simple bounding sphere test on the objects
	_diffpos = (pos0 - pos1);
	_distsq = (_diffpos.x * _diffpos.x) + (_diffpos.y * _diffpos.y) + (_diffpos.z * _diffpos.z);
	_r1r2 = (collisionradius0 + collisionradius1);

	// Test the values to see if there is a broadphase collision
	m_collisiontest.BroadphasePenetrationSq = ((_r1r2 * _r1r2) - _distsq);
	return (m_collisiontest.BroadphasePenetrationSq > 0.0f);
}

bool GamePhysicsEngine::CheckFullCollision(iObject *obj0, iObject *obj1, OrientedBoundingBox **ppOutCollider0, OrientedBoundingBox **ppOutCollider1)
{
	if (obj0->GetCollisionMode() == Game::CollisionMode::BroadphaseCollisionOnly)
	{
		if (obj1->GetCollisionMode() == Game::CollisionMode::BroadphaseCollisionOnly)
		{
			// obj0 = broadphase only, obj1 = broadphase only.  Return true automatically since we only call this method if broadphase collision was detected
			// By setting the collision detection type to "SphereVsSphere" we tell downstream methods that they should use the broadphase penetration distance sq, 
			// and that the "Penetration" value will not be set here for efficiency (to avoid the sqrt)
			m_collisiontest.Type = CollisionDetectionType::SphereVsSphere;
			(*ppOutCollider0) = (*ppOutCollider1) = NULL;
			return true;
		}
		else
		{
			// obj0 = broadphase only, obj1 = full collision
			(*ppOutCollider0) = NULL; 

			// We are now interested in the detailed hierarchy for object1; update the OBB hierarchy for the object if 
			// it has been invalidated since it was last used			
			if (obj1->CollisionOBB.IsInvalidated())
			{
				obj1->CollisionOBB.UpdateFromObject(*obj1);
			}

			// Perform the collision test
			return TestSpherevsOBBHierarchyCollision(obj0->GetPosition(), obj0->GetCollisionSphereRadiusSq(), obj1->CollisionOBB, ppOutCollider1);
		}
	}
	else
	{
		if (obj1->GetCollisionMode() == Game::CollisionMode::BroadphaseCollisionOnly)
		{
			// obj0 = full collision, obj1 = broadphase only
			(*ppOutCollider1) = NULL; 

			// We are now interested in the detailed hierarchy for object1; update the OBB hierarchy for the object if 
			// it has been invalidated since it was last used			
			if (obj0->CollisionOBB.IsInvalidated())
			{
				obj0->CollisionOBB.UpdateFromObject(*obj0);
			}

			// Perform the collision test
			return TestSpherevsOBBHierarchyCollision(obj1->GetPosition(), obj1->GetCollisionSphereRadiusSq(), obj0->CollisionOBB, ppOutCollider0);
		}
		else
		{
			// obj0 = full collision, obj1 = full collision

			// We are now interested in the detailed hierarchy for both object0 and object1, so recalculate the data for all nodes 
			// in their collision hierarchies before testing collision
			if (obj0->CollisionOBB.IsInvalidated()) obj0->CollisionOBB.UpdateFromObject(*obj0);
			if (obj1->CollisionOBB.IsInvalidated()) obj1->CollisionOBB.UpdateFromObject(*obj1);

			// Perform the collision test
			return TestOBBvsOBBHierarchy(obj0->CollisionOBB, obj1->CollisionOBB, ppOutCollider0, ppOutCollider1);
		}
	}
}

// Determines and applies collision response based upon a collision between object0.Collider0 and object1.Collider1.  
// Called from main PerformCollisionDetection() method.
#ifdef RJ_NEW_COLLISION_HANDLING
void GamePhysicsEngine::HandleCollision(iActiveObject *object0, iActiveObject *object1,
										const OrientedBoundingBox::CoreOBBData *collider0, const OrientedBoundingBox::CoreOBBData *collider1)
{
	// No parameter checks here; we rely on the integrity of main collision detection method (which should be the only method
	// to invoke this one) to ensure that object[0|1] are non-null valid objects.  For efficiency.  
	// collider[0|1] can be null if there is no relevant colliding OBB (e.g. if the object is broadphase collision-only)

	// Special case; if either object is a ship section & part of a larger complex ship, move up the hierarchy one level
	// and treat the ship itself as being the colliding object.  Ship statistics (e.g. mass) are derived from the combination of all
	// its sections, so this is the correct object to be involving in the collision
	if (object0->GetObjectType() == iObject::ObjectType::ComplexShipSectionObject) 
	{
		ComplexShipSection *sec = (ComplexShipSection*)object0;
		if (sec->GetParent()) object0 = (iActiveObject*)sec->GetParent();
	}
	if (object1->GetObjectType() == iObject::ObjectType::ComplexShipSectionObject)
	{
		ComplexShipSection *sec = (ComplexShipSection*)object1;
		if (sec->GetParent()) object1 = (iActiveObject*)sec->GetParent();
	}

	// Store the momentum of each object before applying a response, to allow calculation of the impact force
	D3DXVECTOR3 obj0_pre_wm = object0->PhysicsState.WorldMomentum;
	D3DXVECTOR3 obj1_pre_wm = object1->PhysicsState.WorldMomentum;

	/* Apply collision response using the impulse method - first, apply the normal component */

	// Get a reference to the object centre points, and the normal between them
	D3DXVECTOR3 normal;
	const D3DXVECTOR3 & c0 = (collider0 ? collider0->Centre : object0->GetPosition());
	const D3DXVECTOR3 & c1 = (collider1 ? collider1->Centre : object1->GetPosition());
	normal = (c0 - c1);
	D3DXVec3Normalize(&normal, &normal);

	// Determine hit point on the surface of each object 
	D3DXVECTOR3 hit0, hit1;
	if (collider0)			  hit0 = ClosestPointOnOBB(*collider0, c1);
	else					  hit0 = (c0 - (normal * object0->GetCollisionSphereRadius()));
	if (collider1)			  hit1 = ClosestPointOnOBB(*collider1, c0);
	else					  hit1 = (c1 + (normal * object1->GetCollisionSphereRadius()));

	// Get vectors from object centres to their hitpoints
	D3DXVECTOR3 r0 = (hit0 - c0);
	D3DXVECTOR3 r1 = (hit1 - c1);

	// Store mass & inverse mass reference parameters for convenience
	const float & mass0 = object0->GetMass(); 
	const float & mass1 = object1->GetMass();
	const float & invMass0 = object0->GetInverseMass();
	const float & invMass1 = object1->GetInverseMass();

	// Cross the angular velocity of each object with its hitpoint contact vector
	D3DXVECTOR3 angR0, angR1;
	D3DXVec3Cross(&angR0, &object0->PhysicsState.AngularVelocity, &r0);
	D3DXVec3Cross(&angR1, &object1->PhysicsState.AngularVelocity, &r1);

	// Determine the component of the relative object velocity that is along the normal vector
	D3DXVECTOR3 v0 = (object0->PhysicsState.WorldMomentum /* * mass0 */) + angR0;
	D3DXVECTOR3 v1 = (object1->PhysicsState.WorldMomentum /* * mass1 */) + angR1;
	D3DXVECTOR3 vrel = (v0 - v1);
	float vn = D3DXVec3Dot(&vrel, &normal);

	// If the objects are moving away from each other then there is no collision response required, UNLESS
	// the flag to ignore this test has been set
	if (!m_flag_handle_diverging_collisions && -vn < 0.01f) return;

	// Transform the inertia tensor for each object into world space
	D3DXMATRIX worldInvI0, worldInvI1;
	D3DXMatrixMultiply(&worldInvI0, object0->GetOrientationMatrix(), &(object0->PhysicsState.InverseInertiaTensor));	
	D3DXMatrixMultiply(&worldInvI1, object1->GetOrientationMatrix(), &(object1->PhysicsState.InverseInertiaTensor));	

	// Derive the impulse 'jn' that should be applied along the contact normal, incorporating both linear and angular momentum
	D3DXVECTOR3 Crn0, Crn1, ICrn0, ICrn1, CICrn0, CICrn1;
	D3DXVec3Cross(&Crn0, &r0, &normal);
	D3DXVec3Cross(&Crn1, &r1, &normal);
	D3DXVec3TransformCoord(&ICrn0, &Crn0, &worldInvI0);
	D3DXVec3TransformCoord(&ICrn1, &Crn1, &worldInvI1);
	D3DXVec3Cross(&CICrn0, &ICrn0, &r0);
	D3DXVec3Cross(&CICrn1, &ICrn1, &r1);

	float jn = ((-Game::C_COLLISION_SPACE_COEFF_ELASTICITY * vn) - vn)
			 /
			   (invMass0 + invMass1 + D3DXVec3Dot(&normal, &CICrn0) + D3DXVec3Dot(&normal, &CICrn1));

	// Adjustment; scale normal impulse by the degree of penetration to avoid 'sinking' of low-velocity objects into one another
	//jn += (m_collisiontest.DeterminePenetration() * 1.5f);

	// The impulse will be applied along the normal vector between the two object hitpoints
	D3DXVECTOR3 impulse = (normal * jn);

	// Calculate change in angular velocity
	D3DXVECTOR3 angInc0, angInc1;
	D3DXVec3Cross(&angInc0, &r0, &(impulse));
	D3DXVec3Cross(&angInc1, &r1, &(-impulse));
	D3DXVec3TransformCoord(&angInc0, &angInc0, &worldInvI0);
	D3DXVec3TransformCoord(&angInc1, &angInc1, &worldInvI1);

	// Apply change in linear and angular velocity to each object
	object0->PhysicsState.WorldMomentum += (impulse * invMass0);
	object0->PhysicsState.AngularVelocity += angInc0;
	object1->PhysicsState.WorldMomentum += (-impulse * invMass1);
	object1->PhysicsState.AngularVelocity += angInc1;
	

	// Log details of the collision to the debug output, if the relevant compiler flag is set
#	ifdef RJ_LOG_COLLISION_DETAILS 
		OutputDebugString("\n\n*** Collision = { \n");
		OutputDebugString(concat("\t Object0 = \"")(object0->GetName())("\" [")((int)object0->GetCollisionMode())("], Object1 = \"")
			(object1->GetName())("\" [")((int)object1->GetCollisionMode())("] \n").str().c_str());
		OutputDebugString(concat("\t collision normal = [")(normal.x)(",")(normal.y)(",")(normal.z)("]\n").str().c_str());
		OutputDebugString(concat("\t v0 = (object0 momentum [")(object0->PhysicsState.WorldMomentum.x)(",")(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)
			("] * mass [")(mass0)("]) + angvel at contact vector [")(angR0.x)(",")(angR0.y)(",")(angR0.z)("] = [")(v0.x)(",")(v0.y)(",")(v0.z)("]\n").str().c_str());
		OutputDebugString(concat("\t v1 = (object1 momentum [")(object1->PhysicsState.WorldMomentum.x)(",")(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)
			("] * mass [")(mass1)("]) + angvel at contact vector [")(angR1.x)(",")(angR1.y)(",")(angR1.z)("] = [")(v1.x)(",")(v1.y)(",")(v1.z)("]\n").str().c_str());
		OutputDebugString(concat("\t vrel = (v0 [")(v0.x)(",")(v0.y)(",")(v0.z)("] - v1 [")(v1.x)(",")(v1.y)(",")(v1.z)("]) = [")(vrel.x)(",")(vrel.y)(",")(vrel.z)("]\n").str().c_str());
		OutputDebugString(concat("\t vn = dot(vrel, normal) = ")(vn)(" (+ve value means objects are diverging and there is no collision)\n").str().c_str());
		OutputDebugString(concat("\t Object0 pre-collision state: world momentum = [")(object0->PhysicsState.WorldMomentum.x)(",")
			(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object0->PhysicsState.AngularVelocity.x)
			(",")(object0->PhysicsState.AngularVelocity.y)(",")(object0->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
		OutputDebugString(concat("\t Object1 pre-collision state: world momentum = [")(object1->PhysicsState.WorldMomentum.x)(",")
			(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object1->PhysicsState.AngularVelocity.x)
			(",")(object1->PhysicsState.AngularVelocity.y)(",")(object1->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
		OutputDebugString(concat("\t jn = symmetric impulse along collision normal = ")(jn)("\n").str().c_str());
		OutputDebugString(concat("\t AngInc0 = change in normal angular velocity for object0 = [")(angInc0.x)(",")(angInc0.y)(",")(angInc0.z)("]\n").str().c_str());
		OutputDebugString(concat("\t AngInc1 = change in normal angular velocity for object1 = [")(angInc1.x)(",")(angInc1.y)(",")(angInc1.z)("]\n").str().c_str());
		OutputDebugString(concat("\t Object0 post-normal collision state: world momentum = [")(object0->PhysicsState.WorldMomentum.x)(",")
			(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object0->PhysicsState.AngularVelocity.x)
			(",")(object0->PhysicsState.AngularVelocity.y)(",")(object0->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
		OutputDebugString(concat("\t Object1 post-normal collision state: world momentum = [")(object1->PhysicsState.WorldMomentum.x)(",")
			(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object1->PhysicsState.AngularVelocity.x)
			(",")(object1->PhysicsState.AngularVelocity.y)(",")(object1->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
#	endif



	/* Now apply tangent component to simulate friction at the contact point */
	
	// Recalculate velocity data, since it has been changed by the normal impulse above.  TODO: REQUIRED?
	D3DXVec3Cross(&angR0, &object0->PhysicsState.AngularVelocity, &r0);
	D3DXVec3Cross(&angR1, &object1->PhysicsState.AngularVelocity, &r1);
	v0 = object0->PhysicsState.WorldMomentum + angR0;
	v1 = object1->PhysicsState.WorldMomentum + angR1;
	vrel = (v0 - v1);
	vn = D3DXVec3Dot(&vrel, &normal);

	// Determine tangent vector, perpendicular to the collision normal
	D3DXVECTOR3 tangent = vrel - (D3DXVec3Dot(&vrel, &normal) * normal);
	float tangent_mag = D3DXVec3Length(&tangent);

	// Only proceed with calcualting the tangential velocity if the tangent is valid
	if (tangent_mag > Game::C_EPSILON)
	{
		// Safety feature: if objects somehow become merged together (e.g. spawned inside each other) the derived tangent can exceed -1e38 and leave
		// us with an undefined float error.  In such cases, we will apply a default tangent to prevent overflow
		// Use the fact that (x == x) will trivially return true for all finite numbers, but where x = -1.#IND000, (-1.#IND000 != -1.#IND000)
		// TODO: this isn't perfect, and prevents some collisions registering at shallow impact angles.  Issue with calc?  Or need double precision here?
		if (!(tangent.x == tangent.x)) tangent = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		D3DXVECTOR3 T = (-tangent / tangent_mag);

		// Calculate intermediate cross products 
		D3DXVECTOR3 Cr0tan, Cr1tan, transformedCr0tan, transformedCr1tan, Ctransr0, Ctransr1;
		D3DXVec3Cross(&Cr0tan, &r0, &T);
		D3DXVec3Cross(&Cr1tan, &r1, &T);
		D3DXVec3TransformCoord(&transformedCr0tan, &Cr0tan, &worldInvI0);
		D3DXVec3TransformCoord(&transformedCr1tan, &Cr1tan, &worldInvI1);
		D3DXVec3Cross(&Ctransr0, &transformedCr0tan, &r0);
		D3DXVec3Cross(&Ctransr1, &transformedCr1tan, &r1);

		// Now determine the tangential impulse 'jt'
		float denom = (invMass0 + invMass1 + D3DXVec3Dot(&T, &Ctransr0) + D3DXVec3Dot(&T, &Ctransr1));
		if (denom > Game::C_EPSILON)
		{
			float jt, jt_initial = (tangent_mag / denom);

			// Test whether the friction imparted by the normal impulse vector is more appropriate 
			// than this calculated tangent impulse.  If so, replace it with the normal impulse derived above
			const float STATIC_FRICTION = 0.7f;
			const float DYNAMIC_FRICTION = 0.5f;
			
			// Perform the comparison and pull in normal-derived impulse if more appropriate
			float normal_derived_impulse = (jn * STATIC_FRICTION);
			if (jt_initial < normal_derived_impulse)
				jt = jt_initial;
			else
				jt = (jn * DYNAMIC_FRICTION);

			// Calculate change in angular velocity
			D3DXVECTOR3 tangent_impulse = (T * jt);
			D3DXVec3Cross(&angInc0, &r0, &(tangent_impulse));
			D3DXVec3Cross(&angInc1, &r1, &(-tangent_impulse));
			D3DXVec3TransformCoord(&angInc0, &angInc0, &worldInvI0);
			D3DXVec3TransformCoord(&angInc1, &angInc1, &worldInvI1);

			// Apply the change in linear and angular impulse to each object
			object0->PhysicsState.WorldMomentum += (tangent_impulse * invMass0);
			object0->PhysicsState.AngularVelocity += angInc0;
			object1->PhysicsState.WorldMomentum += (-tangent_impulse * invMass1);
			object1->PhysicsState.AngularVelocity += angInc1;

#		ifdef RJ_LOG_COLLISION_DETAILS
			OutputDebugString(concat("\n\t Normalised tangent vector = [")(tangent.x)(",")(tangent.y)(",")(tangent.z)("]\n").str().c_str());
			OutputDebugString(concat("\t jt = symmetric impulse along collision tangent = ")(jt)("\n").str().c_str());
			OutputDebugString(concat("\t AngInc0 = change in tangential angular velocity for object0 = [")(angInc0.x)(",")(angInc0.y)(",")(angInc0.z)("]\n").str().c_str());
			OutputDebugString(concat("\t AngInc1 = change in tangential angular velocity for object1 = [")(angInc1.x)(",")(angInc1.y)(",")(angInc1.z)("]\n").str().c_str());
			OutputDebugString(concat("\t Object0 post-tangential collision state: world momentum = [")(object0->PhysicsState.WorldMomentum.x)(",")
				(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object0->PhysicsState.AngularVelocity.x)
				(",")(object0->PhysicsState.AngularVelocity.y)(",")(object0->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
			OutputDebugString(concat("\t Object1 post-tangential collision state: world momentum = [")(object1->PhysicsState.WorldMomentum.x)(",")
				(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object1->PhysicsState.AngularVelocity.x)
				(",")(object1->PhysicsState.AngularVelocity.y)(",")(object1->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
			if (object0->PhysicsState.WorldMomentum.x > Game::C_MAX_LINEAR_VELOCITY || object0->PhysicsState.WorldMomentum.y > Game::C_MAX_LINEAR_VELOCITY ||
				object0->PhysicsState.WorldMomentum.z > Game::C_MAX_LINEAR_VELOCITY || object0->PhysicsState.AngularVelocity.x > Game::C_MAX_ANGULAR_VELOCITY ||
				object0->PhysicsState.AngularVelocity.y > Game::C_MAX_ANGULAR_VELOCITY || object0->PhysicsState.AngularVelocity.z > Game::C_MAX_ANGULAR_VELOCITY)
				OutputDebugString("\t Object0 exceeds physical limits on linear and/or angular velocity; state will be scaled within limits\n");
			if (object1->PhysicsState.WorldMomentum.x > Game::C_MAX_LINEAR_VELOCITY || object1->PhysicsState.WorldMomentum.y > Game::C_MAX_LINEAR_VELOCITY ||
				object1->PhysicsState.WorldMomentum.z > Game::C_MAX_LINEAR_VELOCITY || object1->PhysicsState.AngularVelocity.x > Game::C_MAX_ANGULAR_VELOCITY ||
				object1->PhysicsState.AngularVelocity.y > Game::C_MAX_ANGULAR_VELOCITY || object1->PhysicsState.AngularVelocity.z > Game::C_MAX_ANGULAR_VELOCITY)
				OutputDebugString("\t Object1 exceeds physical limits on linear and/or angular velocity; state will be scaled within limits\n");
			OutputDebugString("}\n\n");
#		endif
		}
	}

	// Scale all resulting object forces to be within allowable linear/angular velocity ranges
	/*ScaleVectorWithinMagnitudeLimit(object0->PhysicsState.WorldMomentum, Game::C_MAX_LINEAR_VELOCITY);
	ScaleVectorWithinMagnitudeLimit(object1->PhysicsState.WorldMomentum, Game::C_MAX_LINEAR_VELOCITY);
	ScaleVectorWithinMagnitudeLimit(object0->PhysicsState.AngularVelocity, Game::C_MAX_ANGULAR_VELOCITY);
	ScaleVectorWithinMagnitudeLimit(object1->PhysicsState.AngularVelocity, Game::C_MAX_ANGULAR_VELOCITY);
	*/
	// We have made multiple changes to the world momentum of both objects; recalculate the resulting local momentum for each now
	object0->RecalculateLocalMomentum();
	object1->RecalculateLocalMomentum();

	// Determine the impact force on each object by comparing pre- & post-collision momentum
	ObjectImpact.Object.PreImpactVelocity = obj0_pre_wm;
	ObjectImpact.Collider.PreImpactVelocity = obj1_pre_wm;
	ObjectImpact.Object.VelocityChange = (object0->PhysicsState.WorldMomentum - obj0_pre_wm);
	ObjectImpact.Collider.VelocityChange = (object1->PhysicsState.WorldMomentum - obj1_pre_wm);
	ObjectImpact.Object.VelocityChangeMagnitude = D3DXVec3Length(&ObjectImpact.Object.VelocityChange);
	ObjectImpact.Collider.VelocityChangeMagnitude = D3DXVec3Length(&ObjectImpact.Collider.VelocityChange);
	ObjectImpact.Object.ImpactForce = (ObjectImpact.Object.VelocityChangeMagnitude * object0->GetMass());
	ObjectImpact.Collider.ImpactForce = (ObjectImpact.Collider.VelocityChangeMagnitude * object1->GetMass());
	ObjectImpact.TotalImpactForce = (ObjectImpact.Object.ImpactForce + ObjectImpact.Collider.ImpactForce);

	// Notify object 0 of the collision
	object0->CollisionWithObject(object1, ObjectImpact);

	// Swap the definition of object & collider and then notify object1 of the impact
	std::swap(ObjectImpact.Object, ObjectImpact.Collider);
	object1->CollisionWithObject(object1, ObjectImpact);	
}
#endif

// Determines and applies collision response based upon a collision between object0.Collider0 and object1.Collider1.  Both 
// are environment objects. Called from main PerformCollisionDetection() method.  NOTE: this could also handle iActiveObject 
// if we want to generalise one level
/*void GamePhysicsEngine::HandleEnvironmentCollision(iEnvironmentObject *object0, iEnvironmentObject *object1,
	const OrientedBoundingBox::CoreOBBData *collider0, const OrientedBoundingBox::CoreOBBData *collider1)
{
	// No parameter checks here; we rely on the integrity of main collision detection method (which should be the only method
	// to invoke this one) to ensure that object[0|1] are non-null valid objects.  For efficiency.  
	// collider[0|1] can be null if there is no relevant colliding OBB (e.g. if the object is broadphase collision-only



}

// Determines collision response between an active object and the terrain.  Implements some simplifying assumptions (for now) such as the
// fact that the active object will always be small enough to approximate the contact point as its origin.  The collision response is always
// an equal and opposite response to the active object's momentum along the contact normal, resulting in the active object stopping
// immediately, and the terrain object remaining immobile.
void GamePhysicsEngine::HandleTerrainCollision(iEnvironmentObject *object, StaticTerrain *terrain)
{
	// No parameter checks; this will only ever be called by the physics engine at the point it has already confirmed both are valid objects
	return;
	// Get the contact point on the terrain OBB.  Simplifying assumption: contact point on the active object is its origin (for now)
	D3DXVECTOR3 dist;
	D3DXVECTOR3 contact = ClosestPointOnOBB(terrain->GetOBBData(), object->GetEnvironmentPosition(), dist);

	// Get the collision normal by taking the vector from this contact point to the object origin.  Normalise, and transform into 
	// world space since this is where all physics calculations are performed
	D3DXVECTOR3 worldnormal, normal = (contact - object->GetEnvironmentPosition());
	D3DXVec3Normalize(&normal, &normal);
	D3DXVec3TransformCoord(&worldnormal, &normal, object->GetParentEnvironment()->GetWorldMatrix());

	// Determine the proportion of this object's world momentum that is along the (world space) contact normal
	float mom = D3DXVec3Dot(&object->PhysicsState.WorldMomentum, &worldnormal);

	// If the momentum value is positive along this vector then applies an equal and opposite force to halt the object's movement
	// TODO: we can also pass this data back to the object to enable calculation of collision damage where appropriate (> some threshold)
	if (mom > Game::C_EPSILON) object->ApplyWorldSpaceForce(contact, (worldnormal * -mom));

	// Finally, test whether the object has penetrated the terrain OBB in any dimension.  If it has, move it outside of the OBB bounds
	// Subtract the OBB extent to give the distance from the EDGE of the OBB.  If any component is now -ve we are inside the OBB
	/*bool intersecting = false;
	dist -= terrain->GetExtent();							
	if (dist.x > Game::C_EPSILON) dist.x = 0.0f; else { dist.x = -dist.x; intersecting = true; }
	if (dist.y > Game::C_EPSILON) dist.y = 0.0f; else { dist.y = -dist.y; intersecting = true; }
	if (dist.z > Game::C_EPSILON) dist.z = 0.0f; else { dist.z = -dist.z; intersecting = true; }

	// If the object is intersecting then move outside of the OBB bounds now 
	if (intersecting)
	{
		D3DXMATRIX invworld;
		D3DXMatrixInverse(&invworld, NULL, terrain->GetWorldMatrix());
		D3DXVec3TransformCoord(&dist, &dist, &invworld);
		object->SetPosition(object->GetPosition() + dist);
	}

	// Finally, test whether the object has penetrated the terrain OBB in any dimension.  If it has, move it outside of the OBB bounds
	// by adjusting the position gradually away from the terrain object along the contact normal (which will point OUTWARDS from the OBB, 
	// since the object is inside the OBB due to the intersection)
	dist -= terrain->GetExtent();
	if (dist.y < Game::C_EPSILON_NEG || dist.x < Game::C_EPSILON_NEG || dist.z < Game::C_EPSILON_NEG)
	{
		// Get and normalise the vector between object origin points
		D3DXVECTOR3 origin_normal = (terrain->GetPosition() - (object->GetEnvironmentPosition() + D3DXVECTOR3(0.0f, object->GetSize().y*0.5f, 0.0f)));
		D3DXVec3Normalize(&origin_normal, &origin_normal);

		// Move the object back along this normal gradually, to push it out of the terrain object and remove the intersection
		object->SetEnvironmentPosition(object->GetEnvironmentPosition() + (origin_normal * D3DXVec3Dot(&dist, &origin_normal)));
	}
}
*/

// Default destructor
GamePhysicsEngine::~GamePhysicsEngine(void)
{
}

// Performs hierarchical collision detection between two OBB hierarchies
bool GamePhysicsEngine::TestOBBvsOBBHierarchy(OrientedBoundingBox & obj0, OrientedBoundingBox & obj1, 
											  OrientedBoundingBox ** ppOutCollider0, OrientedBoundingBox ** ppOutCollider1)
{
	// Test the two objects for overlap; if they are not colliding then early-exit immediately
	if (TestOBBvsOBBCollision(obj0.Data, obj1.Data) == false) return false;

	// Otherwise, we have a collision
	if (!obj0.HasChildren())
	{
		if (!obj1.HasChildren())
		{
			// If both obj0 & obj1 are leaf OBBs then report the collision immediately
			(*ppOutCollider0) = &obj0; (*ppOutCollider1) = &obj1;
			return true;
		}
		else
		{
			// Obj0 is a leaf, Obj1 is a branch.  Iterate through each of obj1-children now and recurse down the hierarchy
			for (int i = 0; i < obj1.ChildCount; ++i)
			{
				// Roll up a positive result if we receive one
				if (TestOBBvsOBBHierarchy(obj0, obj1.Children[i], ppOutCollider0, ppOutCollider1) == true) return true;
			}
		}
	}
	else
	{
		// Obj0 is a branch, obj1 is [branch|leaf].  Iterate through each child of obj0 and call the method recursively
		for (int i = 0; i < obj0.ChildCount; ++i)
		{
			 // Roll up a positive result if we receive one
			if (TestOBBvsOBBHierarchy(obj0.Children[0], obj1, ppOutCollider0, ppOutCollider1) == true) return true;
		}
	}

	// There were no successful collisions after traversing both hierarchies, so the objects are not colliding
	return false;
}



// Tests for the intersection of two oriented bounding boxes (OBB)
bool GamePhysicsEngine::TestOBBvsOBBCollision(const OrientedBoundingBox::CoreOBBData & box0, const OrientedBoundingBox::CoreOBBData & box1)
{
	// (Adapted from http://www.geometrictools.com/GTEngine/Include/GteIntrOrientedBox3OrientedBox3.inl)

	static const float cutoff = 1.0f - Game::C_EPSILON;
	bool parallelPairExists = false;

	// Determine the distance between box centre points
	D3DXVECTOR3 dist = (box1.Centre - box0.Centre);

	// Declare storage for key intermediate calculations
    float dot01[3][3];       // dot01[i][j] = Dot(A0[i],A1[j]) = A1[j][i]
    float absDot01[3][3];    // |dot01[i][j]|
    /*float dotDA0[3];         // Dot(D, A0[i]) ---- REMOVED, now store as part of SAT result for efficiency */
    float r0, r1, r;         // interval radii and distance between centers
    float r01;               // r0 + r1
	float r01_r;			 // (r01 - r), the penetration (+ve) or separation (-ve) distance

	// Reset the SAT penetration depth, since we are looking for the axis with minimum penetration in this test
	m_collisiontest.Penetration = FLT_MAX;

	// Test for separation on the axis box0.Centre + t*box0.Axis[0].
    for (int i = 0; i < 3; ++i)
    {
        dot01[0][i] = D3DXVec3Dot(&box0.Axis[0], &box1.Axis[i]);
        absDot01[0][i] = fabs(dot01[0][i]);
        if (absDot01[0][i] > cutoff)
        {
            parallelPairExists = true;
        }
    }
    m_collisiontest.SATResult.AxisDist0[0] = D3DXVec3Dot(&dist, &box0.Axis[0]);
    r = fabs(m_collisiontest.SATResult.AxisDist0[0]);
    r1 = box1.Extent[0] * absDot01[0][0] + box1.Extent[1] * absDot01[0][1] + box1.Extent[2] * absDot01[0][2];
    r01 = box0.Extent[0] + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 0; m_collisiontest.SATResult.Object1Axis = -1; }
    if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 0; result.separating[1] = -1;
    }

	// Test for separation on the axis box0.Centre + t*box0.Axis[1].
    for (int i = 0; i < 3; ++i)
    {
        dot01[1][i] = D3DXVec3Dot(&box0.Axis[1], &box1.Axis[i]);
        absDot01[1][i] = fabs(dot01[1][i]);
        if (absDot01[1][i] > cutoff)
        {
            parallelPairExists = true;
        }
    }
    m_collisiontest.SATResult.AxisDist0[1] = D3DXVec3Dot(&dist, &box0.Axis[1]);
    r = fabs(m_collisiontest.SATResult.AxisDist0[1]);
    r1 = box1.Extent[0] * absDot01[1][0] + box1.Extent[1] * absDot01[1][1] + box1.Extent[2] * absDot01[1][2];
    r01 = box0.Extent[1] + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 1; m_collisiontest.SATResult.Object1Axis = -1; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 1; result.separating[1] = -1;
    }

	// Test for separation on the axis box0.Centre + t*box0.Axis[2].
    for (int i = 0; i < 3; ++i)
    {
        dot01[2][i] = D3DXVec3Dot(&box0.Axis[2], &box1.Axis[i]);
        absDot01[2][i] = fabs(dot01[2][i]);
        if (absDot01[2][i] > cutoff)
        {
            parallelPairExists = true;
        }
    }
    m_collisiontest.SATResult.AxisDist0[2] = D3DXVec3Dot(&dist, &box0.Axis[2]);
    r = fabs(m_collisiontest.SATResult.AxisDist0[2]);
    r1 = box1.Extent[0] * absDot01[2][0] + box1.Extent[1] * absDot01[2][1] + box1.Extent[2] * absDot01[2][2];
    r01 = box0.Extent[2] + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 2; m_collisiontest.SATResult.Object1Axis = -1; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 2; result.separating[1] = -1;
    }

    // Test for separation on the axis box0.Centre + t*box1.Axis[0].
	m_collisiontest.SATResult.AxisDist1[0] = D3DXVec3Dot(&dist, &box1.Axis[0]);
	r = fabs(m_collisiontest.SATResult.AxisDist1[0]);
    r0 = box0.Extent[0] * absDot01[0][0] + box0.Extent[1] * absDot01[1][0] + box0.Extent[2] * absDot01[2][0];
    r01 = r0 + box1.Extent[0];
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = -1; m_collisiontest.SATResult.Object1Axis = 0; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = -1; result.separating[1] = 0;
    }

    // Test for separation on the axis box0.Centre + t*box1.Axis[1].
	m_collisiontest.SATResult.AxisDist1[1] = D3DXVec3Dot(&dist, &box1.Axis[1]);
	r = fabs(m_collisiontest.SATResult.AxisDist1[1]);
    r0 = box0.Extent[0] * absDot01[0][1] + box0.Extent[1] * absDot01[1][1] + box0.Extent[2] * absDot01[2][1];
    r01 = r0 + box1.Extent[1];
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = -1; m_collisiontest.SATResult.Object1Axis = 1; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = -1; result.separating[1] = 1;
    }

    // Test for separation on the axis box0.Centre + t*box1.Axis[2].
	m_collisiontest.SATResult.AxisDist1[2] = D3DXVec3Dot(&dist, &box1.Axis[2]);
	r = fabs(m_collisiontest.SATResult.AxisDist1[2]);
    r0 = box0.Extent[0] * absDot01[0][2] + box0.Extent[1] * absDot01[1][2] + box0.Extent[2] * absDot01[2][2];
    r01 = r0 + box1.Extent[2];
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = -1; m_collisiontest.SATResult.Object1Axis = 2; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = -1; result.separating[1] = 2;
    }

    // At least one pair of box axes was parallel, so the separation is
    // effectively in 2D.  The edge-edge axes do not need to be tested.
    if (parallelPairExists)
    {
        return true;
    }

    // Test for separation on the axis box0.Centre + t*box0.Axis[0]xA1[0].
    r = fabs(m_collisiontest.SATResult.AxisDist0[2] * dot01[1][0] - m_collisiontest.SATResult.AxisDist0[1] * dot01[2][0]);
    r0 = box0.Extent[1] * absDot01[2][0] + box0.Extent[2] * absDot01[1][0];
    r1 = box1.Extent[1] * absDot01[0][2] + box1.Extent[2] * absDot01[0][1];
    r01 = r0 + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 0; m_collisiontest.SATResult.Object1Axis = 0; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 0; result.separating[1] = 0;
    }

    // Test for separation on the axis box0.Centre + t*box0.Axis[0]xA1[1].
    r = fabs(m_collisiontest.SATResult.AxisDist0[2] * dot01[1][1] - m_collisiontest.SATResult.AxisDist0[1] * dot01[2][1]);
    r0 = box0.Extent[1] * absDot01[2][1] + box0.Extent[2] * absDot01[1][1];
    r1 = box1.Extent[0] * absDot01[0][2] + box1.Extent[2] * absDot01[0][0];
    r01 = r0 + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 0; m_collisiontest.SATResult.Object1Axis = 1; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 0; result.separating[1] = 1;
    }

    // Test for separation on the axis box0.Centre + t*box0.Axis[0]xA1[2].
    r = fabs(m_collisiontest.SATResult.AxisDist0[2] * dot01[1][2] - m_collisiontest.SATResult.AxisDist0[1] * dot01[2][2]);
    r0 = box0.Extent[1] * absDot01[2][2] + box0.Extent[2] * absDot01[1][2];
    r1 = box1.Extent[0] * absDot01[0][1] + box1.Extent[1] * absDot01[0][0];
    r01 = r0 + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 0; m_collisiontest.SATResult.Object1Axis = 2; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 0; result.separating[1] = 2;
    }
	
    // Test for separation on the axis box0.Centre + t*box0.Axis[1]xA1[0].
    r = fabs(m_collisiontest.SATResult.AxisDist0[0] * dot01[2][0] - m_collisiontest.SATResult.AxisDist0[2] * dot01[0][0]);
    r0 = box0.Extent[0] * absDot01[2][0] + box0.Extent[2] * absDot01[0][0];
    r1 = box1.Extent[1] * absDot01[1][2] + box1.Extent[2] * absDot01[1][1];
    r01 = r0 + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 1; m_collisiontest.SATResult.Object1Axis = 0; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 1; result.separating[1] = 0;
    }

    // Test for separation on the axis box0.Centre + t*box0.Axis[1]xA1[1].
    r = fabs(m_collisiontest.SATResult.AxisDist0[0] * dot01[2][1] - m_collisiontest.SATResult.AxisDist0[2] * dot01[0][1]);
    r0 = box0.Extent[0] * absDot01[2][1] + box0.Extent[2] * absDot01[0][1];
    r1 = box1.Extent[0] * absDot01[1][2] + box1.Extent[2] * absDot01[1][0];
    r01 = r0 + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 1; m_collisiontest.SATResult.Object1Axis = 1; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 1; result.separating[1] = 1;
    }

    // Test for separation on the axis box0.Centre + t*box0.Axis[1]xA1[2].
    r = fabs(m_collisiontest.SATResult.AxisDist0[0] * dot01[2][2] - m_collisiontest.SATResult.AxisDist0[2] * dot01[0][2]);
    r0 = box0.Extent[0] * absDot01[2][2] + box0.Extent[2] * absDot01[0][2];
    r1 = box1.Extent[0] * absDot01[1][1] + box1.Extent[1] * absDot01[1][0];
    r01 = r0 + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 1; m_collisiontest.SATResult.Object1Axis = 2; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 1; result.separating[1] = 2;
    }

    // Test for separation on the axis box0.Centre + t*box0.Axis[2]xA1[0].
    r = fabs(m_collisiontest.SATResult.AxisDist0[1] * dot01[0][0] - m_collisiontest.SATResult.AxisDist0[0] * dot01[1][0]);
    r0 = box0.Extent[0] * absDot01[1][0] + box0.Extent[1] * absDot01[0][0];
    r1 = box1.Extent[1] * absDot01[2][2] + box1.Extent[2] * absDot01[2][1];
    r01 = r0 + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 2; m_collisiontest.SATResult.Object1Axis = 0; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 2; result.separating[1] = 0;
    }

    // Test for separation on the axis box0.Centre + t*box0.Axis[2]xA1[1].
    r = fabs(m_collisiontest.SATResult.AxisDist0[1] * dot01[0][1] - m_collisiontest.SATResult.AxisDist0[0] * dot01[1][1]);
    r0 = box0.Extent[0] * absDot01[1][1] + box0.Extent[1] * absDot01[0][1];
    r1 = box1.Extent[0] * absDot01[2][2] + box1.Extent[2] * absDot01[2][0];
    r01 = r0 + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 2; m_collisiontest.SATResult.Object1Axis = 1; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 2; result.separating[1] = 1;
    }

    // Test for separation on the axis box0.Centre + t*box0.Axis[2]xA1[2].
    r = fabs(m_collisiontest.SATResult.AxisDist0[1] * dot01[0][2] - m_collisiontest.SATResult.AxisDist0[0] * dot01[1][2]);
    r0 = box0.Extent[0] * absDot01[1][2] + box0.Extent[1] * absDot01[0][2];
    r1 = box1.Extent[0] * absDot01[2][1] + box1.Extent[1] * absDot01[2][0];
    r01 = r0 + r1;
	r01_r = (r01 - r);
	if (r01_r < m_collisiontest.Penetration) { m_collisiontest.Penetration = r01_r; m_collisiontest.SATResult.Object0Axis = 2; m_collisiontest.SATResult.Object1Axis = 2; }
	if (r01_r < 0.0f)
    {
        return false;		// result.separating[0] = 2; result.separating[1] = 2;
    }

	// The OBBs are overlapping in all possible axes, so we have a collision
    return true;
}

// Tests for the intersection of a bounding sphere with an OBB collision hierarchy 
bool GamePhysicsEngine::TestSpherevsOBBHierarchyCollision(	const D3DXVECTOR3 & sphereCentre, const float sphereRadiusSq, 
															OrientedBoundingBox & obb, OrientedBoundingBox ** ppOutOBBCollider)
{
	// Test the intersection at this level of the OBB hierarchy; if it fails then return false immediately
	if (TestSpherevsOBBCollision(sphereCentre, sphereRadiusSq, obb.Data) == false) return false;

	// Now test for any children in the hierarchy
	if (!obb.HasChildren())
	{
		// The obb has no children, and it collides, so return true immediately
		(*ppOutOBBCollider) = &obb;
		return true;
	}
	else
	{
		// Iterate through each child element and move down its hierarchy.  If a branch fails to collide at any point
		// it rolls back up and the next one is tried
		for (int i = 0; i < obb.ChildCount; ++i)
		{
			if (TestSpherevsOBBHierarchyCollision(sphereCentre, sphereRadiusSq, obb.Children[i], ppOutOBBCollider) == true)
			{
				// This branch succeeded all the way down to a leaf OBB which is colliding, so return success now
				return true;
			}
		}

		// None of the branches resulted in a leaf-level collision, so return false to indicate no collisions
		return false;
	}
}


// Tests for the intersection of a bounding sphere and an oriented bounding box (OBB)
// Input taken from http://www.gamedev.net/topic/579584-obb---sphere-collision-detection/
bool GamePhysicsEngine::TestSpherevsOBBCollision(	const D3DXVECTOR3 & sphereCentre, const float sphereRadiusSq, 
													const OrientedBoundingBox::CoreOBBData & obb)
{
	// We will update the collision detection data struct with the results of this test
	m_collisiontest.Type = CollisionDetectionType::SphereVsOBB;

	// Get the closest point in/on the sphere to the OBB, and subtract the sphere centre to get a vector
	// from the centre of the sphere to the closest/intersection point with the OBB
	D3DXVECTOR3 v = (ClosestPointOnOBB(obb, sphereCentre) - sphereCentre);

	// Dot(v,v) = v.LengthSq.  If Dot(v,v) <= SphereRadiusSq then it means the closest point to the 
	// OBB is actually intersecting the OBB.  
	m_collisiontest.Penetration = (sphereRadiusSq - D3DXVec3Dot(&v, &v));
	return (m_collisiontest.Penetration > 0.0f);
}

// Tests for the intersection of a ray with an AABB.  Results will be populated with min/max intersection points if an intersection
// took place.  If min<max then we have an intersection.  Returns a flag indicating whether the intersection took place.  If 
// min<0 then the ray began inside the AABB.  
// Input from http://tavianator.com/cgit/dimension.git/tree/libdimension/bvh.c#n191
bool GamePhysicsEngine::TestRayVsAABBIntersection(const Ray & ray, const AABB & aabb, float t)
{
	float tx1, tx2, ty1, ty2, tz1, tz2;

	tx1 = (aabb.Points[0].x - ray.Origin.x)*ray.InvDirection.x;
	tx2 = (aabb.Points[1].x - ray.Origin.x)*ray.InvDirection.x;

	RayIntersectionResult.tmin = min(tx1, tx2);
	RayIntersectionResult.tmax = max(tx1, tx2);

	ty1 = (aabb.Points[0].y - ray.Origin.y)*ray.InvDirection.y;
	ty2 = (aabb.Points[1].y - ray.Origin.y)*ray.InvDirection.y;

	RayIntersectionResult.tmin = max(RayIntersectionResult.tmin, min(ty1, ty2));
	RayIntersectionResult.tmax = min(RayIntersectionResult.tmax, max(ty1, ty2));

	tz1 = (aabb.Points[0].z - ray.Origin.z)*ray.InvDirection.z;
	tz2 = (aabb.Points[1].z - ray.Origin.z)*ray.InvDirection.z;

	RayIntersectionResult.tmin = max(RayIntersectionResult.tmin, min(tz1, tz2));
	RayIntersectionResult.tmax = min(RayIntersectionResult.tmax, max(tz1, tz2));

	return (RayIntersectionResult.tmax >= max(0.0, RayIntersectionResult.tmin) && RayIntersectionResult.tmin < t);
}

// Perform a continuous collision test between two moving spheres.  Populates the collision result data with details on the collision
// points and time t = [0 1] at which the collision occured, if at all.  Returns a flag indicating whether a collision took place.
// Input from http://studiofreya.com/3d-math-and-physics/little-more-advanced-collision-detection-spheres/
bool GamePhysicsEngine::TestContinuousSphereCollision(const iActiveObject *object0, const iActiveObject *object1)
{
	// Parameter check
	if (!object0 || !object1) return false;
	m_collisiontest.Type = GamePhysicsEngine::CollisionDetectionType::ContinuousSphereVsSphere;
	ContinuousCollisionTestInterimData & D = m_collisiontest.ContinuousTestResult.InterimCalculations;

	// We do not use the current object positions, since they have already moved.  Instead we roll back the position to
	// the start of the frame (t=0) and then perform continuous collision detection for their movement during the 
	// frame (until t=1).  
	D.wm0 = (object0->PhysicsState.WorldMomentum * PhysicsClock.TimeFactor);
	D.wm1 = (object1->PhysicsState.WorldMomentum * PhysicsClock.TimeFactor);
	D.pos0 = (object0->GetPosition() - D.wm0);
	D.pos1 = (object1->GetPosition() - D.wm1);

	// Get the vector between these (adjusted) object centres, the relative velocity between objects, and the combined collision radii
	D.s = (D.pos0 - D.pos1);
	D.v = (D.wm0 - D.wm1);
	D.r = (object0->GetCollisionSphereRadius() + object1->GetCollisionSphereRadius());

	// If c is negative, the objects already overlap and we can return immediately
	D.c = D3DXVec3Dot(&D.s, &D.s) - (D.r * D.r);
	if (D.c < 0.0f)
	{
		m_collisiontest.ContinuousTestResult.IntersectionTime = 0.0f;
		return true;
	}

	D.a = D3DXVec3Dot(&D.v, &D.v);
	D.b = D3DXVec3Dot(&D.v, &D.s);

	// If b is >= 0 then the objects are not moving towards each other, so we can again return immediately
	if (D.b >= 0.0f) return false;

	// Calculate d as the result of the simultaneous linear equations; if it is negative then there are no 
	// real roots to the solution "t = (-b - sqrt(d)) / a" so we can report no collision
	D.d = D.b*D.b - D.a*D.c;
	if (D.d < 0.0f) return false;

	// The time of intersection can be found as the solution "t = (-b - sqrt(d)) / a"
	m_collisiontest.ContinuousTestResult.IntersectionTime = (-D.b - sqrt(D.d)) / D.a;

	// We know there is a collision in the future.  However it is only a collision in this test if it occurs within the frame time
	if (m_collisiontest.ContinuousTestResult.IntersectionTime > 1.0f) return false;

	// Get the object collision sphere centres at the time of intersection.  Calculated by advancing the object
	// positions by the proportion of their world momentum that is covered by collision time t
	m_collisiontest.ContinuousTestResult.CollisionPos0 = (D.pos0 + (D.wm0 * m_collisiontest.ContinuousTestResult.IntersectionTime));
	m_collisiontest.ContinuousTestResult.CollisionPos1 = (D.pos1 + (D.wm1 * m_collisiontest.ContinuousTestResult.IntersectionTime));
	
	// Determine the contact point between the two objects; this will be the point on the contact normal that is 
	// Radius0 away from Object0
	m_collisiontest.ContinuousTestResult.ContactNormal = (m_collisiontest.ContinuousTestResult.CollisionPos1 - m_collisiontest.ContinuousTestResult.CollisionPos0);
	D3DXVec3Normalize(&m_collisiontest.ContinuousTestResult.NormalisedContactNormal, &m_collisiontest.ContinuousTestResult.ContactNormal);
	m_collisiontest.ContinuousTestResult.ContactPoint = 
		(m_collisiontest.ContinuousTestResult.CollisionPos0 + (m_collisiontest.ContinuousTestResult.NormalisedContactNormal * object0->GetCollisionSphereRadius()));

	// We can now report the collision that was detected
	return true;
}

// Perform a continuous collision test between a moving sphere and a static OBB.  Populates the collision result data.  Returns
// a flag indicating whether a collision took place
bool GamePhysicsEngine::TestContinuousSphereVsOBBCollision(const iActiveObject *sphere, const iActiveObject *obb)
{
	// Parameter check
	if (!sphere || !obb) return false;
	m_collisiontest.Type = GamePhysicsEngine::CollisionDetectionType::ContinuousSphereVsOBB;
	ContinuousCollisionTestInterimData & D = m_collisiontest.ContinuousTestResult.InterimCalculations;

	// We do not use the current sphere position, since it has already moved.  Instead we roll back the position to
	// the start of the frame (t=0) and then perform continuous collision detection for its movement during the 
	// frame (until t=1).  The OBB position is taken as-is since we assume it is static in this test
	D.wm0 = (sphere->PhysicsState.WorldMomentum * PhysicsClock.TimeFactor);
	D.pos0 = (sphere->GetPosition() - D.wm0);
	
	// We will use a ray/box intersection test.  Generate a ray to simulate the sphere path and transform
	// it into the OBB's coordinate frame.  We are now in the OBB's frame so can treat it as an AABB
	AABB box = AABB(obb->CollisionOBB.Data); 
	Ray ray = Ray(D.pos0, D.wm0);
	ray.TransformIntoCoordinateSystem(obb->CollisionOBB.Data.Centre, obb->CollisionOBB.Data.Axis);
	
	// Extend the AABB extent to account for the radius of the moving sphere, so we can then treat the
	// ray as having point-width for simplicity
	const float & radius = sphere->GetCollisionSphereRadius();
	box.Points[0].x -= radius; box.Points[0].y -= radius; box.Points[0].z -= radius;
	box.Points[1].x += radius; box.Points[1].y += radius; box.Points[1].z += radius;

	// Now test the ray/AABB intersection
	if (TestRayVsAABBIntersection(ray, box) == false) return false;

	// We have a collision.  Populate the intersection point in the results struct.  If tmin < 0 then
	// the sphere begain inside the AABB at t=0.  Otherwise, first intersection is at time tmin, therefore
	// at point (x0 + n*tmin) along the (original) ray.  x0==pos0 and n==wm0
	m_collisiontest.ContinuousTestResult.IntersectionTime = RayIntersectionResult.tmin;
	m_collisiontest.ContinuousTestResult.ContactPoint = (D.pos0 + (D.wm0 * RayIntersectionResult.tmin));
	return true;
}

float GamePhysicsEngine::GetCCDTestDistance(const iActiveObject *object) const
{
	// Get all objects within a potential collision volume, based upon the current object velocity & with a buffer for ricochets
	D3DXVECTOR3 wm = (object->PhysicsState.WorldMomentum * PhysicsClock.TimeFactor);	// Base the estimate on the object's current velocity
	FloorVector(wm, 1.0f);																// Floor to 1.0 so that the squared distance will always be greater
	return ((wm.x*wm.x) + (wm.y*wm.y) + (wm.z*wm.z)) * 2.0f;							// Add buffer of 2x to be sure of catching any ricochets etc
}


// Determines the closest point on a line segment to the specified point
D3DXVECTOR3 GamePhysicsEngine::ClosestPointOnLineSegment(const D3DXVECTOR3 & line_ep1, const D3DXVECTOR3 & line_ep2, 
														 const D3DXVECTOR3 & point)
{
	// Inputs taken from http://notmagi.me/closest-point-on-line-aabb-and-obb-to-point/

	// Determine the relative distance along the line segment by taking a perpendicular dot product
	D3DXVECTOR3 ab = (line_ep2 - line_ep1);
	float t = ( D3DXVec3Dot(&(point - line_ep1), &ab) / D3DXVec3Dot(&ab, &ab) );

	// Constraint the point to line on the line segment; t = in the range [0.0 1.0]
	if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;

	// Use the standard line equation to get the point 't' along the line
	return (line_ep1 + (t * ab));
}

// Determines the closest point on an AABB to the specified point
D3DXVECTOR3 GamePhysicsEngine::ClosestPointOnAABB(	const D3DXVECTOR3 & AABB_min, const D3DXVECTOR3 & AABB_max, 
													const D3DXVECTOR3 & point )
{
	// Inputs taken from http://notmagi.me/closest-point-on-line-aabb-and-obb-to-point/
	
	// By default, set the point itself to be the closest point, and then we will adjust if it is outside the AABB
	D3DXVECTOR3 result = point;

	// Compare the x dimension
	if		(point.x < AABB_min.x)	result.x = AABB_min.x;
	else if (point.x > AABB_max.x)	result.x = AABB_max.x;

	// Compare the y dimension
	if		(point.y < AABB_min.y)	result.y = AABB_min.y;
	else if (point.y > AABB_max.y)	result.y = AABB_max.y;

	// Compare the z dimension
	if		(point.z < AABB_min.z)	result.z = AABB_min.z;
	else if (point.z > AABB_max.z)	result.z = AABB_max.z;

	// Return the resulting point
	return result;
}


// Determines the closest point on an OBB to the specified location
D3DXVECTOR3 GamePhysicsEngine::ClosestPointOnOBB(const OrientedBoundingBox::CoreOBBData & obb, const D3DXVECTOR3 & point)
{
	// Input taken from http://www.gamedev.net/topic/579584-obb---sphere-collision-detection/, http://notmagi.me/closest-point-on-line-aabb-and-obb-to-point/
	float dist;

	// Get the vector from this point to the centre of the OBB
	D3DXVECTOR3 d = (point - obb.Centre);

	// Start the result point at the centre of the obb
	D3DXVECTOR3 result = obb.Centre;

	// Check each axis in turn.  Unroll loop just in case the compiler doesn't.  First, axis[0]
	dist = D3DXVec3Dot(&d, &obb.Axis[0]);
	if		(dist > obb.Extent[0])			result += (obb.Extent[0] * obb.Axis[0]);	// Full +ve extent along axis
	else if	(dist < -obb.Extent[0])			result -= (obb.Extent[0] * obb.Axis[0]);	// Full -ve extent along axis
	else									result += (dist * obb.Axis[0]);				// Partial extent along axis

	// Repeat for axis[1]
	dist = D3DXVec3Dot(&d, &obb.Axis[1]);
	if		(dist > obb.Extent[1])			result += (obb.Extent[1] * obb.Axis[1]);	// Full +ve extent along axis
	else if	(dist < -obb.Extent[1])			result -= (obb.Extent[1] * obb.Axis[1]);	// Full -ve extent along axis
	else									result += (dist * obb.Axis[1]);				// Partial extent along axis

	// Finally repeat for axis[2]
	dist = D3DXVec3Dot(&d, &obb.Axis[2]);
	if		(dist > obb.Extent[2])			result += (obb.Extent[2] * obb.Axis[2]);	// Full +ve extent along axis
	else if	(dist < -obb.Extent[2])			result -= (obb.Extent[2] * obb.Axis[2]);	// Full -ve extent along axis
	else									result += (dist * obb.Axis[2]);				// Partial extent along axis

// TODO: can use ExtentAlongAxis rather than the multiplications above?
	
	// Return the final point
	return result;
}

// Determines the closest point on an OBB to the specified location.  Also returns an output parameter that indicates how close the point
// is to the OBB centre in each of the OBB's basis axes.  Any distance < the extent in that axis means the point is inside the OBB.
D3DXVECTOR3 GamePhysicsEngine::ClosestPointOnOBB(const OrientedBoundingBox::CoreOBBData & obb, const D3DXVECTOR3 & point, D3DXVECTOR3 & outDistance)
{
	// Get the vector from this point to the centre of the OBB
	D3DXVECTOR3 d = (point - obb.Centre);

	// Start the result point at the centre of the obb
	D3DXVECTOR3 result = obb.Centre;

	// Check each axis in turn.  Unroll loop just in case the compiler doesn't.  First, axis[0]
	outDistance.x = D3DXVec3Dot(&d, &obb.Axis[0]);
	if (outDistance.x > obb.Extent[0])			result += (obb.Extent[0] * obb.Axis[0]);		// Full +ve extent along axis
	else if (outDistance.x < -obb.Extent[0])	result -= (obb.Extent[0] * obb.Axis[0]);		// Full -ve extent along axis
	else										result += (outDistance.x * obb.Axis[0]);		// Partial extent along axis

	// Repeat for axis[1]
	outDistance.y = D3DXVec3Dot(&d, &obb.Axis[1]);
	if (outDistance.y > obb.Extent[1])			result += (obb.Extent[1] * obb.Axis[1]);		// Full +ve extent along axis
	else if (outDistance.y < -obb.Extent[1])	result -= (obb.Extent[1] * obb.Axis[1]);		// Full -ve extent along axis
	else										result += (outDistance.y * obb.Axis[1]);		// Partial extent along axis

	// Finally repeat for axis[2]
	outDistance.z = D3DXVec3Dot(&d, &obb.Axis[2]);
	if (outDistance.z > obb.Extent[2])			result += (obb.Extent[2] * obb.Axis[2]);		// Full +ve extent along axis
	else if (outDistance.z < -obb.Extent[2])	result -= (obb.Extent[2] * obb.Axis[2]);		// Full -ve extent along axis
	else										result += (outDistance.z * obb.Axis[2]);		// Partial extent along axis

	// Return the final point
	return result;
}

#ifdef RJ_OLD_COLLISION_HANDLING
// Determines and applies collision response based upon a collision between object0.Collider0 and object1.Collider1.  
// Called from main PerformCollisionDetection() method.
void GamePhysicsEngine::HandleCollision(iActiveObject *object0, iActiveObject *object1,
	const OrientedBoundingBox::CoreOBBData *collider0, const OrientedBoundingBox::CoreOBBData *collider1)
{
	// No parameter checks here; we rely on the integrity of main collision detection method (which should be the only method
	// to invoke this one) to ensure that object[0|1] are non-null valid objects.  For efficiency.  
	// collider[0|1] can be null if there is no relevant colliding OBB (e.g. if the object is broadphase collision-only)

	// Special case; if either object is a ship section & part of a larger complex ship, move up the hierarchy one level
	// and treat the ship itself as being the colliding object.  Ship statistics (e.g. mass) are derived from the combination of all
	// its sections, so this is the correct object to be involving in the collision
	if (object0->GetObjectType() == iObject::ObjectType::ComplexShipSectionObject)
	{
		ComplexShipSection *sec = (ComplexShipSection*)object0;
		if (sec->GetParent()) object0 = (iActiveObject*)sec->GetParent();
	}
	if (object1->GetObjectType() == iObject::ObjectType::ComplexShipSectionObject)
	{
		ComplexShipSection *sec = (ComplexShipSection*)object1;
		if (sec->GetParent()) object1 = (iActiveObject*)sec->GetParent();
	}

	// Store the momentum of each object before applying a response, to allow calculation of the impact force
	D3DXVECTOR3 obj0_pre_wm = object0->PhysicsState.WorldMomentum;
	D3DXVECTOR3 obj1_pre_wm = object1->PhysicsState.WorldMomentum;

	/* Apply collision response using the impulse method - first, apply the normal component */

	// Get a reference to the object centre points, and the normal between them
	D3DXVECTOR3 normal, normal_n;
	const D3DXVECTOR3 & c0 = (collider0 ? collider0->Centre : object0->GetPosition());
	const D3DXVECTOR3 & c1 = (collider1 ? collider1->Centre : object1->GetPosition());
	normal = (c0 - c1);
	D3DXVec3Normalize(&normal_n, &normal);

	// Determine hit point on the surface of each object 
	D3DXVECTOR3 hit0, hit1;
	if (collider0)			  hit0 = ClosestPointOnOBB(*collider0, c1);
	else					  hit0 = (c0 - (normal_n * object0->GetCollisionSphereRadius()));
	if (collider1)			  hit1 = ClosestPointOnOBB(*collider1, c0);
	else					  hit1 = (c1 + (normal_n * object1->GetCollisionSphereRadius()));

	// Get vectors from object centres to their hitpoints
	D3DXVECTOR3 r0 = (hit0 - c0);
	D3DXVECTOR3 r1 = (hit1 - c1);

	// Store mass & inverse mass reference parameters for convenience
	const float & mass0 = object0->GetMass();
	const float & mass1 = object1->GetMass();
	const float & invMass0 = object0->GetInverseMass();
	const float & invMass1 = object1->GetInverseMass();

	// Cross the angular velocity of each object with its hitpoint contact vector
	D3DXVECTOR3 angR0, angR1;
	D3DXVec3Cross(&angR0, &object0->PhysicsState.AngularVelocity, &r0);
	D3DXVec3Cross(&angR1, &object1->PhysicsState.AngularVelocity, &r1);

	// Determine the component of the relative object velocity that is along the normal vector
	D3DXVECTOR3 v0 = (object0->PhysicsState.WorldMomentum * mass0) + angR0;
	D3DXVECTOR3 v1 = (object1->PhysicsState.WorldMomentum * mass1) + angR1;
	D3DXVECTOR3 vrel = (v0 - v1);
	float vn = D3DXVec3Dot(&vrel, &normal);

	// If the objects are moving away from each other then there is no collision response required
	if (-vn < 0.01f) return;

	// Transform the inertia tensor for each object into world space
	D3DXMATRIX worldInvI0, worldInvI1;
	D3DXMatrixMultiply(&worldInvI0, object0->GetOrientationMatrix(), &(object0->PhysicsState.InverseInertiaTensor));
	D3DXMatrixMultiply(&worldInvI1, object1->GetOrientationMatrix(), &(object1->PhysicsState.InverseInertiaTensor));

	// Derive the impulse 'jn' that should be applied along the contact normal, incorporating both linear and angular momentum
	D3DXVECTOR3 Crn0, Crn1, ICrn0, ICrn1, CICrn0, CICrn1;
	D3DXVec3Cross(&Crn0, &r0, &normal);
	D3DXVec3Cross(&Crn1, &r1, &normal);
	D3DXVec3TransformCoord(&ICrn0, &Crn0, &worldInvI0);
	D3DXVec3TransformCoord(&ICrn1, &Crn1, &worldInvI1);
	D3DXVec3Cross(&CICrn0, &ICrn0, &r0);
	D3DXVec3Cross(&CICrn1, &ICrn1, &r1);

	float jn = (-1.0f * (1.0f + Game::C_COLLISION_SPACE_COEFF_ELASTICITY) * vn)
		/
		(D3DXVec3Dot(&normal, &normal) * ((invMass0 + invMass1) +
		D3DXVec3Dot(&normal, &(CICrn0 + CICrn1))));

	// Calculate change in angular velocity
	D3DXVECTOR3 angInc0, angInc1;
	D3DXVec3Cross(&angInc0, &r0, &(normal * jn));
	D3DXVec3Cross(&angInc1, &r1, &(normal * jn));
	D3DXVec3TransformCoord(&angInc0, &angInc0, &worldInvI0);
	D3DXVec3TransformCoord(&angInc1, &angInc1, &worldInvI1);

	// Apply change in linear and angular velocity to each object
	object0->PhysicsState.WorldMomentum += (normal * jn * invMass0);
	object0->PhysicsState.AngularVelocity += angInc0;
	object1->PhysicsState.WorldMomentum -= (normal * jn * invMass1);
	object1->PhysicsState.AngularVelocity -= angInc1;


	/* Now apply tangent component to simulate friction at the contact point */

	// Determine tangent vector, perpendicular to the collision normal
	D3DXVECTOR3 tangent = vrel - (D3DXVec3Dot(&vrel, &normal) * normal);
	D3DXVec3Normalize(&tangent, &tangent);

	// Safety feature: if objects somehow become merged together (e.g. spawned inside each other) the derived tangent can exceed -1e38 and leave
	// us with an undefined float error.  In such cases, we will apply a default tangent to prevent overflow
	// Use the fact that (x == x) will trivially return true for all finite numbers, but where x = -1.#IND000, (-1.#IND000 != -1.#IND000)
	// TODO: this isn't perfect, and prevents some collisions registering at shallow impact angles.  Issue with calc?  Or need double precision here?
	if (!(tangent.x == tangent.x)) tangent = D3DXVECTOR3(1.0f, 0.0f, 0.0f);

	// Calculate intermediate cross products 
	D3DXVECTOR3 Cr0tan, Cr1tan, transformedCr0tan, transformedCr1tan, Ctransr0, Ctransr1;
	D3DXVec3Cross(&Cr0tan, &r0, &tangent);
	D3DXVec3Cross(&Cr1tan, &r1, &tangent);
	D3DXVec3TransformCoord(&transformedCr0tan, &Cr0tan, &worldInvI0);
	D3DXVec3TransformCoord(&transformedCr1tan, &Cr1tan, &worldInvI1);
	D3DXVec3Cross(&Ctransr0, &transformedCr0tan, &r0);
	D3DXVec3Cross(&Ctransr1, &transformedCr1tan, &r1);

	// Now determine the tangential impulse 'jt'
	float jt = (-1.0f * D3DXVec3Dot(&vrel, &tangent))
		/
		(invMass0 + invMass1 + D3DXVec3Dot(&tangent, &(Ctransr0 + Ctransr1)));

	// Calculate change in angular velocity
	D3DXVECTOR3 tjt = (tangent * jt);
	D3DXVec3Cross(&angInc0, &r0, &tjt);
	D3DXVec3Cross(&angInc1, &r1, &tjt);
	D3DXVec3TransformCoord(&angInc0, &angInc0, &worldInvI0);
	D3DXVec3TransformCoord(&angInc1, &angInc1, &worldInvI1);

	// Apply the change in linear and angular impulse to each object
	object0->PhysicsState.WorldMomentum += (tangent * jt * invMass0);
	object0->PhysicsState.AngularVelocity += angInc0;
	object1->PhysicsState.WorldMomentum -= (tangent * jt * invMass1);
	object1->PhysicsState.AngularVelocity -= angInc1;

	// Log details of the collision to the debug output, if the relevant compiler flag is set
#ifdef RJ_LOG_COLLISION_DETAILS 
	OutputDebugString("\n\n*** Collision = { \n");
	OutputDebugString(concat("\t Object0 = \"")(object0->GetName())("\" [")((int)object0->GetCollisionMode())("], Object1 = \"")
		(object1->GetName())("\" [")((int)object1->GetCollisionMode())("] \n").str().c_str());
	OutputDebugString(concat("\t collision normal = [")(normal.x)(",")(normal.y)(",")(normal.z)("]\n").str().c_str());
	OutputDebugString(concat("\t v0 = (object0 momentum [")(object0->PhysicsState.WorldMomentum.x)(",")(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)
		("] * mass [")(mass0)("]) + angvel at contact vector [")(angR0.x)(",")(angR0.y)(",")(angR0.z)("] = [")(v0.x)(",")(v0.y)(",")(v0.z)("]\n").str().c_str());
	OutputDebugString(concat("\t v1 = (object1 momentum [")(object1->PhysicsState.WorldMomentum.x)(",")(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)
		("] * mass [")(mass1)("]) + angvel at contact vector [")(angR1.x)(",")(angR1.y)(",")(angR1.z)("] = [")(v1.x)(",")(v1.y)(",")(v1.z)("]\n").str().c_str());
	OutputDebugString(concat("\t vrel = (v0 [")(v0.x)(",")(v0.y)(",")(v0.z)("] - v1 [")(v1.x)(",")(v1.y)(",")(v1.z)("]) = [")(vrel.x)(",")(vrel.y)(",")(vrel.z)("]\n").str().c_str());
	OutputDebugString(concat("\t vn = dot(vrel, normal) = ")(vn)(" (+ve value means objects are diverging and there is no collision)\n").str().c_str());
	OutputDebugString(concat("\t Object0 pre-collision state: world momentum = [")(object0->PhysicsState.WorldMomentum.x)(",")
		(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object0->PhysicsState.AngularVelocity.x)
		(",")(object0->PhysicsState.AngularVelocity.y)(",")(object0->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object1 pre-collision state: world momentum = [")(object1->PhysicsState.WorldMomentum.x)(",")
		(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object1->PhysicsState.AngularVelocity.x)
		(",")(object1->PhysicsState.AngularVelocity.y)(",")(object1->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	OutputDebugString(concat("\t jn = symmetric impulse along collision normal = ")(jn)("\n").str().c_str());
	OutputDebugString(concat("\t AngInc0 = change in normal angular velocity for object0 = [")(angInc0.x)(",")(angInc0.y)(",")(angInc0.z)("]\n").str().c_str());
	OutputDebugString(concat("\t AngInc1 = change in normal angular velocity for object1 = [")(angInc1.x)(",")(angInc1.y)(",")(angInc1.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object0 post-normal collision state: world momentum = [")(object0->PhysicsState.WorldMomentum.x)(",")
		(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object0->PhysicsState.AngularVelocity.x)
		(",")(object0->PhysicsState.AngularVelocity.y)(",")(object0->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object1 post-normal collision state: world momentum = [")(object1->PhysicsState.WorldMomentum.x)(",")
		(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object1->PhysicsState.AngularVelocity.x)
		(",")(object1->PhysicsState.AngularVelocity.y)(",")(object1->PhysicsState.AngularVelocity.z)("]\n").str().c_str());

	OutputDebugString(concat("\n\t Normalised tangent vector = [")(tangent.x)(",")(tangent.y)(",")(tangent.z)("]\n").str().c_str());
	OutputDebugString(concat("\t jt = symmetric impulse along collision tangent = ")(jt)("\n").str().c_str());
	OutputDebugString(concat("\t AngInc0 = change in tangential angular velocity for object0 = [")(angInc0.x)(",")(angInc0.y)(",")(angInc0.z)("]\n").str().c_str());
	OutputDebugString(concat("\t AngInc1 = change in tangential angular velocity for object1 = [")(angInc1.x)(",")(angInc1.y)(",")(angInc1.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object0 post-tangential collision state: world momentum = [")(object0->PhysicsState.WorldMomentum.x)(",")
		(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object0->PhysicsState.AngularVelocity.x)
		(",")(object0->PhysicsState.AngularVelocity.y)(",")(object0->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object1 post-tangential collision state: world momentum = [")(object1->PhysicsState.WorldMomentum.x)(",")
		(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object1->PhysicsState.AngularVelocity.x)
		(",")(object1->PhysicsState.AngularVelocity.y)(",")(object1->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	if (object0->PhysicsState.WorldMomentum.x > Game::C_MAX_LINEAR_VELOCITY || object0->PhysicsState.WorldMomentum.y > Game::C_MAX_LINEAR_VELOCITY ||
		object0->PhysicsState.WorldMomentum.z > Game::C_MAX_LINEAR_VELOCITY || object0->PhysicsState.AngularVelocity.x > Game::C_MAX_ANGULAR_VELOCITY ||
		object0->PhysicsState.AngularVelocity.y > Game::C_MAX_ANGULAR_VELOCITY || object0->PhysicsState.AngularVelocity.z > Game::C_MAX_ANGULAR_VELOCITY)
		OutputDebugString("\t Object0 exceeds physical limits on linear and/or angular velocity; state will be scaled within limits\n");
	if (object1->PhysicsState.WorldMomentum.x > Game::C_MAX_LINEAR_VELOCITY || object1->PhysicsState.WorldMomentum.y > Game::C_MAX_LINEAR_VELOCITY ||
		object1->PhysicsState.WorldMomentum.z > Game::C_MAX_LINEAR_VELOCITY || object1->PhysicsState.AngularVelocity.x > Game::C_MAX_ANGULAR_VELOCITY ||
		object1->PhysicsState.AngularVelocity.y > Game::C_MAX_ANGULAR_VELOCITY || object1->PhysicsState.AngularVelocity.z > Game::C_MAX_ANGULAR_VELOCITY)
		OutputDebugString("\t Object1 exceeds physical limits on linear and/or angular velocity; state will be scaled within limits\n");
	OutputDebugString("}\n\n");
#endif

	// Scale all resulting object forces to be within allowable linear/angular velocity ranges
	ScaleVectorWithinMagnitudeLimit(object0->PhysicsState.WorldMomentum, Game::C_MAX_LINEAR_VELOCITY);
	ScaleVectorWithinMagnitudeLimit(object1->PhysicsState.WorldMomentum, Game::C_MAX_LINEAR_VELOCITY);
	ScaleVectorWithinMagnitudeLimit(object0->PhysicsState.AngularVelocity, Game::C_MAX_ANGULAR_VELOCITY);
	ScaleVectorWithinMagnitudeLimit(object1->PhysicsState.AngularVelocity, Game::C_MAX_ANGULAR_VELOCITY);

	// We have made multiple changes to the world momentum of both objects; recalculate the resulting local momentum for each now
	object0->RecalculateLocalMomentum();
	object1->RecalculateLocalMomentum();

	// Determine the impact force on each object by comparing pre- & post-collision momentum
	ObjectImpact.Object.PreImpactVelocity = obj0_pre_wm;
	ObjectImpact.Collider.PreImpactVelocity = obj1_pre_wm;
	ObjectImpact.Object.VelocityChange = (object0->PhysicsState.WorldMomentum - obj0_pre_wm);
	ObjectImpact.Collider.VelocityChange = (object1->PhysicsState.WorldMomentum - obj1_pre_wm);
	ObjectImpact.Object.VelocityChangeMagnitude = D3DXVec3Length(&ObjectImpact.Object.VelocityChange);
	ObjectImpact.Collider.VelocityChangeMagnitude = D3DXVec3Length(&ObjectImpact.Collider.VelocityChange);
	ObjectImpact.Object.ImpactForce = (ObjectImpact.Object.VelocityChangeMagnitude * object0->GetMass());
	ObjectImpact.Collider.ImpactForce = (ObjectImpact.Collider.VelocityChangeMagnitude * object1->GetMass());
	ObjectImpact.TotalImpactForce = (ObjectImpact.Object.ImpactForce + ObjectImpact.Collider.ImpactForce);

	// Notify object 0 of the collision
	object0->CollisionWithObject(object1, ObjectImpact);

	// Swap the definition of object & collider and then notify object1 of the impact
	std::swap(ObjectImpact.Object, ObjectImpact.Collider);
	object1->CollisionWithObject(object1, ObjectImpact);
}


#endif
#ifdef RJ_OLD_COLLISION_HANDLING_2

void GamePhysicsEngine::HandleCollision(iActiveObject *object0, iActiveObject *object1,
	const OrientedBoundingBox::CoreOBBData *collider0, const OrientedBoundingBox::CoreOBBData *collider1)
{
	// No parameter checks here; we rely on the integrity of main collision detection method (which should be the only method
	// to invoke this one) to ensure that object[0|1] are non-null valid objects.  For efficiency.  
	// collider[0|1] can be null if there is no relevant colliding OBB (e.g. if the object is broadphase collision-only)

	// Special case; if either object is a ship section & part of a larger complex ship, move up the hierarchy one level
	// and treat the ship itself as being the colliding object.  Ship statistics (e.g. mass) are derived from the combination of all
	// its sections, so this is the correct object to be involving in the collision
	if (object0->GetObjectType() == iObject::ObjectType::ComplexShipSectionObject)
	{
		ComplexShipSection *sec = (ComplexShipSection*)object0;
		if (sec->GetParent()) object0 = (iActiveObject*)sec->GetParent();
	}
	if (object1->GetObjectType() == iObject::ObjectType::ComplexShipSectionObject)
	{
		ComplexShipSection *sec = (ComplexShipSection*)object1;
		if (sec->GetParent()) object1 = (iActiveObject*)sec->GetParent();
	}

	// Store the momentum of each object before applying a response, to allow calculation of the impact force
	D3DXVECTOR3 obj0_pre_wm = object0->PhysicsState.WorldMomentum;
	D3DXVECTOR3 obj1_pre_wm = object1->PhysicsState.WorldMomentum;

	/* Apply collision response using the impulse method - first, apply the normal component */

	// Get a reference to the object centre points, and the normal between them
	D3DXVECTOR3 normal, normal_n;
	const D3DXVECTOR3 & c0 = (collider0 ? collider0->Centre : object0->GetPosition());
	const D3DXVECTOR3 & c1 = (collider1 ? collider1->Centre : object1->GetPosition());
	normal = (c0 - c1);
	D3DXVec3Normalize(&normal_n, &normal);
	//D3DXVec3Normalize(&normal, &normal);//PHYSDBG

	// Determine hit point on the surface of each object 
	D3DXVECTOR3 hit0, hit1;
	if (collider0)			  hit0 = ClosestPointOnOBB(*collider0, c1);
	else					  hit0 = (c0 - (normal_n * object0->GetCollisionSphereRadius()));
	if (collider1)			  hit1 = ClosestPointOnOBB(*collider1, c0);
	else					  hit1 = (c1 + (normal_n * object1->GetCollisionSphereRadius()));

	// Get vectors from object centres to their hitpoints
	D3DXVECTOR3 r0 = (hit0 - c0);
	D3DXVECTOR3 r1 = (hit1 - c1);

	// Store mass & inverse mass reference parameters for convenience
	const float & mass0 = object0->GetMass();
	const float & mass1 = object1->GetMass();
	const float & invMass0 = object0->GetInverseMass();
	const float & invMass1 = object1->GetInverseMass();

	// Cross the angular velocity of each object with its hitpoint contact vector
	D3DXVECTOR3 angR0, angR1;
	D3DXVec3Cross(&angR0, &object0->PhysicsState.AngularVelocity, &r0);
	D3DXVec3Cross(&angR1, &object1->PhysicsState.AngularVelocity, &r1);

	// Determine the component of the relative object velocity that is along the normal vector
	D3DXVECTOR3 v0 = (object0->PhysicsState.WorldMomentum /* * mass0 */) + angR0;	// PHYSDBG
	D3DXVECTOR3 v1 = (object1->PhysicsState.WorldMomentum /* * mass1 */) + angR1;	// PHYSDBG
	D3DXVECTOR3 vrel = (v0 - v1);
	float vn = D3DXVec3Dot(&vrel, &normal);

	// If the objects are moving away from each other then there is no collision response required
	if (-vn < 0.01f) return;

	// Transform the inertia tensor for each object into world space
	D3DXMATRIX worldInvI0, worldInvI1;
	D3DXMatrixMultiply(&worldInvI0, object0->GetOrientationMatrix(), &(object0->PhysicsState.InverseInertiaTensor));
	D3DXMatrixMultiply(&worldInvI1, object1->GetOrientationMatrix(), &(object1->PhysicsState.InverseInertiaTensor));

	// Derive the impulse 'jn' that should be applied along the contact normal, incorporating both linear and angular momentum
	D3DXVECTOR3 Crn0, Crn1, ICrn0, ICrn1, CICrn0, CICrn1;
	D3DXVec3Cross(&Crn0, &r0, &normal);
	D3DXVec3Cross(&Crn1, &r1, &normal);
	D3DXVec3TransformCoord(&ICrn0, &Crn0, &worldInvI0);
	D3DXVec3TransformCoord(&ICrn1, &Crn1, &worldInvI1);
	D3DXVec3Cross(&CICrn0, &ICrn0, &r0);
	D3DXVec3Cross(&CICrn1, &ICrn1, &r1);

	float jn = (-1.0f * (1.0f + /*Game::C_COLLISION_SPACE_COEFF_ELASTICITY*/2.0f) * vn)	// PHYSDBG
		/
		(D3DXVec3Dot(&normal, &normal) * ((invMass0 + invMass1) +
		D3DXVec3Dot(&normal, &(CICrn0 + CICrn1))));

	// Calculate change in angular velocity
	D3DXVECTOR3 angInc0, angInc1;
	D3DXVec3Cross(&angInc0, &r0, &(normal * jn));
	D3DXVec3Cross(&angInc1, &r1, &(normal * jn));
	D3DXVec3TransformCoord(&angInc0, &angInc0, &worldInvI0);
	D3DXVec3TransformCoord(&angInc1, &angInc1, &worldInvI1);

	// Apply change in linear and angular velocity to each object
	object0->PhysicsState.WorldMomentum += (normal * jn * invMass0);
	object0->PhysicsState.AngularVelocity += angInc0;
	object1->PhysicsState.WorldMomentum -= (normal * jn * invMass1);
	object1->PhysicsState.AngularVelocity -= angInc1;


	// Log details of the collision to the debug output, if the relevant compiler flag is set
#	ifdef RJ_LOG_COLLISION_DETAILS 
	OutputDebugString("\n\n*** Collision = { \n");
	OutputDebugString(concat("\t Object0 = \"")(object0->GetName())("\" [")((int)object0->GetCollisionMode())("], Object1 = \"")
		(object1->GetName())("\" [")((int)object1->GetCollisionMode())("] \n").str().c_str());
	OutputDebugString(concat("\t collision normal = [")(normal.x)(",")(normal.y)(",")(normal.z)("]\n").str().c_str());
	OutputDebugString(concat("\t v0 = (object0 momentum [")(object0->PhysicsState.WorldMomentum.x)(",")(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)
		("] * mass [")(mass0)("]) + angvel at contact vector [")(angR0.x)(",")(angR0.y)(",")(angR0.z)("] = [")(v0.x)(",")(v0.y)(",")(v0.z)("]\n").str().c_str());
	OutputDebugString(concat("\t v1 = (object1 momentum [")(object1->PhysicsState.WorldMomentum.x)(",")(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)
		("] * mass [")(mass1)("]) + angvel at contact vector [")(angR1.x)(",")(angR1.y)(",")(angR1.z)("] = [")(v1.x)(",")(v1.y)(",")(v1.z)("]\n").str().c_str());
	OutputDebugString(concat("\t vrel = (v0 [")(v0.x)(",")(v0.y)(",")(v0.z)("] - v1 [")(v1.x)(",")(v1.y)(",")(v1.z)("]) = [")(vrel.x)(",")(vrel.y)(",")(vrel.z)("]\n").str().c_str());
	OutputDebugString(concat("\t vn = dot(vrel, normal) = ")(vn)(" (+ve value means objects are diverging and there is no collision)\n").str().c_str());
	OutputDebugString(concat("\t Object0 pre-collision state: world momentum = [")(object0->PhysicsState.WorldMomentum.x)(",")
		(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object0->PhysicsState.AngularVelocity.x)
		(",")(object0->PhysicsState.AngularVelocity.y)(",")(object0->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object1 pre-collision state: world momentum = [")(object1->PhysicsState.WorldMomentum.x)(",")
		(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object1->PhysicsState.AngularVelocity.x)
		(",")(object1->PhysicsState.AngularVelocity.y)(",")(object1->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	OutputDebugString(concat("\t jn = symmetric impulse along collision normal = ")(jn)("\n").str().c_str());
	OutputDebugString(concat("\t AngInc0 = change in normal angular velocity for object0 = [")(angInc0.x)(",")(angInc0.y)(",")(angInc0.z)("]\n").str().c_str());
	OutputDebugString(concat("\t AngInc1 = change in normal angular velocity for object1 = [")(angInc1.x)(",")(angInc1.y)(",")(angInc1.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object0 post-normal collision state: world momentum = [")(object0->PhysicsState.WorldMomentum.x)(",")
		(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object0->PhysicsState.AngularVelocity.x)
		(",")(object0->PhysicsState.AngularVelocity.y)(",")(object0->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object1 post-normal collision state: world momentum = [")(object1->PhysicsState.WorldMomentum.x)(",")
		(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object1->PhysicsState.AngularVelocity.x)
		(",")(object1->PhysicsState.AngularVelocity.y)(",")(object1->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
#	endif



	/* Now apply tangent component to simulate friction at the contact point */

	// Determine tangent vector, perpendicular to the collision normal
	D3DXVECTOR3 tangent = vrel - (D3DXVec3Dot(&vrel, &normal) * normal);
	D3DXVec3Normalize(&tangent, &tangent);

	// Safety feature: if objects somehow become merged together (e.g. spawned inside each other) the derived tangent can exceed -1e38 and leave
	// us with an undefined float error.  In such cases, we will apply a default tangent to prevent overflow
	// Use the fact that (x == x) will trivially return true for all finite numbers, but where x = -1.#IND000, (-1.#IND000 != -1.#IND000)
	// TODO: this isn't perfect, and prevents some collisions registering at shallow impact angles.  Issue with calc?  Or need double precision here?
	if (!(tangent.x == tangent.x)) tangent = D3DXVECTOR3(1.0f, 0.0f, 0.0f);

	// Calculate intermediate cross products 
	D3DXVECTOR3 Cr0tan, Cr1tan, transformedCr0tan, transformedCr1tan, Ctransr0, Ctransr1;
	D3DXVec3Cross(&Cr0tan, &r0, &tangent);
	D3DXVec3Cross(&Cr1tan, &r1, &tangent);
	D3DXVec3TransformCoord(&transformedCr0tan, &Cr0tan, &worldInvI0);
	D3DXVec3TransformCoord(&transformedCr1tan, &Cr1tan, &worldInvI1);
	D3DXVec3Cross(&Ctransr0, &transformedCr0tan, &r0);
	D3DXVec3Cross(&Ctransr1, &transformedCr1tan, &r1);

	// Now determine the tangential impulse 'jt'
	float jt = (-1.0f * D3DXVec3Dot(&vrel, &tangent))
		/
		(invMass0 + invMass1 + D3DXVec3Dot(&tangent, &(Ctransr0 + Ctransr1)));

	// Calculate change in angular velocity
	D3DXVECTOR3 tjt = (tangent * jt);
	D3DXVec3Cross(&angInc0, &r0, &tjt);
	D3DXVec3Cross(&angInc1, &r1, &tjt);
	D3DXVec3TransformCoord(&angInc0, &angInc0, &worldInvI0);
	D3DXVec3TransformCoord(&angInc1, &angInc1, &worldInvI1);

	// Apply the change in linear and angular impulse to each object
	object0->PhysicsState.WorldMomentum += (tangent * jt * invMass0);
	object0->PhysicsState.AngularVelocity += angInc0;
	object1->PhysicsState.WorldMomentum -= (tangent * jt * invMass1);
	object1->PhysicsState.AngularVelocity -= angInc1;

#	ifdef RJ_LOG_COLLISION_DETAILS
	OutputDebugString(concat("\n\t Normalised tangent vector = [")(tangent.x)(",")(tangent.y)(",")(tangent.z)("]\n").str().c_str());
	OutputDebugString(concat("\t jt = symmetric impulse along collision tangent = ")(jt)("\n").str().c_str());
	OutputDebugString(concat("\t AngInc0 = change in tangential angular velocity for object0 = [")(angInc0.x)(",")(angInc0.y)(",")(angInc0.z)("]\n").str().c_str());
	OutputDebugString(concat("\t AngInc1 = change in tangential angular velocity for object1 = [")(angInc1.x)(",")(angInc1.y)(",")(angInc1.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object0 post-tangential collision state: world momentum = [")(object0->PhysicsState.WorldMomentum.x)(",")
		(object0->PhysicsState.WorldMomentum.y)(",")(object0->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object0->PhysicsState.AngularVelocity.x)
		(",")(object0->PhysicsState.AngularVelocity.y)(",")(object0->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	OutputDebugString(concat("\t Object1 post-tangential collision state: world momentum = [")(object1->PhysicsState.WorldMomentum.x)(",")
		(object1->PhysicsState.WorldMomentum.y)(",")(object1->PhysicsState.WorldMomentum.z)("], angular velocity = [")(object1->PhysicsState.AngularVelocity.x)
		(",")(object1->PhysicsState.AngularVelocity.y)(",")(object1->PhysicsState.AngularVelocity.z)("]\n").str().c_str());
	if (object0->PhysicsState.WorldMomentum.x > Game::C_MAX_LINEAR_VELOCITY || object0->PhysicsState.WorldMomentum.y > Game::C_MAX_LINEAR_VELOCITY ||
		object0->PhysicsState.WorldMomentum.z > Game::C_MAX_LINEAR_VELOCITY || object0->PhysicsState.AngularVelocity.x > Game::C_MAX_ANGULAR_VELOCITY ||
		object0->PhysicsState.AngularVelocity.y > Game::C_MAX_ANGULAR_VELOCITY || object0->PhysicsState.AngularVelocity.z > Game::C_MAX_ANGULAR_VELOCITY)
		OutputDebugString("\t Object0 exceeds physical limits on linear and/or angular velocity; state will be scaled within limits\n");
	if (object1->PhysicsState.WorldMomentum.x > Game::C_MAX_LINEAR_VELOCITY || object1->PhysicsState.WorldMomentum.y > Game::C_MAX_LINEAR_VELOCITY ||
		object1->PhysicsState.WorldMomentum.z > Game::C_MAX_LINEAR_VELOCITY || object1->PhysicsState.AngularVelocity.x > Game::C_MAX_ANGULAR_VELOCITY ||
		object1->PhysicsState.AngularVelocity.y > Game::C_MAX_ANGULAR_VELOCITY || object1->PhysicsState.AngularVelocity.z > Game::C_MAX_ANGULAR_VELOCITY)
		OutputDebugString("\t Object1 exceeds physical limits on linear and/or angular velocity; state will be scaled within limits\n");
	OutputDebugString("}\n\n");
#	endif

	// Scale all resulting object forces to be within allowable linear/angular velocity ranges
	ScaleVectorWithinMagnitudeLimit(object0->PhysicsState.WorldMomentum, Game::C_MAX_LINEAR_VELOCITY);
	ScaleVectorWithinMagnitudeLimit(object1->PhysicsState.WorldMomentum, Game::C_MAX_LINEAR_VELOCITY);
	ScaleVectorWithinMagnitudeLimit(object0->PhysicsState.AngularVelocity, Game::C_MAX_ANGULAR_VELOCITY);
	ScaleVectorWithinMagnitudeLimit(object1->PhysicsState.AngularVelocity, Game::C_MAX_ANGULAR_VELOCITY);

	// We have made multiple changes to the world momentum of both objects; recalculate the resulting local momentum for each now
	object0->RecalculateLocalMomentum();
	object1->RecalculateLocalMomentum();

	// Determine the impact force on each object by comparing pre- & post-collision momentum
	ObjectImpact.Object.PreImpactVelocity = obj0_pre_wm;
	ObjectImpact.Collider.PreImpactVelocity = obj1_pre_wm;
	ObjectImpact.Object.VelocityChange = (object0->PhysicsState.WorldMomentum - obj0_pre_wm);
	ObjectImpact.Collider.VelocityChange = (object1->PhysicsState.WorldMomentum - obj1_pre_wm);
	ObjectImpact.Object.VelocityChangeMagnitude = D3DXVec3Length(&ObjectImpact.Object.VelocityChange);
	ObjectImpact.Collider.VelocityChangeMagnitude = D3DXVec3Length(&ObjectImpact.Collider.VelocityChange);
	ObjectImpact.Object.ImpactForce = (ObjectImpact.Object.VelocityChangeMagnitude * object0->GetMass());
	ObjectImpact.Collider.ImpactForce = (ObjectImpact.Collider.VelocityChangeMagnitude * object1->GetMass());
	ObjectImpact.TotalImpactForce = (ObjectImpact.Object.ImpactForce + ObjectImpact.Collider.ImpactForce);

	// Notify object 0 of the collision
	object0->CollisionWithObject(object1, ObjectImpact);

	// Swap the definition of object & collider and then notify object1 of the impact
	std::swap(ObjectImpact.Object, ObjectImpact.Collider);
	object1->CollisionWithObject(object1, ObjectImpact);
}


#endif