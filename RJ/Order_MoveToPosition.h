#pragma once

#ifndef __Order_MoveToPositionH__
#define __Order_MoveToPositionH__

#include "Order.h"

class Order_MoveToPosition : public Order
{
public:
	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::MoveToPosition; }

	// Constructor including main order parameters#ifndef __
	Order_MoveToPosition(D3DXVECTOR3 position, float closedistance)
	{
		this->Parameters.Float3_1 = position;
		this->Parameters.Float3_2.x = closedistance;
	}

	// Default constructor / destructor
	Order_MoveToPosition(void) { }
	~Order_MoveToPosition(void) { }
};


#endif
