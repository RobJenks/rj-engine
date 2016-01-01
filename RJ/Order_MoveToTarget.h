#pragma once

#ifndef __Order_MoveToTargetH__
#define __Order_MoveToTargetH__

#include "Order.h"
#include "ObjectReference.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order_MoveToTarget : public ALIGN16<Order_MoveToTarget>, public Order
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(Order_MoveToTarget)

	// Constructor including main order parameters
	Order_MoveToTarget(iSpaceObject *target, float closedistance, bool lead_target)
		:
		Target(target),
		CloseDistance(closedistance),
		CloseDistanceSq(closedistance * closedistance),
		LeadTarget(lead_target)
	{
		// All order subclasses must set their order type on construction
		m_ordertype = Order::OrderType::MoveToTarget;
	}

	// Default destructor
	~Order_MoveToTarget(void) { }

public:

	ObjectReference<iSpaceObject>	Target;
	float							CloseDistance, CloseDistanceSq;
	bool							LeadTarget;

};


#endif
