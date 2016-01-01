#pragma once

#ifndef __OrderH__
#define __OrderH__

#include "DX11_Core.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Order : public ALIGN16<Order>
{
public:
	// Define the data type used to store order IDs.  ID is always positive with 0 representing 'null'/no order
	typedef unsigned long ID_TYPE;

	// Enumeration of all available order types
	enum OrderType 
	{
		Unknown = 0, 

		MoveToPosition,						// Orders a ship to move DIRECTLY to the specified position
		MoveToTarget,						// Orders a ship to move DIRECTLY to the designated target
		MoveAwayFromTarget,					// Orders a ship to put distance between itself and the designated target
		AttackBasic,						// Performs basic attack maneuvers against the target object
		
		ActorMoveToPosition,				// Orders an actor to move DIRECTLY to the specified position
		ActorMoveToTarget,					// Orders an actor to move DIRECTLY to the designated target
		ActorTravelToPosition,				// Orders an actor to move to the specified position using the local nav network
		ActorTravelToTarget					// Orders an actor to move to the designated target using the local nav network
	};

	// Enumeration of possible results upon executing an order
	enum OrderResult
	{
		NotExecuted = 0,			// Default.  If no action has been taken on this order
		InvalidOrder,				// If the order is invalid, or not executable by the entity
		Executed,					// If the entity evaluated the order this cycle
		ExecutedAndCompleted		// If the entity evaluated the order, and determined it can be removed from the queue
	};

	// Enumeration of possible order sources
	enum OrderSource
	{
		Entity = 0,
		StrategicAI
	};

	// Unique (to the owner) ID of this order
	ID_TYPE										ID;

	// Determines whether the order is active.  If not, it effectively does not exist
	bool										Active;

	// Method to retrieve the type of order this represents
	CMPINLINE OrderType							GetType(void) const		{ return m_ordertype; }

	// Dependency on another order before this is executed (0 == no dependency)
	ID_TYPE										Dependency;

	// Pointer to the parent order of this one, if relevant.  Used when one order spawns multiple child requests
	ID_TYPE										Parent;

	// Source of the order, i.e. who assigned it to the entity
	OrderSource									Source;

	// The frequency of order evaluation, and time since the order was last evaluated
	unsigned int								EvaluationFrequency;
	unsigned int								TimeSinceLastEvaluation;

	// Constructor / destructor
	Order(void);
	virtual ~Order(void);

	// Returns a flag indicating whether the specified order type is a combat order
	CMPINLINE static bool						IsCombatOrderType(Order::OrderType order_type)
	{
		switch (order_type)
		{
			case OrderType::AttackBasic:
				return true;
			default:
				return false;
			}
	}

	// Translates an order type to its string representation
	static std::string							TranslateOrderTypeToString(Order::OrderType order_type);

protected:

	// Order type is a protected field set only upon construction
	Order::OrderType							m_ordertype;

};



#endif