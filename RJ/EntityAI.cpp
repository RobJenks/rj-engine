#include <sstream>
#include "GameVarsExtern.h"
#include "Utility.h"
#include "Collections.h"
#include "EntityAIStates.h"
#include "EntityAI.h"

// Initialise static fields and constants
const float EntityAI::NEUTRAL_ENTITY_SITUATION_MODIFIER = 0.5f;
EntityAI::DependencyInfoStruct EntityAI::m_dependencycheck;

// Default constructor
EntityAI::EntityAI(void)
{
	// Initialise the unique order ID counter for this entity
	m_ordercreationcount = 0;

	// Initialise entity AI to a default state
	m_eai_state = EntityAIStates::EntityAIState::Independent;
	m_eai_engagement_state = EntityAIStates::EntityEngagementState::EngageIfSensible;
	m_eai_entity_strength = 10.0f;
	m_eai_bravery = 0.0f;
	m_eai_bad_situation_threshold = 0.25f;
	
	// Initialise default values
	MaintenanceFrequency = Game::C_DEFAULT_ORDER_QUEUE_MAINTENANCE_FREQUENCY;
	TimeSinceLastMaintenance = 0U;
}

// Initialise a newly-copied entity AI object to a default starting state
void EntityAI::InitialiseCopiedObject(EntityAI *source)
{
	// Clear any existing orders, since these will otherwise have been copied from the source object
	// Do not delete/erase since this would otherwise delete the orders that are still being executed by the source object
	Orders.clear();

	// Reset the unique order ID counter for this entity
	m_ordercreationcount = 0;

	// Reset other key fields to a starting state
	TimeSinceLastMaintenance = 0U;
}

// Entity AI state determines how much autonomy the entity has in choosing its own actions
void EntityAI::ChangeEntityAIState(EntityAIStates::EntityAIState state)
{
	// If this is not a change we can quit immediately.  Otherwise, store the new state
	if (state == m_eai_state) return;
	m_eai_state = state;

	// Take specific action based on the new entity state
	if (m_eai_state == EntityAIStates::EntityAIState::StrategicControl)
	{
		// If we are changing to a state where are no longer independent, cancel any current orders 
		// that we determined independently
		CancelAllOrdersFromSource(Order::OrderSource::Entity);
	}
	else if (m_eai_state == EntityAIStates::EntityAIState::Independent)
	{
		// Conversely, if we are now becoming independent, cancel any orders we previously received from strategic AI
		CancelAllOrdersFromSource(Order::OrderSource::StrategicAI);
	}
}

// Entity AI engagement state determines how the entity will respond to nearby enemy contacts
void EntityAI::ChangeEntityAIEngagementState(EntityAIStates::EntityEngagementState state)
{
	// If this is not a change we can quit immediately.  Otherwise, store the new state
	if (state == m_eai_engagement_state) return;
	m_eai_engagement_state = state;

	// Most changes will be handled by the regular order processing logic.  However if we are changing to a state
	// where we flee from all engagements, directly remove any 'attack' orders from the queue so we flee immediately
	if (m_eai_engagement_state == EntityAIStates::EntityEngagementState::FleeFromEngagements)
	{
		CancelAllCombatOrders();
	}
}


// Adds a new order to the queue.  Assigns and returns the ID of the new order.  Returns/assigns 0 if the order cannot be assigned
Order::ID_TYPE EntityAI::AssignNewOrder(Order *order)
{
	// Test whether the order is compatible with this entity
	if (!order || CanAcceptOrderType(order->GetType()) == false) return 0;

	// Make sure the order isn't already assigned; if so, refuse to assign the same order again
	if (Orders.end() != std::find_if(Orders.begin(), Orders.end(),
		[&order](const Order *order_element) { return (order == order_element || order->ID == order_element->ID); })) return 0;

	// The order appears to be valid, so generate a new unique ID for it here
	order->ID = GenerateNewUniqueID();

	// Add this order to the overall order queue
	Orders.push_back(order);

	// Return the ID of the item that was just added
	return order->ID;
}

// Generates a new unique order ID for this entity
Order::ID_TYPE EntityAI::GenerateNewUniqueID(void)
{
	// Increment and return the next ID in sequence
	return ++(m_ordercreationcount);
}

// Retrieves an order based on its unique ID.  Returns a pointer to the element in the order queue,
// or NULL if no order exists with that ID
Order * EntityAI::GetOrder(Order::ID_TYPE id)
{
	// Attempt to locate this order in the queue
	EntityAI::OrderQueue::iterator it = std::find_if(Orders.begin(), Orders.end(),
		[&id](const Order *order) { return order->ID == id; });

	// Return either a pointer to the order, or NULL if no order exists with that ID
	return (it == Orders.end() ? NULL : (*it));
}

// Returns an iterator to an order based on its unique ID, or Orders.end() if no order exists with that ID
EntityAI::OrderQueue::iterator EntityAI::GetOrderRef(Order::ID_TYPE id)
{
	// Attempt to locate and return an iterator to this order in the queue
	return std::find_if(Orders.begin(), Orders.end(),
		[&id](const Order *order) { return order->ID == id; });
}

// Cancels an order, if order has either been completed or is no longer valid
void EntityAI::CancelOrder(OrderQueue::iterator order, bool perform_maintenance)
{
	// Erase the order at this iterator position
	Collections::DeleteEraseElement(Orders, order);
	
	// If the maintenance flag is set, check the full order queue to see if this freed up any dependencies
	if (perform_maintenance) MaintainOrderQueue();
}

// Cancels an order, if order has either been completed or is no longer valid
void EntityAI::CancelOrder(Order::ID_TYPE id, bool perform_maintenance)
{
	// Try to locate and erase the order in the queue by its ID
	CancelOrder(GetOrderRef(id), perform_maintenance);
}

// Cancels an order, if order has either been completed or is no longer valid
void EntityAI::CancelOrder(Order *order, bool perform_maintenance)
{
	// Parameter check
	if (!order) return;

	// Try to locate and erase the order in the queue by its ID
	CancelOrder(GetOrderRef(order->ID), perform_maintenance);
}

// Cancels the order at the specified index.  Private method for internal use
void EntityAI::CancelOrderAtIndex(OrderQueue::size_type index, bool perform_maintenance)
{
	// Make sure the index is valid
	if (index < 0 || index >= Orders.size()) return;

	// Remove the order from this entity's queue
	CancelOrder((Orders.begin() + index), perform_maintenance);
}

// Clears the entire order queue, leaving the entity in an idle state
void EntityAI::CancelAllOrders(void)
{
	// Delete and erase the entire order queue
	Collections::DeleteErase(Orders, Orders.begin(), Orders.end());
}

// Cancels all orders received from the specified source
void EntityAI::CancelAllOrdersFromSource(Order::OrderSource source, bool perform_maintenance)
{
	// Remove all orders from the specified source
	Collections::DeleteErase(Orders, [source](const Order *order) { return (order->Source == source); });

	// If the maintenance flag is set, check the full order queue to see if this freed up any dependencies
	if (perform_maintenance) MaintainOrderQueue();
}

// Cancels all orders of the specified type
void EntityAI::CancelAllOrdersOfType(Order::OrderType type, bool perform_maintenance)
{
	// Remove all orders of the specified type
	Collections::DeleteErase(Orders, [type](const Order *order) { return (order->GetType() == type); });

	// If the maintenance flag is set, check the full order queue to see if this freed up any dependencies
	if (perform_maintenance) MaintainOrderQueue();
}

// Cancels all combat-related orders 
void EntityAI::CancelAllCombatOrders(bool perform_maintenance)
{
	// Remove any combat orders from the queue
	Collections::DeleteErase(Orders, [](const Order *order) { return (Order::IsCombatOrderType(order->GetType())); });

	// The next maintenance cycle will remove any active orders that were children of the combat
	// orders we removed.  This method should therefore ideally be called with perform_maintenance = true,
	// or shortly before a maintenance cycle is due to be executed anyway
	if (perform_maintenance) MaintainOrderQueue();
}

// Processes each item in the order queue in parallel (unless an item has active dependency, in which case it is not executed).  'interval' is time since last execution
void EntityAI::ProcessOrderQueue(unsigned int interval)
{
	Order::OrderResult result;
	bool maintenance_required = false;

	// Iterate through the order queue and consider each item in turn
	// We have to do this via a loop rather than iterator since the "ProcessOrder" function can introduce new order
	// into the queue that would otherwise invalidate the iterator.  This way, any new orders will be added to the
	// end, ignored in this cycle (since ordercount is already calculated), and then executed next cycle
	OrderQueue::size_type ordercount = Orders.size();
	for (OrderQueue::size_type i = 0; i < ordercount; ++i)
	{
		// We will process this order if it is valid and active
		Order *order = Orders[i];
		if (order && order->Active)
		{
			// Increment the time since this order was last evaluated
			order->TimeSinceLastEvaluation += interval;

			// No need to continue if we are still under the evaluation frequency, or if we have a dependency preventing us from executing
			if (order->Dependency != 0 || (order->TimeSinceLastEvaluation < order->EvaluationFrequency)) continue;

			// Execute the order and test the result
			result = ProcessOrder(order);

			// If we have signalled the order is complete, or invalid, set it to inactive and trigger maintenance of the order queue
			if (result == Order::OrderResult::ExecutedAndCompleted || result == Order::OrderResult::InvalidOrder)
			{
				order->Active = false;
				maintenance_required = true;
			}
		}
	}

	// If we are over the defined maintenance interval, or if we want to directly initiate it, then also perform a maintenance on the order queue now
	this->TimeSinceLastMaintenance += interval;
	if ( this->TimeSinceLastMaintenance > this->MaintenanceFrequency || maintenance_required ) MaintainOrderQueue();
}

// Maintains the order queue and resolves dependencies, to maintain the integrity of the remaining queue
void EntityAI::MaintainOrderQueue(void)
{
	Order::ID_TYPE id = 0;
	OrderQueue::size_type n = Orders.size();
	EntityAI::m_dependencycheck.clear();

	// Reset the counter since last maintenance of the order queue
	TimeSinceLastMaintenance = 0U;

	// Pass 1: Loop through the order queue and record any active dependencies 
	// Also use this first pass to remove any orders that are no longer active.  This is why we loop rather than iterate
	n = Orders.size();
	for (OrderQueue::size_type i = 0; i < n; ++i)
	{
		// Get a handle to the item
		Order *order = Orders[i];

		// Take action depending on order state
		if (!order || order->Active == false)
		{
			// We want to cancel the order at this index
			CancelOrderAtIndex(i, false);

			// Now reduce the current index and size by 1, so we can continue from the next actual item
			--i; --n;
		}
		else	// The item is valid, so check it for dependencies
		{
			// If this item has a dependency then we need to record it
			if (order->Dependency != 0)
			{
				// Record this item in the dependency vector
				EntityAI::m_dependencycheck.push_back(DependencyInfo(DependencyInfo::DependencyType::Dependency, order, order->Dependency, false));
			}
			if (order->Parent != 0)
			{
				// Also record any active parent pointers, so we can check for any child orders leftover from removed parents
				EntityAI::m_dependencycheck.push_back(DependencyInfo(DependencyInfo::DependencyType::ParentDependency, order, order->Parent, false));
			}
		}
	}

	// If we have no active dependencies then we can quit here
	if (EntityAI::m_dependencycheck.empty()) return;

	// Pass 2: Loop back through the order queue and flag any dependencies that are still active
	EntityAI::OrderQueue::iterator it2_end = Orders.end();
	for (EntityAI::OrderQueue::iterator it2 = Orders.begin(); it2 != it2_end; ++it2)
	{
		// Check the ID of this item against the vector of outstanding dependencies
		id = (*it2)->ID;
		EntityAI::DependencyInfoStruct::iterator it3_end = EntityAI::m_dependencycheck.end();
		for (EntityAI::DependencyInfoStruct::iterator it3 = EntityAI::m_dependencycheck.begin(); it3 != it3_end; ++it3)
		{
			if ((*it3).dependency == id)
				(*it3).found = true;			// We have found the item this is dependent on, so set found = true
		}
	}

	// Finally, check the dependency vector for any items where found == false.  
	EntityAI::DependencyInfoStruct::iterator it4_end = EntityAI::m_dependencycheck.end();
	for (EntityAI::DependencyInfoStruct::iterator it4 = EntityAI::m_dependencycheck.begin(); it4 != it4_end; ++it4)
	{
		const EntityAI::DependencyInfo & dep = (*it4);
		if (dep.found == false)
		{
			// We need to take action on any items with dependencies which were not found in the order queue
			if (dep.type == EntityAI::DependencyInfo::DependencyType::ParentDependency)
				dep.order->Active = false;			// If the item parent could not be found, mark the order as inactive for removal next maintenance cycle
			else
				dep.order->Dependency = 0;			// If the item dependency could not be found, remove the dependency to mark it as ready for execution
		}
	}
}

// Finds and returns all orders that are currently being executed, i.e. are active and have no dependencies before they can execute
EntityAI::OrderQueue::size_type EntityAI::GetExecutingOrderCount(void) const
{
	EntityAI::OrderQueue::size_type count = 0;
	EntityAI::OrderQueue::const_iterator it_end = Orders.end();
	for (EntityAI::OrderQueue::const_iterator it = Orders.begin(); it != it_end; ++it)
	{
		if ((*it) && (*it)->Active && (*it)->Dependency == 0) ++count;
	}

	return count;
}


// Outputs a vector of pointers to orders that are currently being executed, i.e. are active and have no dependencies before they can execute
void EntityAI::GetAllExecutingOrders(std::vector<const Order*> &outExecutingOrders)
{
	outExecutingOrders.clear();
	EntityAI::OrderQueue::iterator it_end = Orders.end();
	for (EntityAI::OrderQueue::iterator it = Orders.begin(); it != it_end; ++it)
	{
		const Order *order = (*it);
		if (order && order->Active && order->Dependency == 0) outExecutingOrders.push_back(order);
	}
}

// Returns a string representation of the entity's current order queue, for debug purposes
std::string EntityAI::GetDebugOrderQueueString(void) const
{
	std::ostringstream s;
	EntityAI::OrderQueue::const_iterator it_end = Orders.end();
	for (EntityAI::OrderQueue::const_iterator it = Orders.begin(); it != it_end; ++it)
	{
		const Order *order = (*it);
		if (it != Orders.begin()) s << ", ";

		if (order->Active == false)			s << "[[[" << Order::TranslateOrderTypeToString(order->GetType()) << " (" << order->ID << ")]]]";
		else if (order->Dependency != 0)	s << "[" << Order::TranslateOrderTypeToString(order->GetType()) << " (" << order->ID << ")]";
		else								s << Order::TranslateOrderTypeToString(order->GetType()) << " (" << order->ID << ")";
	}

	return s.str();
}

// Destructor
EntityAI::~EntityAI(void)
{
	// Cancel any orders still in the queue to deallocate any associated memory
	CancelAllOrders();
}


// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void EntityAI::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(GetOrder, (Order::ID_TYPE)command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(HasOrders)
	REGISTER_DEBUG_ACCESSOR_FN(GetOrderCount)
	REGISTER_DEBUG_ACCESSOR_FN(GetExecutingOrderCount)
	REGISTER_DEBUG_ACCESSOR_FN(CanAcceptOrderType, (Order::OrderType)command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(GetEntityAIState)
	REGISTER_DEBUG_ACCESSOR_FN(GetEntityAIEngagementState)
	REGISTER_DEBUG_ACCESSOR_FN(GetEstimatedEntityStrength)
	REGISTER_DEBUG_ACCESSOR_FN(GetEntityBraveryModifier)
	REGISTER_DEBUG_ACCESSOR_FN(GetEntityBadSituationThreshold)
	REGISTER_DEBUG_ACCESSOR_FN(GetDebugOrderQueueString)

	// Mutator methods
	REGISTER_DEBUG_FN(CancelOrder, (Order::ID_TYPE)command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(CancelAllOrders)
	REGISTER_DEBUG_FN(CancelAllOrdersFromSource, (Order::OrderSource)command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(CancelAllOrdersOfType, (Order::OrderType)command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(CancelAllCombatOrders)
	REGISTER_DEBUG_FN(ProcessOrderQueue, (unsigned int)command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(MaintainOrderQueue)
	REGISTER_DEBUG_FN(ChangeEntityAIState, (EntityAIStates::EntityAIState)command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(ChangeEntityAIEngagementState, (EntityAIStates::EntityEngagementState)command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(SetEntityBraveryModifier, command.ParameterAsFloat(2))
	REGISTER_DEBUG_FN(SetEntityBadSituationThreshold, command.ParameterAsFloat(2))


	// Pass processing back to any base classes, if applicable, if we could not execute the function
	/* No base classes to pass control back to */

}