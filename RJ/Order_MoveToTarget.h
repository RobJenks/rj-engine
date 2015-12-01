#pragma once

#ifndef __Order_MoveToTargetH__
#define __Order_MoveToTargetH__

#include "Order.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order_MoveToTarget : public ALIGN16<Order_MoveToTarget>, public Order
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(Order_MoveToTarget)

	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::MoveToTarget; }

	// Constructor including main order parameters
	Order_MoveToTarget(iSpaceObject *target, float closedistance)
		:
		Target(target),
		CloseDistance(closedistance),
		CloseDistanceSq(closedistance * closedistance)
	{
	}

	// Default constructor / destructor
	Order_MoveToTarget(void) { }
	~Order_MoveToTarget(void) { }

public:

	iSpaceObject *					Target;
	float							CloseDistance, CloseDistanceSq;

};


#endif
