#pragma once

#ifndef __Order_MoveToTargetH__
#define __Order_MoveToTargetH__

#include "Order.h"

class Order_MoveToTarget : public Order
{
public:
	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::MoveToTarget; }

	// Constructor including main order parameters
	Order_MoveToTarget(iSpaceObject *target, float closedistance)
	{
		this->Parameters.Target_1 = target;
		this->Parameters.Float3_1.x = closedistance;
	}

	// Default constructor / destructor
	Order_MoveToTarget(void) { }
	~Order_MoveToTarget(void) { }
};


#endif
