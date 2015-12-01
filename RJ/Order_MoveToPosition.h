#pragma once

#ifndef __Order_MoveToPositionH__
#define __Order_MoveToPositionH__

#include "Order.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order_MoveToPosition : public ALIGN16<Order_MoveToPosition>, public Order
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(Order_MoveToPosition)

	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::MoveToPosition; }

	// Constructor including main order parameters
	Order_MoveToPosition(CXMVECTOR position, float closedistance)
		:
		Target(position),
		CloseDistance(closedistance),
		CloseDistanceSq(closedistance * closedistance)
	{
	}

	// Default constructor / destructor
	Order_MoveToPosition(void) { }
	~Order_MoveToPosition(void) { }

public:

	// Order parameters
	AXMVECTOR					Target;
	float						CloseDistance, CloseDistanceSq;

};


#endif
