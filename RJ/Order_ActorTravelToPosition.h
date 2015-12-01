#pragma once

#ifndef __Order_ActorTravelToPositionH__
#define __Order_ActorTravelToPositionH__

#include <vector>
#include "Order.h"
class iSpaceObjectEnvironment;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order_ActorTravelToPosition : public ALIGN16<Order_ActorTravelToPosition>, public Order
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(Order_ActorTravelToPosition)

	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::ActorTravelToPosition; }


	// Constructor including main order parameters
	// CloseDistance is the distance to the target that we will attempt to reach.  FollowDistance
	// is the distance we will get to each waypoint on the route
	Order_ActorTravelToPosition(iSpaceObjectEnvironment *environment, CXMVECTOR startpos, CXMVECTOR targetpos, 
								float closedistance, float followdistance, bool run);

	// Calculates the path that should be followed in order to reach the target position
	void CalculateTravelPath(void);

	// Default constructor / destructor
	~Order_ActorTravelToPosition(void) { }


public:

	// Order parameters
	AXMVECTOR						StartPosition;
	AXMVECTOR						TargetPosition;
	float							CloseDistance, CloseDistanceSq;		// Distance to the target that we will attempt to reach
	float							FollowDistance, FollowDistanceSq;	// Distance we will get within each waypoint on the route
	bool							Run;
	iSpaceObjectEnvironment *		Environment;


	// Maintain an array of positions that will be followed on the path, and an index specifying 
	// which position is the current move target
	INTVECTOR3 *					PathNodes;
	int								PathLength, PathIndex;

};



#endif