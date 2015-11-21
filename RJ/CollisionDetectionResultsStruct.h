#pragma once

#ifndef __CollisionDetectionResultsStructH__
#define __CollisionDetectionResultsStructH__

#include "CompilerSettings.h"

// Struct holding details on the collision detection performed per frame
// This class has no special alignment requirements
struct CollisionDetectionResultsStruct
{
public:
	
	struct
	{
		int CollisionChecks;				// The number of space object pairs tested for collision (after pruning of potential combinations)
		int BroadphaseCollisions;			// The number of broadphase collisions detected between space objects
		int Collisions;						// The number of actual collisions detected between space objects

		int CCDCollisionChecks;				// The number of space object pairs tested during continuous collision detection
		int CCDCollisions;					// The number of actual collisions detected during continuous collision detection

	} SpaceCollisions;

	struct 
	{
		int ElementsChecked;				// The number of environment elements that were checked around the focal location
		int ObjectsChecked;					// The number of environment objects that were subsequently checked for collisions
		int ElementsCheckedAroundObjects;	// The total number of elements checked around all environment objects in scope for testing
		int ObjectVsTerrainChecks;			// The number of object/terrain collision checks that were performed
		int ObjectVsObjectChecks;			// The number of object/object collision checks that were performed
		int BroadphaseCollisions;			// The number of broadphase collisions detected between environment objects
		int Collisions;						// The number of actual collisions detected between environment objects


	} EnvironmentCollisions;

	// Clear the collision data
	CMPINLINE void ClearData(void)
	{
		memset(this, 0, sizeof(CollisionDetectionResultsStruct));
	}

	// Default constructor; sets all collision result counters to zero
	CollisionDetectionResultsStruct(void) { ClearData(); }

	// Returns the total number of collisions detected
	CMPINLINE int GetTotalCollisions(void) const { return (SpaceCollisions.Collisions + EnvironmentCollisions.Collisions); }

	// Returns the total number of broadphase collisions detected
	CMPINLINE int GetTotalBroadphaseCollisions(void) const { return (SpaceCollisions.BroadphaseCollisions + EnvironmentCollisions.BroadphaseCollisions); }

	// Returns the total number of collision pairs tested this frame
	CMPINLINE int GetTotalCollisionPairsChecked(void) const { return (	SpaceCollisions.CollisionChecks + 
																		EnvironmentCollisions.ObjectVsObjectChecks + 
																		EnvironmentCollisions.ObjectVsTerrainChecks); }
};


#endif