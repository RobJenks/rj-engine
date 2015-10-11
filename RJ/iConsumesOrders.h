#pragma once

#ifndef __iConsumesOrdersH__
#define __iConsumesOrdersH__

#include <queue>
#include "ErrorCodes.h"
#include "Order.h"
using namespace std;

class iConsumesOrders
{
public:

	// Queue of orders currently assigned to this entity
	typedef vector<Order*>					OrderQueue;
	OrderQueue								Orders;

	// Adds a new order to the queue.  Assigns and returns the ID of the new order.  Returns/assigns 0 if the order cannot be assigned
	Order::ID_TYPE							AssignNewOrder(Order *order);

	// Retrieves an order based on its unique ID
	Order *									GetOrder(Order::ID_TYPE id);

	// Cancels an order, if order has either been completed or is no longer valid
	void									CancelOrder(Order *order, bool perform_maintenance);
	void									CancelOrder(Order::ID_TYPE id, bool perform_maintenance);

	// Clears the entire order queue, leaving the entity in an idle state
	void									CancelAllOrders(void);

	// Processes each item in the order queue in parallel (unless an item has active dependency, in which case it is not executed).  'interval' is time since last execution
	void									ProcessOrderQueue(float interval);

	// Maintains the order queue and resolves dependencies, to maintain the integrity of the remaining queue
	void									MaintainOrderQueue(void);

	// Accessor methods for key order queue properties
	bool									HasOrders(void)					{ return (Orders.size() > 0); }
	OrderQueue::size_type					GetOrderCount(void)				{ return  Orders.size(); }

	// Virtual method to process the specified order.  Called when processing the full queue.  Returns result of processing the order
	virtual Order::OrderResult				ProcessOrder(Order *order) = 0;

	// Virtual method to determine whether the entity can accept an order of the given type
	virtual bool							CanAcceptOrderType(Order::OrderType type) = 0;


	// Constructor / destructor
	iConsumesOrders(void);
	~iConsumesOrders(void);

private:

	// Unique ID counter for this entity; method will generate a new unique order ID for this entity on request
	Order::ID_TYPE							m_ordercreationcount;
	Order::ID_TYPE							GenerateNewUniqueID(void);

	// Struct used to store basic info when evaluating dependencies
	struct DependencyInfo 
	{ 
		Order *order; Order::ID_TYPE dependency; bool found;
		DependencyInfo(void) { this->order = NULL; this->dependency = 0; this->found = false; }
		DependencyInfo(Order *order, Order::ID_TYPE dependency, bool found)
		{ this->order = order; this->dependency = dependency; this->found = found; }
	};

	// Vector of dependency structs, used for efficienctly evaluating dependencies in the queue
	typedef vector<DependencyInfo>			DependencyInfoStruct;
	DependencyInfoStruct					m_dependencycheck;

	// The frequency of order queue maintenance, and time since the queue was last checked
	float									MaintenanceFrequency;
	float									TimeSinceLastMaintenance;

	// Cancels the order at the specified index
	void									CancelOrderAtIndex(OrderQueue::size_type index, bool perform_maintenance);
};



#endif