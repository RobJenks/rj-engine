#pragma once

#ifndef __GamePhysicsEngineH__
#define __GamePhysicsEngineH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "GameVarsExtern.h"
#include "FastMath.h"
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


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class GamePhysicsEngine : public ALIGN16<GamePhysicsEngine>
{
public:

	// Enumeration of the types of collision detection that may be performed
	enum CollisionDetectionType { Unknown = 0, SphereVsSphere, SphereVsOBB, OBBvsOBB, ContinuousSphereVsSphere, ContinuousSphereVsOBB };

	// Struct holding data on a ray intersection test
	// Class has no special alignment requirements
	struct RayIntersectionTestResult
	{
		float tmin, tmax;				// The min and max intersection points.  Intersection took place if tmin < tmax
	} RayIntersectionResult;

	// Struct holding data on a line intersection test.  Intersection occured at (p = P1 + [k0|k1] (P2 - P1))
	// Class has no special alignment requirements
	struct LineSegmentIntersectionData
	{
		float	k1, k2;				// k1 and k2 are the two points of intersection, if an intersection occurred
		LineSegmentIntersectionData(void) : k1(0.0f), k2(0.0f) { }
	} LineSegmentIntersectionResult;

	// Struct holding results of a collision test against an OBB or OBB hierarchy
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct OBBIntersectionData : public ALIGN16<OBBIntersectionData>
	{
		OrientedBoundingBox *						OBB;
		float										IntersectionTime;
		AXMVECTOR									CollisionPoint;
	} OBBIntersectionResult;

	// Struct holding data on a SAT test
	// Class has no special alignment requirments
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
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct ContinuousCollisionTestInterimData : public ALIGN16<ContinuousCollisionTestInterimData>
	{
		AXMVECTOR wm0, wm1;						// World velocity of each object
		AXMVECTOR pos0, pos1;					// Initial position of each object, at (t=0)
		AXMVECTOR s;							// Distance between object centers at (t=0)
		AXMVECTOR v;							// Relative velocity between objects
		AXMVECTOR r;							// Combined collision radii of the two objects

		AXMVECTOR a, b, c, d;					// Results of the sphere/sphere intersection test
	};

	// Struct holding data on a continuous collision test
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct ContinuousCollisionTestResult : public ALIGN16<ContinuousCollisionTestResult>
	{
		ContinuousCollisionTestInterimData	InterimCalculations;		// Data used during the continuous collision detection algorithms, 
																		// allowing it to be efficiently shared

		float								IntersectionTime;			// The time of intersection, in the range [0 1]
		AXMVECTOR							CollisionPos0;				// Object0 centre point at the time of collision
		AXMVECTOR							CollisionPos1;				// Object1 centre point at the time of collision
		AXMVECTOR							ContactNormal;				// The contact normal from object 0 to object 1
		AXMVECTOR							NormalisedContactNormal;	// Normalised contact normal from object 0 to object 1
		AXMVECTOR							ContactPoint;				// The contact point between the two objects

		ContinuousCollisionTestResult(void) { memset(this, 0, sizeof(this)); }
	};

	// Struct holding the result of the last collision detection that was performed
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct CollisionDetectionResult : public ALIGN16<CollisionDetectionResult>
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
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct ImpactData : public ALIGN16<ImpactData>
	{
		// Class is 16-bit aligned to allow use of SIMD member variables
		__declspec(align(16))
		struct ObjectImpactData : public ALIGN16<ObjectImpactData>
		{
			AXMVECTOR		PreImpactVelocity;				// Our velocity before the impact
			AXMVECTOR		VelocityChange;					// The change in our velocity following the impact
			AXMVECTOR		VelocityChangeMagnitude;		// The magnitude of our change in velocity (VelocityChange.Length) (vectorised single value)
			AXMVECTOR		ImpactForce;					// The force with which we have been impacted (The momentum we were hit with) (vectorised single value)

			ObjectImpactData(void) { PreImpactVelocity = VelocityChange = VelocityChangeMagnitude = ImpactForce = XMVectorZero(); }
		};

		AXMVECTOR			TotalImpactForce;				// The combined closing momentum at the collision point (vectorised single value)

		ObjectImpactData	Object;							// Impact data for the current object
		ObjectImpactData	Collider;						// Impact data for the colliding object

		ImpactData(void)	{ TotalImpactForce = XMVectorZero(); }
	};

	// Struct holding data on an impact between an object and the terrain
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct TerrainImpactData : public ALIGN16<TerrainImpactData>
	{
		StaticTerrain *		Terrain;						// The terrain object that was impacted
		AXMVECTOR			ResponseVector;					// The vector direction of the collision response
		AXMVECTOR			ResponseVelocity;				// The resulting velocity along the collision response vector (vectorised single value)
		AXMVECTOR			ImpactVelocity;					// The object velocity at point of impact  (vectorised single value)
		AXMVECTOR			ImpactForce;					// The force of the impact, i.e. the change in velocity * object momentum  (vectorised single value)

		TerrainImpactData(void) { Terrain = NULL; ResponseVector = ResponseVelocity = ImpactVelocity = ImpactForce = XMVectorZero(); }
	};

	// Struct holding the internal clock state of the physics engine, which may differ from the global clock state
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct PhysicsClockData : public ALIGN16<PhysicsClockData>
	{
		float				TimeFactor;						// The clock delta (secs) for this physics cycle
		AXMVECTOR			TimeFactorV;					// The clock delta (secs) for this physics cycle (vectorised form)
		float				RemainingFrameTime;				// The remaining time (secs) in the render frame (e.g. multiple physics cycles
															// could fill one render frame; at 0.002s per render frame and 0.001s per physics cycle
															// this field would be 0.001 followed by 0.000)
		AXMVECTOR			RemainingFrameTimeV;			// The remaining time (secs) in the render frame (e.g. multiple physics cycles
															// could fill one render frame; at 0.002s per render frame and 0.001s per physics cycle
															// this field would be 0.001 followed by 0.000) (vectorised form)

		float				FrameCycleTimeLimit;			// The maximum cycle time for this iteration of the game loop.  Will be increased from 
															// the game default if FPS starts to slow, to ensure the simulation can keep up

		PhysicsClockData(void) 
		{ 
			TimeFactor = RemainingFrameTime = FrameCycleTimeLimit = 0.0f; 
			TimeFactorV = RemainingFrameTimeV = XMVectorZero();
		}
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
	void									PerformEnvironmentCollisionDetection(iSpaceObjectEnvironment *env, const FXMVECTOR location, float radius);

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
	bool									TestSpherevsOBBHierarchyCollision(	const FXMVECTOR sphereCentre, const float sphereRadiusSq, 
																				OrientedBoundingBox & obb, OrientedBoundingBox ** ppOutOBBCollider);

	// Tests for the intersection of a bounding sphere and an oriented bounding box (OBB)
	bool									TestSpherevsOBBCollision(const FXMVECTOR sphereCentre, const float sphereRadiusSq, 
																 	 const OrientedBoundingBox::CoreOBBData & obb);

	// Tests for the intersection of a ray with a sphere.  Returns no details; only whether a collision took place
	bool									TestRaySphereIntersection(const FXMVECTOR ray_origin, const FXMVECTOR ray_dir,
																	  const FXMVECTOR sphere_centre, const GXMVECTOR sphere_radiussq) const;
	bool									TestRaySphereIntersection(const BasicRay & ray, const FXMVECTOR sphere_centre, const FXMVECTOR sphere_radiussq) const;

	// Tests for the intersection of a line segment (p1 to p2) with a sphere.  Returns no details; only whether an intersection took place
	bool									TestLineSegmentvsSphereIntersection(const FXMVECTOR p1, const FXMVECTOR p2,
																				const FXMVECTOR sphere_centre, float sphere_radius);

	// Tests for the intersection of a line vector (p1 + dp == p2) with a sphere.  Returns no details; only whether an intersection took place
	bool									TestLineVectorvsSphereIntersection(	const FXMVECTOR p1, const FXMVECTOR dp,
																				const FXMVECTOR sphere_centre, float sphere_radius);

	// Tests for the intersection of a line segment (p1 to p2) with a sphere.  Returns intersection points within 
	// the LineSegmentIntersectionResult structure
	bool									DetermineLineSegmentvsSphereIntersection(	const FXMVECTOR p1, const FXMVECTOR p2,
																						const FXMVECTOR sphere_centre, float sphere_radius);

	// Executes a raycast amongst the given collection of objects and returns a reference to the closest object that was hit.  No spatial
	// partitioning performed; assumed that the object collection will be constructed reasonably intelligently.
	template <typename T>
	T *										PerformRaycast(const BasicRay & ray, const std::vector<T*> & objects) const;

	// Tests for the intersection of a ray with an AABB.  Results will not be returned/stored, only a flag indicating whether 
	// the intersection took place.
	/*bool									TestRayVsAABBIntersection(const Ray & ray, const AABB & aabb, float t);
	CMPINLINE bool							TestRayVsAABBIntersection(const Ray & ray, const AABB & aabb)
	{
		return TestRayVsAABBIntersection(ray, aabb, 1.0f);
	}*/

	// Tests for the intersection of a ray with an AABB.  Results will be populated with min/max intersection points if an intersection
	// took place.  If min<max then we have an intersection.  Returns a flag indicating whether the intersection took place.  If 
	// min<0 then the ray began inside the AABB
	bool									DetermineRayVsAABBIntersection(const Ray & ray, const AABB & aabb, float t);
	CMPINLINE bool							DetermineRayVsAABBIntersection(const Ray & ray, const AABB & aabb)
	{
		return DetermineRayVsAABBIntersection(ray, aabb, 1.0f);		// By default, limit to the exact extent of the ray
	}

	// Tests for the intersection of a ray with an OBB, by transforming the ray into OBB-space so that the OBB can be treated
	// as an AABB centred on the origin and we can test via a ray-AABB comparison.  Does not return or store any results; 
	// simply returns a flag indicating whether the intersection took place
	/*bool									TestRayVsOBBIntersection(const Ray & ray, const OrientedBoundingBox::CoreOBBData & obb, float t);
	CMPINLINE bool							TestRayVsOBBIntersection(const Ray & ray, const OrientedBoundingBox::CoreOBBData & obb)
	{
		return TestRayVsOBBIntersection(ray, obb, 1.0f);		// By default, limit to the exact extent of the ray
	}*/

	// Tests for the intersection of a ray with an OBB, by transforming the ray into OBB-space so that the OBB can be treated
	// as an AABB centred on the origin and we can test via a ray-AABB comparison.  Results will be populated with min/max intersection 
	// points if an intersection took place.  If min<max then we have an intersection.  Returns a flag indicating whether the 
	// intersection took place.  If min<0 then the ray began inside the OBB
	bool									DetermineRayVsOBBIntersection(const Ray & ray, const OrientedBoundingBox::CoreOBBData & obb, float t);
	CMPINLINE bool							DetermineRayVsOBBIntersection(const Ray & ray, const OrientedBoundingBox::CoreOBBData & obb)
	{
		return DetermineRayVsOBBIntersection(ray, obb, 1.0f);		// By default, limit to the exact extent of the ray
	}

	// Tests for the intersection of a line vector with an OBB hierarchy, by treating as a ray and transforming into OBB-space so that each 
	// OBB can be treated as an AABB centred on the origin and we can test via a ray-AABB comparison.  Results will be populated with min/max intersection 
	// points if an intersection took place.  If min<max then we have an intersection.  Returns a flag indicating whether the 
	// intersection took place.  If min<0 then the ray began inside the OBB
	bool									DetermineLineVectorVsOBBHierarchyIntersection(const FXMVECTOR line_pos, const FXMVECTOR line_delta, OrientedBoundingBox & obb);

	// Tests for the (approximate) intersection between a volumetric ray and an OBB, by testing a point ray against an
	// OBB with temporarily expanded bounds.  Not a completely precise test but sufficient for most purposes.  ray_point_volume
	// indicates the expansion of OBB bounds; use a replicated "radius" vector if we just want to simulate a ray with certain radius 
	bool									TestVolumetricRayVsOBBIntersection(	const Ray & ray, const FXMVECTOR ray_point_volume,
																				const OrientedBoundingBox::CoreOBBData & obb, float t);
	CMPINLINE bool							TestVolumetricRayVsOBBIntersection(	const Ray & ray, const FXMVECTOR ray_point_volume,
																				const OrientedBoundingBox::CoreOBBData & obb)
	{
		return TestVolumetricRayVsOBBIntersection(ray, ray_point_volume, obb, 1.0f);
	}

	// Determine the point of collision between a ray and an OBB hierarchy

	// Perform a continuous collision test between two moving objects.  Populates the collision result data with details on the collision
	// point and time t = [0 1] at which the collision occured, if at all.  Returns a flag indicating whether a collision took place.
	bool									TestContinuousSphereCollision(const iActiveObject *object1, const iActiveObject *object2);

	// Perform a continuous collision test between a moving sphere and a static OBB.  Populates the collision result data.  Returns
	// a flag indicating whether a collision took place
	bool									TestContinuousSphereVsOBBCollision(const iActiveObject *sphere, const iActiveObject *obb);
	
	// Determines the closest point on a line segment to the specified point
	XMVECTOR								ClosestPointOnLineSegment(	const FXMVECTOR line_ep1, const FXMVECTOR line_ep2, 
																		const FXMVECTOR point );

	// Determines the closest point on an AABB to the specified point
	XMVECTOR								ClosestPointOnAABB(	const FXMVECTOR AABB_min, const FXMVECTOR AABB_max, 
																const FXMVECTOR point );

	// Determines the closest point on an OBB to the specified point
	XMVECTOR								ClosestPointOnOBB(const OrientedBoundingBox::CoreOBBData & obb, const FXMVECTOR point);

	// Determines the closest point on an OBB to the specified location.  Also returns an output parameter that indicates how close the point
	// is to the OBB centre in each of the OBB's basis axes.  Any distance < the extent in that axis means the point is inside the OBB.
	XMVECTOR								ClosestPointOnOBB(const OrientedBoundingBox::CoreOBBData & obb, const FXMVECTOR point, XMVECTOR & outDistance);

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

	// Static method that returns the most appropriate bounding volume type for the given object, based on e.g. size & dimension ratios
	 static Game::BoundingVolumeType		DetermineBestBoundingVolumeTypeForObject(const iObject *object);


protected:

	// Checks for a broadphase collision between the two objects.  No parameter checking since this should only be called internally on pre-validated parameters
	CMPINLINE bool							CheckBroadphaseCollision(const iObject *obj0, const iObject *obj1);
	CMPINLINE bool							CheckBroadphaseCollision(const FXMVECTOR pos0, float collisionradius0, const FXMVECTOR pos1, float collisionradius1);

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
	AXMVECTOR								_diffpos;
	float									_distsq, _r1r2;
	OrientedBoundingBox::CoreOBBData		_obbdata;
	std::vector<OrientedBoundingBox*>		_obb_vector;
};

// Executes a raycast amongst the given collection of objects and returns a reference to the closest object that was hit.  No spatial
// partitioning performed; assumed that the object collection will be constructed reasonably intelligently.
template <typename T>
T * GamePhysicsEngine::PerformRaycast(const BasicRay & ray, const std::vector<T*> & objects) const
{
	// We will keep track of the closest object that was intersected
	T *closest = NULL;
	XMVECTOR diff, dsq, a0, a1;
	XMVECTOR closest_dsq = XMVectorReplicate(FLT_MAX);
	static const AXMVECTOR nullvec = XMVectorZero();

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
		diff = XMVectorSubtract(ray.Origin, sphere_centre);
		dsq = XMVector3LengthSq(diff);
		if (XMVector2GreaterOrEqual(dsq, closest_dsq)) continue;							// Early-exit if the object is further away than our current best
		a0 = XMVectorSubtract(dsq, XMVectorReplicate(obj->GetCollisionSphereRadiusSq()));	// This is now equiv to "a0 = D3DXVec3Dot(&diff, &diff) - sphere_radiussq"

		// If a0 is <= 0 then the ray began inside the sphere, so we have an immediate 0-distance intersection and cannot get better than that
		if (XMVector2LessOrEqual(a0, nullvec)) return obj;

		// Project object difference vector onto the ray
		a1 = XMVector3Dot(ray.Direction, diff);												// Expanded "D3DXVec3Dot(&ray.Direction, &diff)"
		if (XMVector2GreaterOrEqual(a1, nullvec)) continue;
		
		// Intersection occurs when Q(t) has real roots.  We can avoid testing the root by instead
		// testing whether the discriminant [i.e. sqrtf(discrimininant)] is positive
		// if (((a1 * a1) - a0) >= 0.0f)
		if (XMVector2GreaterOrEqual(XMVectorSubtract(XMVectorMultiply(a1, a1), a0), nullvec))
		{
			// Test whether this intersection is closer than our current best, and record it if so
			//if (dsq < closest_dsq)
			if (XMVector2Less(dsq, closest_dsq))
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







