#pragma once

#ifndef __Order_MoveAwayFromTargetH__
#define __Order_MoveAwayFromTargetH__

#include "Order.h"
#include "ObjectReference.h"
#include "Utility.h"
class iSpaceObject;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order_MoveAwayFromTarget : public ALIGN16<Order_MoveAwayFromTarget>, public Order
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(Order_MoveAwayFromTarget)

	// Constructor including main order parameters.  The existing momentum weighting [0.0 - 1.0] specifies how
	// much weight the ship's existing momentum vector should play in plotting a retreat path
	Order_MoveAwayFromTarget(iSpaceObject *target, float retreat_distance, float existing_momentum_weighting)
		:
		Target(target),
		RetreatDistance(max(retreat_distance, 1.0f)),
		RetreatDistanceSq(RetreatDistance * RetreatDistance),
		RetreatDistanceSqV(XMVectorReplicate(RetreatDistanceSq)),
		MomentumWeighting(clamp(existing_momentum_weighting, 0.0f, 1.0f)),
		_VectorTravelTarget(RetreatDistance * 10.0f)
	{
		// All order subclasses must set their order type on construction
		m_ordertype = Order::OrderType::MoveAwayFromTarget;
	}

	// Default destructor
	~Order_MoveAwayFromTarget(void) { }

public:

	ObjectReference<iSpaceObject>	Target;
	float							RetreatDistance, RetreatDistanceSq;
	XMVECTOR						RetreatDistanceSqV;
	float							MomentumWeighting;
	float							_VectorTravelTarget;	// Multiple of the retreat distance, precalculated, for efficiency at runtime

};


#endif
