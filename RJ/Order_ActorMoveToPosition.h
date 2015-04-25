#pragma once

#ifndef __Order_ActorMoveToPositionH__
#define __Order_ActorMoveToPositionH__

#include "Order.h"

class Order_ActorMoveToPosition : public Order
{
public:
	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::ActorMoveToPosition; }

	// Constructor including main order parameters
	Order_ActorMoveToPosition(D3DXVECTOR3 position, float getwithin, bool run)
	{
		this->Parameters.Float3_1 = position;
		this->Parameters.Float3_2.x = getwithin;
		this->Parameters.Flag_1 = run;
	}

	// Default constructor / destructor
	Order_ActorMoveToPosition(void) { }
	~Order_ActorMoveToPosition(void) { }
};



#endif