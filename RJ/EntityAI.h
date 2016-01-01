#pragma once

#ifndef __EntityAIH__
#define __EntityAIH__

#include <queue>
#include "ErrorCodes.h"
#include "EntityAIStates.h"
#include "Order.h"
using namespace std;

// This class does not have any special alignment requirements
class EntityAI
{
public:

	// Queue of orders currently assigned to this entity
	typedef vector<Order*>					OrderQueue;
	OrderQueue								Orders;

	// Initialise a newly-copied entity AI object to a default starting state
	void									InitialiseCopiedObject(EntityAI *source);

	// Adds a new order to the queue.  Assigns and returns the ID of the new order.  Returns/assigns 0 if the order cannot be assigned
	Order::ID_TYPE							AssignNewOrder(Order *order);

	// Retrieves an order based on its unique ID
	Order *									GetOrder(Order::ID_TYPE id);

	// Returns an iterator to an order based on its unique ID, or Orders.end() if no order exists with that ID
	OrderQueue::iterator					GetOrderRef(Order::ID_TYPE id);

	// Cancels an order, if order has either been completed or is no longer valid
	void									CancelOrder(OrderQueue::iterator order, bool perform_maintenance);
	CMPINLINE void							CancelOrder(OrderQueue::iterator order)									{ CancelOrder(order, true); }
	void									CancelOrder(Order::ID_TYPE id, bool perform_maintenance); 
	CMPINLINE void							CancelOrder(Order::ID_TYPE id)											{ CancelOrder(id, true); }
	void									CancelOrder(Order * order, bool perform_maintenance);
	CMPINLINE void							CancelOrder(Order * order)												{ CancelOrder(order, true); }

	// Clears the entire order queue, leaving the entity in an idle state
	void									CancelAllOrders(void);

	// Cancels all orders received from the specified source
	void									CancelAllOrdersFromSource(Order::OrderSource source, bool perform_maintenance);
	CMPINLINE void							CancelAllOrdersFromSource(Order::OrderSource source)					{ CancelAllOrdersFromSource(source, true); }

	// Cancels all orders of the specified type
	void									CancelAllOrdersOfType(Order::OrderType type, bool perform_maintenance);
	CMPINLINE void							CancelAllOrdersOfType(Order::OrderType type)							{ CancelAllOrdersOfType(type, true); }

	// Cancels all combat-related orders 
	void									CancelAllCombatOrders(bool perform_maintenance);
	CMPINLINE void							CancelAllCombatOrders(void)												{ CancelAllCombatOrders(true); }

	// Processes each item in the order queue in parallel (unless an item has active dependency, in which case it is not executed).  'interval' is time since last execution
	void									ProcessOrderQueue(unsigned int interval);

	// Maintains the order queue and resolves dependencies, to maintain the integrity of the remaining queue
	void									MaintainOrderQueue(void);

	// Accessor methods for key order queue properties
	bool									HasOrders(void)					{ return (Orders.size() > 0); }
	OrderQueue::size_type					GetOrderCount(void)				{ return  Orders.size(); }

	// Finds and returns the number of orders that are currently being executed, i.e. are active and have no dependencies before they can execute
	OrderQueue::size_type					GetExecutingOrderCount(void) const;

	// Outputs a vector of pointers to orders that are currently being executed, i.e. are active and have no dependencies before they can execute
	void									GetAllExecutingOrders(std::vector<const Order*> & outExecutingOrders);

	// Virtual method to process the specified order.  Called when processing the full queue.  Returns result of processing the order
	virtual Order::OrderResult				ProcessOrder(Order *order) = 0;

	// Virtual method to determine whether the entity can accept an order of the given type
	virtual bool							CanAcceptOrderType(Order::OrderType type) = 0;

	// Entity AI state determines how much autonomy the entity has in choosing its own actions
	CMPINLINE EntityAIStates::EntityAIState	GetEntityAIState(void) const						{ return m_eai_state; }
	void									ChangeEntityAIState(EntityAIStates::EntityAIState state);

	// Entity AI engagement state determines how the entity will respond to nearby enemy contacts
	CMPINLINE EntityAIStates::EntityEngagementState	GetEntityAIEngagementState(void) const		{ return m_eai_engagement_state; }
	void											ChangeEntityAIEngagementState(EntityAIStates::EntityEngagementState state);

	// Entity strength is an assessment of its own strength, which is used for sizing up enemies and evaluating the current sitaution
	CMPINLINE float							GetEstimatedEntityStrength(void) const				{ return m_eai_entity_strength; }

	// Entity bravery indicates how willing they are to take on unfavourable combat situations 
	// Bravery is a multiplier on our own strength and can be negative for cowardly entities
	CMPINLINE float							GetEntityBraveryModifier(void) const				{ return m_eai_bravery; }
	CMPINLINE void							SetEntityBraveryModifier(float modifier)			{ m_eai_bravery = modifier; }

	// Modifier applied to neutral entities (who may/may not help us in a fight) when assessing the current situation
	static const float						NEUTRAL_ENTITY_SITUATION_MODIFIER;

	// The "bad situation" threshold is the ratio of friendly to enemy strength, below which 
	// we consider the situation to be too unfavourable to remain
	CMPINLINE float							GetEntityBadSituationThreshold(void) const			{ return m_eai_bad_situation_threshold; }
	CMPINLINE void							SetEntityBadSituationThreshold(float threshold)		{ m_eai_bad_situation_threshold = threshold; }

	// Returns a string representation of the entity's current order queue, for debug purposes
	std::string								GetDebugOrderQueueString(void) const;

	// Constructor / destructor
	EntityAI(void);
	~EntityAI(void);

protected:

	// Key entity AI states
	EntityAIStates::EntityAIState				m_eai_state;
	EntityAIStates::EntityEngagementState		m_eai_engagement_state;

	// Entity assessment of its own strength, which is used for sizing up enemies and evaluating the current sitaution
	float										m_eai_entity_strength;
	
	// Entity bravery indicates how willing they are to take on unfavourable combat situations 
	// Bravery is a multiplier on our own strength and can be negative for cowardly entities
	float										m_eai_bravery;

	// The "bad situation" threshold is the ratio of friendly to enemy strength, below which 
	// we consider the situation to be too unfavourable to remain
	float										m_eai_bad_situation_threshold;

	// Unique ID counter for this entity; method will generate a new unique order ID for this entity on request
	Order::ID_TYPE								m_ordercreationcount;
	Order::ID_TYPE								GenerateNewUniqueID(void);

	// Struct used to store basic info when evaluating dependencies.  Valid ONLY at the point of evaluation
	// since this relies on pointers into the order vector; any modification of the vector could result
	// in pointers becoming invalidated.  Order pointer is only used for convenience when checking
	// dependencies during order queue maintenance
	struct DependencyInfo 
	{ 
		enum DependencyType { Dependency = 0, ParentDependency };
		DependencyType type; Order *order; Order::ID_TYPE dependency; bool found;
		DependencyInfo(void) { type = DependencyType::Dependency; order = NULL; dependency = 0; found = false; }
		DependencyInfo(DependencyType _type, Order *_order, Order::ID_TYPE _dependency, bool _found)
		{
			type = _type; order = _order; dependency = _dependency; found = _found;
		}
	};

	// Vector of dependency structs, used for efficienctly evaluating dependencies in the queue
	typedef vector<DependencyInfo>			DependencyInfoStruct;
	static DependencyInfoStruct				m_dependencycheck;

	// The frequency of order queue maintenance, and time since the queue was last checked
	unsigned int							MaintenanceFrequency;
	unsigned int							TimeSinceLastMaintenance;

	// Cancels the order at the specified index
	void									CancelOrderAtIndex(OrderQueue::size_type index, bool perform_maintenance);
	
};



#endif