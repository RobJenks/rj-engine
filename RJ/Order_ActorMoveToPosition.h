#pragma once

#ifndef __Order_ActorMoveToPositionH__
#define __Order_ActorMoveToPositionH__

#include "Order.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order_ActorMoveToPosition : public ALIGN16<Order_ActorMoveToPosition>, public Order
{
public:
	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::ActorMoveToPosition; }

	// Constructor including main order parameters
	Order_ActorMoveToPosition(FXMVECTOR position, float getwithin, bool run)
	{
		this->Parameters.Vector_1 = position;
		this->Parameters.Float3_1.x = getwithin;
		this->Parameters.Flag_1 = run;
	}

	// Default constructor / destructor
	Order_ActorMoveToPosition(void) { }
	~Order_ActorMoveToPosition(void) { }
};



#endif