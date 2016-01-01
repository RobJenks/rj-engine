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

	// Constructor including main order parameters
	Order_MoveToPosition(CXMVECTOR position, float closedistance)
		:
		Target(position),
		CloseDistance(closedistance),
		CloseDistanceSq(closedistance * closedistance)
	{
		// All order subclasses must set their order type on construction
		m_ordertype = Order::OrderType::MoveToPosition;
	}

	// Default destructor
	~Order_MoveToPosition(void) { }

public:

	// Order parameters
	AXMVECTOR					Target;
	float						CloseDistance, CloseDistanceSq;

};


#endif
