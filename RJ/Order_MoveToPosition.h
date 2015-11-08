#pragma once

#ifndef __Order_MoveToPositionH__
#define __Order_MoveToPositionH__

#include "Order.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order_MoveToPosition : public ALIGN16<Order_MoveToPosition>, public Order
{
public:
	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::MoveToPosition; }

	// Constructor including main order parameters#ifndef __
	Order_MoveToPosition(CXMVECTOR position, float closedistance)
	{
		this->Parameters.Vector_1 = position;
		this->Parameters.Float3_1.x = closedistance;
	}

	// Default constructor / destructor
	Order_MoveToPosition(void) { }
	~Order_MoveToPosition(void) { }
};


#endif
