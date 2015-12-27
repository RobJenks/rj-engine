#pragma once

#ifndef __Order_AttackBasicH__
#define __Order_AttackBasicH__

#include "Order.h"
#include "ObjectReference.h"
class iSpaceObject;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order_AttackBasic : public ALIGN16<Order_AttackBasic>, public Order
{
public:
	
	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(Order_AttackBasic)

	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::AttackBasic; }

	// Constructor including main order parameters
	Order_AttackBasic(Ship *attacker, iSpaceObject *target);

	// Default constructor / destructor
	Order_AttackBasic(void) { }
	~Order_AttackBasic(void) { }

public:

	// Order parameters
	ObjectReference<iSpaceObject>		Target;								// Target of the attack
	float								CloseDist, CloseDistSq;				// Close distance when making a run (based on target size)
	AXMVECTOR							CloseDistV, CloseDistSqV;			// Close distance when making a run (based on target size) - vectorised form
	float								RetreatDist, RetreatDistSq;			// Distance to retreat between runs (based on target size)
	AXMVECTOR							RetreatDistV, RetreatDistSqV;		// Distance to retreat between runs (based on target size) - vectorised form
	

};


#endif
