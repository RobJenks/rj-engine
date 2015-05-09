#pragma once

#ifndef __GamePhysicsEngineH__
#define __GamePhysicsEngineH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "GameVarsExtern.h"
#include "BasicRay.h"
#include "OrientedBoundingBox.h"
#include "CollisionDetectionResultsStruct.h"
class iObject;
class iActiveObject;
class iSpaceObject;
class iEnvironmentObject;
class StaticTerrain;
class iSpaceObjectEnvironment;
class AABB;
class Ray;

class GamePhysicsEngine
{
public:

	// Enumeration of the types of collision detection that may be performed
	enum CollisionDetectionType { Unknown = 0, SphereVsSphere, SphereVsOBB, OBBvsOBB, ContinuousSphereVsSphere, ContinuousSphereVsOBB };

	// Struct holding data on a ray intersection test
	struct RayIntersectionTestResult
	{
		float tmin, tmax;				// The min and max intersection points.  Intersection took place if tmin < tmax
	} RayIntersectionResult;

	// Struct holding data on a SAT test
	struct SATIntersectionResult 
	{ 
		int Object0Axis, Object1Axis;
		float AxisDist0[3], AxisDist1[3];

		SATIntersectionResult(void) 
		{ 
			Object0Axis = Object1Axis = -1;
			AxisDist0[0] = AxisDist0[1] = AxisDist0[2] = AxisDist1[0] = AxisDist1[1] = AxisDist1[2] = 0.0f;
		}
	};

	// Struct holding intermediate data used in performing a continuous collision detection
	struct ContinuousCollisionTestInterimData
	{
		D3DXVECTOR3 wm0, wm1;					// World velocity of each object
		D3DXVECTOR3 pos0, pos1;					// Initial position of each object, at (t=0)
		D3DXVECTOR3 s;							// Distance between object centers at (t=0)
		D3DXVECTOR3 v;							// Relative velocity between objects
		float r;								// Combined collision radii of the two objects

		float a, b, c, d;						// Results of the sphere/sphere intersection test
	};

	// Struct holding data on a continuous collision test
	struct ContinuousCollisionTestResult
	{
		ContinuousCollisionTestInterimData	InterimCalculations;		// Data used during the continuous collision detection algorithms, 
																		// allowing it to be efficiently shared

		float								IntersectionTime;			// The time of intersection, in the range [0 1]
		D3DXVECTOR3							CollisionPos0;				// Object0 centre point at the time of collision
		D3DXVECTOR3							CollisionPos1;				// Object1 centre point at the time of collision
		D3DXVECTOR3							ContactNormal;				// The contact normal from object 0 to object 1
		D3DXVECTOR3							NormalisedContactNormal;	// Normalised contact normal from object 0 to object 1
		D3DXVECTOR3							ContactPoint;				// The contact point between the two objects

		ContinuousCollisionTestResult(void) { memset(this, 0, sizeof(this)); }
	};

	// Struct holding the result of the last collision detection that was performed
	struct CollisionDetectionResult
	{
		CollisionDetectionType			Type;							// The collision detection method that was used
		float							BroadphasePenetrationSq;		// The squared penetration distance measured during broadphase testing
		float							Penetration;					// The penetration amount between the two objects

		SATIntersectionResult			SATResult;						// The results of a SAT intersection test, if performed
		ContinuousCollisionTestResult	ContinuousTestResult;			// The results of a continuous collision test, if performed

		// Default constructor
		CollisionDetectionResult(void) { Type = CollisionDetectionType::Unknown; BroadphasePenetrationSq = Penetration = 0.0f; }

		// Return the penetration for any collision type; for SphereVsSphere this will be the sqrt of the BroadphasePenetrationSq.
		// For any other method this will simply be the Penetration value that was already calculated
		CMPINLINE float			DeterminePenetration(void) const 
		{ 
			return (Type == CollisionDetectionType::SphereVsSphere ? sqrtf(BroadphasePenetrationSq) : Penetration);
		}
	};

	// Struct holding data on an impact between two objects
	struct ImpactData
	{
		struct ObjectImpactData
		{
			D3DXVECTOR3		PreImpactVelocity;				// Our velocity before the impact
			D3DXVECTOR3		VelocityChange;					// The change in our velocity following the impact
			float			VelocityChangeMagnitude;		// The magnitude of our change in velocity (VelocityChange.Length)
			float			ImpactForce;					// The force with which we have been impacted (The momentum we were hit with)

			ObjectImpactData(void) { PreImpactVelocity = VelocityChange = D3DXVECTOR3(0.0f, 0.0f, 0.0f); VelocityChangeMagnitude = ImpactForce = 0.0f; }
		};

		float				TotalImpactForce;				// The combined closing momentum at the collision point

		ObjectImpactData	Object;							// Impact data for the current object
		ObjectImpactData	Collider;						// Impact data for the colliding object

		ImpactData(void)	{ TotalImpactForce = 0.0f; }
	};

	// Struct holding data on an impact between an object and the terrain
	struct TerrainImpactData
	{
		StaticTerrain *		Terrain;						// The terrain object that was impacted
		D3DXVECTOR3			ResponseVector;					// The vector direction of the collision response
		float				ResponseVelocity;				// The resulting velocity along the collision response vector
		float				ImpactVelocity;					// The object velocity at point of impact
		float				ImpactForce;					// The force of the impact, i.e. the change in velocity * object momentum

		TerrainImpactData(void) { Terrain = NULL; ResponseVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f); ResponseVelocity = ImpactVelocity = ImpactForce = 0.0f; }
	};

	// Struct holding the internal clock state of the physics engine, which may differ from the global clock state
	struct PhysicsClockData
	{
		float				TimeFactor;						// The clock delta (secs) for this physics cycle
		float				RemainingFrameTime;				// The remaining time (secs) in the render frame (e.g. multiple physics cycles
															// could fill one render frame; at 0.002s per render frame and 0.001s per physics cycle
															// this field would be 0.001 followed by 0.000)
		float				FrameCycleTimeLimit;			// The maximum cycle time for this iteration of the game loop.  Will be increased from 
															// the game default if FPS starts to slow, to ensure the simulation can keep up

		PhysicsClockData(void) { TimeFactor = RemainingFrameTime = FrameCycleTimeLimit = 0.0f; }
	};

	// Default constructor
	GamePhysicsEngine(void);

	// The physics engine maintains its own internal clock, since physics cycles may not necessarily be in step with the main game loop cycles
	PhysicsClockData						PhysicsClock;

	// Maintain a record of all collision detection activity that took place each frame
	CollisionDetectionResultsStruct			CollisionDetectionResults;

	// Primary method to simulate all physics in the world.  Uses semi-fixed time step to maintain reasonably frame-independent simulation 
	// results, with a simulation limiter to avoid the 'spiral of death' 
	void									SimulatePhysics(void); 

	// Runs a full cycle of physics simulation.  Timing data for this cycle is held within the physics engine internal clock
	// Physics cycle may differ in length from the full game cycles, if e.g. the physics engine fidelity is very high or if the
	// render FPS gets too low
	void									PerformPhysicsCycle(void);

	// Performs collision detection about the specified object.  Determines parameters for e.g. how far around the object we should
	// be testing for collisions, and then passes control to the main collision detection method
	void									PerformCollisionDetection(iObject *focalobject);

	// Performs a full cycle of collision detection & collision response in a radius around the specified focal object (which is typically
	// the player).  Use the object's octree to locate and test objects.  If radius < 0.0f then all objects in the tree will be considered 
	// (which can be very inefficient).  We use a focal object rather than simply a position since the objects are automatically maintaining
	// pointers to the relevant octree nodes during simulation.  Using a position value we would have to calculate the relevant node each frame.
	// This method is specific to space-based collision handling
	void									PerformSpaceCollisionDetection(iSpaceObject *focalobject, float radius);

	// Performs continuous collision detection (CCD) for the specified object, including potentially handling multiple collisions
	// within the same execution cycle and rollback of physics time to simulate high-speed within-frame collisions.  Returns the 
	// object which we collided with, if applicable, otherwise NULL
	iSpaceObject *							PerformContinuousSpaceCollisionDetection(iSpaceObject *object);

	// Performs a full cycle of collision detection & collision response in a radius around the specified focal location (which is typically
	// the player) in the specified environment.  Use the existing environment structure to partition & identify potential colliding pairs.  
	// If radius < 0.0f then all objects in the environment will be considered (which can be inefficient).  This method is specific to 
	// environment-based collision handling
	void									PerformEnvironmentCollisionDetection(iSpaceObjectEnvironment *env, const D3DXVECTOR3 & location, float radius);

	// Performs collision detection for all objects in the specified element of an environment
	void									PerformEnvironmentCollisionDetection(iSpaceObjectEnvironment *env, int x, int y, int z);

	// Performs collision detection for the specified environment object with its surroundings
	void									PerformEnvironmentCollisionDetection(iEnvironmentObject *obj);

	// Performs hierarchical collision detection between two OBB hierarchies, returning the two OBBs that collided (if applicable)
	bool									TestOBBvsOBBHierarchy(	OrientedBoundingBox & obj0, OrientedBoundingBox & obj1, 
																	OrientedBoundingBox ** ppOutCollider0, OrientedBoundingBox ** ppOutCollider1);

	// Tests for the intersection of two oriented bounding boxes (OBB)
	bool									TestOBBvsOBBCollision(const OrientedBoundingBox::CoreOBBData & box0, const OrientedBoundingBox::CoreOBBData & box1);

	// Returns the result of the last positive space collision test. 'Penetration' will represent either the degree of penetration 
	// (in case of collision) or separation (if not).  'Penetration' & "BroadphasePenetrationSq' will be set in 
	// all cases except where Type == SphereVsSphere, in which case only the 'BroadphasePenetrationSq' value will be populated
	CMPINLINE const CollisionDetectionResult & LastCollisionTest(void) const				{ return m_collisiontest; }

	// Tests for the intersection of a bounding sphere with an OBB collision hierarchy 
	bool									TestSpherevsOBBHierarchyCollision(	const D3DXVECTOR3 & sphereCentre, const float sphereRadiusSq, 
																				OrientedBoundingBox & obb, OrientedBoundingBox ** ppOutOBBCollider);

	// Tests for the intersection of a bounding sphere and an oriented bounding box (OBB)
	bool									TestSpherevsOBBCollision(const D3DXVECTOR3 & sphereCentre, const float sphereRadiusSq, 
																 	 const OrientedBoundingBox::CoreOBBData & obb);

	// Tests for the intersection of a ray with a sphere.  Returns no details; only whether a collision took place
	CMPINLINE bool							TestRaySphereIntersection(const D3DXVECTOR3 & ray_origin, const D3DXVECTOR3 & ray_dir,
																	  const D3DXVECTOR3 & sphere_centre, float sphere_radiussq) const;
	CMPINLINE bool							TestRaySphereIntersection(const BasicRay & ray, const D3DXVECTOR3 & sphere_centre, float sphere_radiussq) const;


	// Executes a raycast amongst the given collection of objects and returns a reference to the closest object that was hit.  No spatial
	// partitioning performed; assumed that the object collection will be constructed reasonably intelligently.
	template <typename T>
	T *										PerformRaycast(const BasicRay & ray, const std::vector<T*> & objects) const;

	// Tests for the intersection of a ray with an AABB.  Results will be populated with min/max intersection points if an intersection
	// took place.  If min<max then we have an intersection.  Returns a flag indicating whether the intersection took place.  If 
	// min<0 then the ray began inside the AABB.  
	bool									TestRayVsAABBIntersection(const Ray & ray, const AABB & aabb, float t);
	CMPINLINE bool							TestRayVsAABBIntersection(const Ray & ray, const AABB & aabb)
	{
		return TestRayVsAABBIntersection(ray, aabb, 1.0f);		// By default, limit to the exact extent of the ray
	}

	// Perform a continuous collision test between two moving objects.  Populates the collision result data with details on the collision
	// point and time t = [0 1] at which the collision occured, if at all.  Returns a flag indicating whether a collision took place.
	bool									TestContinuousSphereCollision(const iActiveObject *object1, const iActiveObject *object2);

	// Perform a continuous collision test between a moving sphere and a static OBB.  Populates the collision result data.  Returns
	// a flag indicating whether a collision took place
	bool									TestContinuousSphereVsOBBCollision(const iActiveObject *sphere, const iActiveObject *obb);
	
	// Determines the closest point on a line segment to the specified point
	D3DXVECTOR3								ClosestPointOnLineSegment(	const D3DXVECTOR3 & line_ep1, const D3DXVECTOR3 & line_ep2, 
																		const D3DXVECTOR3 & point );

	// Determines the closest point on an AABB to the specified point
	D3DXVECTOR3								ClosestPointOnAABB(	const D3DXVECTOR3 & AABB_min, const D3DXVECTOR3 & AABB_max, 
																const D3DXVECTOR3 & point );

	// Determines the closest point on an OBB to the specified point
	D3DXVECTOR3								ClosestPointOnOBB(const OrientedBoundingBox::CoreOBBData & obb, const D3DXVECTOR3 & point);

	// Determines the closest point on an OBB to the specified location.  Also returns an output parameter that indicates how close the point
	// is to the OBB centre in each of the OBB's basis axes.  Any distance < the extent in that axis means the point is inside the OBB.
	D3DXVECTOR3								ClosestPointOnOBB(const OrientedBoundingBox::CoreOBBData & obb, const D3DXVECTOR3 & point, D3DXVECTOR3 & outDistance);

	// Struct holding data on an impact between two objects
	ImpactData								ObjectImpact;

	// Struct holding data on a significant impact between an object and the terrain
	TerrainImpactData						TerrainImpact;

	// Returns the distance that should be tested around an object for CCD contacts.  No parameter checking for efficiency
	float									GetCCDTestDistance(const iActiveObject *object) const;

	// Set or test the flag that indicates whether the engine will still handle collisions between diverging objects.  Default: no
	CMPINLINE bool							TestFlag_HandleDivergingCollisions(void) const			{ return m_flag_handle_diverging_collisions; }
	CMPINLINE void							SetFlag_HandleDivergingCollisions(void)					{ m_flag_handle_diverging_collisions = true; }
	CMPINLINE void							ClearFlag_HandleDivergingCollisions(void)				{ m_flag_handle_diverging_collisions = false; }
	CMPINLINE void							SetFlagValue_HandleDivergingCollisions(bool b)			{ m_flag_handle_diverging_collisions = b; }
	
	// Default destructor
	~GamePhysicsEngine(void);

protected:

	// Checks for a broadphase collision between the two objects.  No parameter checking since this should only be called internally on pre-validated parameters
	CMPINLINE bool							CheckBroadphaseCollision(const iObject *obj0, const iObject *obj1);
	CMPINLINE bool							CheckBroadphaseCollision(const D3DXVECTOR3 & pos0, float collisionradius0, const D3DXVECTOR3 & pos1, float collisionradius1);

	// Performs full collision detection between the two objects.  No parameter checking since this should only be called internally on pre-validated parameters
	bool									CheckFullCollision(iObject *obj0, iObject *obj1, OrientedBoundingBox ** ppOutCollider0, OrientedBoundingBox ** ppOutCollider1);

	// Determines collision response between two objects that we have determined are colliding.  Collider0/1 are pointers to
	// the specific OBB within each object that is colliding; this can be NULL, in which case we consider the object as a 
	// whole.  Called from main PerformCollisionDetection() method.
	void									HandleCollision(iActiveObject *object0, iActiveObject *object1,
															const OrientedBoundingBox::CoreOBBData *collider0, 
															const OrientedBoundingBox::CoreOBBData *collider1);

	// Determines collision response between two environment objects that we have determined are colliding.  Collider0/1 are pointers to
	// the specific OBB within each object that is colliding; this can be NULL, in which case we consider the object as a 
	// whole.  Called from main PerformCollisionDetection() method.  NOTE: this could also handle iActiveObject if we want to generalise one level
	/*void									HandleEnvironmentCollision(	iEnvironmentObject *object0, iEnvironmentObject *object1,
																		const OrientedBoundingBox::CoreOBBData *collider0,
																		const OrientedBoundingBox::CoreOBBData *collider1);

	// Determines collision response between an active object and the terrain.  Implements some simplifying assumptions (for now) such as the
	// fact that the active object will always be small enough to approximate the contact point as its origin.  The collision response is always
	// an equal and opposite response to the active object's momentum along the contact normal, resulting in the active object stopping
	// immediately, and the terrain object remaining immobile.
	void									HandleTerrainCollision(iEnvironmentObject *object, StaticTerrain *terrain);
	*/
	// Performs full SAT collision testing between the object and terrain OBBs.  If a collision is detected, applies an appropriate response
	// to move the object out of the terrain collision box by the minimum separating axis.  Returns a value indicating whether any
	// collisions were detected
public:
	bool									TestAndHandleTerrainCollision(	iSpaceObjectEnvironment *env, iEnvironmentObject *object,
																			StaticTerrain *terrain );
protected:

	// Store the last collision detection result in a structure that can be accessed by collision handling/response functions
	CollisionDetectionResult				m_collisiontest;

	// Flag that can be set to have the collision engine handle collisions between objects which are diverging.  Not typical behaviour, 
	// but can be set during CCD where object trajectories may not be converging at every point along the CCD sweep
	bool									m_flag_handle_diverging_collisions;

	// Counter & flag that determine whether collision detection should evaluate pairs of static objects this cycle.  In the majority of cases
	// we can exclude these since static objects will very rarely be colliding
	unsigned int							m_static_cd_counter;
	bool									m_cd_include_static;

	// Temporary variables to avoid multiple reallocations per physics cycle
	D3DXVECTOR3								_diffpos;
	float									_distsq, _r1r2;
	OrientedBoundingBox::CoreOBBData		_obbdata;
};


CMPINLINE bool GamePhysicsEngine::TestRaySphereIntersection(const D3DXVECTOR3 & ray_origin, const D3DXVECTOR3 & ray_dir,
															const D3DXVECTOR3 & sphere_centre, float sphere_radiussq) const
{
	// The sphere is (X-C)^T*(X-C)-1 = 0 and the line is X = P+t*D. Substitute the line equation into the sphere 
	// equation to obtain a quadratic equation Q(t) = t^2 + 2*a1*t + a0 = 0, where a1 = D^T*(P-C), and a0 = (P-C)^T*(P-C)-1.
	D3DXVECTOR3 diff = (ray_origin - sphere_centre);
	float a0 = ((diff.x*diff.x) + (diff.y*diff.y) + (diff.z*diff.z)) - sphere_radiussq;		// Expanded "D3DXVec3Dot(&diff, &diff)"

	// If a0 is <= 0 then the ray began inside the sphere, so we can return true immediately
	if (a0 <= 0.0f) return true;

	// Project object difference vector onto the ray
	float a1 = ((ray_dir.x*diff.x) + (ray_dir.y*diff.y) + (ray_dir.z*diff.z));			// Expanded "D3DXVec3Dot(&ray_dir, &diff)"
	if (a1 >= 0.0f) return false;

	// Intersection occurs when Q(t) has real roots.  We can avoid testing the root by instead
	// testing whether the discriminant [i.e. sqrtf(discrimininant)] is positive
	return (((a1 * a1) - a0) >= 0.0f);
}

CMPINLINE bool GamePhysicsEngine::TestRaySphereIntersection(const BasicRay & ray, const D3DXVECTOR3 & sphere_centre, float sphere_radiussq) const
{
	// The sphere is (X-C)^T*(X-C)-1 = 0 and the line is X = P+t*D. Substitute the line equation into the sphere 
	// equation to obtain a quadratic equation Q(t) = t^2 + 2*a1*t + a0 = 0, where a1 = D^T*(P-C), and a0 = (P-C)^T*(P-C)-1.
	D3DXVECTOR3 diff = (ray.Origin - sphere_centre);
	float a0 = ((diff.x*diff.x) + (diff.y*diff.y) + (diff.z*diff.z)) - sphere_radiussq;				// Expanded "D3DXVec3Dot(&diff, &diff)"

	// If a0 is <= 0 then the ray began inside the sphere, so we can return true immediately
	if (a0 <= 0.0f) return true;

	// Project object difference vector onto the ray
	float a1 = ((ray.Direction.x*diff.x) + (ray.Direction.y*diff.y) + (ray.Direction.z*diff.z));	// Expanded "D3DXVec3Dot(&ray.Direction, &diff)"
	if (a1 >= 0.0f) return false;

	// Intersection occurs when Q(t) has real roots.  We can avoid testing the root by instead
	// testing whether the discriminant [i.e. sqrtf(discrimininant)] is positive
	return (((a1 * a1) - a0) >= 0.0f);
}

// Executes a raycast amongst the given collection of objects and returns a reference to the closest object that was hit.  No spatial
// partitioning performed; assumed that the object collection will be constructed reasonably intelligently.
template <typename T>
T * GamePhysicsEngine::PerformRaycast(const BasicRay & ray, const std::vector<T*> & objects) const
{
	// We will keep track of the closest object that was intersected
	T *closest = NULL;
	float closest_dsq = FLT_MAX;
	D3DXVECTOR3 diff; float dsq, a0, a1;

	// Loop through each object in turn
	int n = (int)objects.size();
	for (int i = 0; i < n; ++i)
	{
		// Here we will expand out the ray/sphere intersection test to avoid function calls, and to avoid duplicating
		// calculations (e.g. difference vector from ray origin to sphere) which are required here & the ray/sphere test

		// The sphere is (X-C)^T*(X-C)-1 = 0 and the line is X = P+t*D. Substitute the line equation into the sphere 
		// equation to obtain a quadratic equation Q(t) = t^2 + 2*a1*t + a0 = 0, where a1 = D^T*(P-C), and a0 = (P-C)^T*(P-C)-1.
		T *obj = objects[i];

		// Get the difference vector from ray origin to sphere centre.  We can early-exit here if it is further away 
		// than our current closest intersection
		diff = (ray.Origin - sphere_centre);
		dsq = ((diff.x*diff.x) + (diff.y*diff.y) + (diff.z*diff.z));
		if (dsq >= closest_dsq) continue;								// Early-exit if the object is further away than our current best
		a0 = dsq - obj->GetCollisionSphereRadiusSq();					// This is now equiv to "a0 = D3DXVec3Dot(&diff, &diff) - sphere_radiussq"

		// If a0 is <= 0 then the ray began inside the sphere, so we have an immediate 0-distance intersection and cannot get better than that
		if (a0 <= 0.0f) return obj;

		// Project object difference vector onto the ray
		float a1 = ((ray.Direction.x*diff.x) + (ray.Direction.y*diff.y) + (ray.Direction.z*diff.z));	// Expanded "D3DXVec3Dot(&ray.Direction, &diff)"
		if (a1 >= 0.0f) continue;

		// Intersection occurs when Q(t) has real roots.  We can avoid testing the root by instead
		// testing whether the discriminant [i.e. sqrtf(discrimininant)] is positive
		if (((a1 * a1) - a0) >= 0.0f)
		{
			// Test whether this intersection is closer than our current best, and record it if so
			if (dsq < closest_dsq)
			{
				closest = obj;
				closest_dsq = dsq;
			}
		}
	}

	// Return the closest intersected object, or NULL if no intersections were detected
	return closest;
}


#endif







