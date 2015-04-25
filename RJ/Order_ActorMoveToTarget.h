#pragma once

#ifndef __Order_ActorMoveToTargetH__
#define __Order_ActorMoveToTargetH__

#include "iEnvironmentObject.h"
#include "Order.h"

class Order_ActorMoveToTarget : public Order
{
public:
	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::ActorMoveToTarget; }

	// Constructor including main order parameters
	Order_ActorMoveToTarget(iEnvironmentObject *target, float getwithin)
	{
		this->Parameters.Target_1 = (iSpaceObject*)target;
		this->Parameters.Float3_1.x = getwithin;
	}

	// Default constructor / destructor
	Order_ActorMoveToTarget(void) { }
	~Order_ActorMoveToTarget(void) { }
};



#endif