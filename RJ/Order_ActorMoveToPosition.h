#pragma once

#ifndef __Order_ActorMoveToPositionH__
#define __Order_ActorMoveToPositionH__

#include "Order.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order_ActorMoveToPosition : public ALIGN16<Order_ActorMoveToPosition>, public Order
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(Order_ActorMoveToPosition)

	// Constructor including main order parameters
	Order_ActorMoveToPosition(FXMVECTOR position, float getwithin, bool run)
		:
		Target(position),
		CloseDistance(getwithin),
		CloseDistanceSq(getwithin * getwithin),
		Run(run)
	{
		// All order subclasses must set their order type on construction
		m_ordertype = Order::OrderType::ActorMoveToPosition;
	}

	// Default destructor
	~Order_ActorMoveToPosition(void) { }

public:

	// Order parameters
	AXMVECTOR					Target;
	float						CloseDistance, CloseDistanceSq;
	bool						Run;

};



#endif