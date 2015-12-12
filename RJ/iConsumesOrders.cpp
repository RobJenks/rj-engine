#include "GameVarsExtern.h"
#include "Utility.h"
#include "iConsumesOrders.h"

// TODO: Can use more efficiency vector search in future, if we can guarantee that orders are added in sorted order

// Default constructor
iConsumesOrders::iConsumesOrders(void)
{
	// Initialise the unique order ID counter for this entity
	this->m_ordercreationcount = 0;

	// Initialise default values
	this->MaintenanceFrequency = Game::C_DEFAULT_ORDER_QUEUE_MAINTENANCE_FREQUENCY;
	this->TimeSinceLastMaintenance = 0.0f;
}

// Adds a new order to the queue.  Assigns and returns the ID of the new order.  Returns/assigns 0 if the order cannot be assigned
Order::ID_TYPE iConsumesOrders::AssignNewOrder(Order *order)
{
	// Test whether the order is compatible with this entity
	if (!order || !this->CanAcceptOrderType(order->GetType())) return 0;

	// Make sure the order isn't already assigned; if so, refuse to assign the same order again
	if (FindInVector<Order*>(Orders, order) > -1) return 0;

	// The order appears to be valid, so generate a new unique ID for it here
	order->ID = GenerateNewUniqueID();

	// Add this order to the overall order queue
	Orders.push_back(order);

	// Return the ID of the item that was just added
	return order->ID;
}

// Generates a new unique order ID for this entity
Order::ID_TYPE iConsumesOrders::GenerateNewUniqueID(void)
{
	// Increment and return the next ID in sequence
	return ++(m_ordercreationcount);
}

// Retrieves an order based on its unique ID
Order * iConsumesOrders::GetOrder(Order::ID_TYPE id)
{
	// We can find items via linear search since there will only ever be a few orders active in the queue
	iConsumesOrders::OrderQueue::const_iterator it_end = Orders.end();
	for (iConsumesOrders::OrderQueue::const_iterator it = Orders.begin(); it != it_end; ++it)
		if ((*it) && (*it)->ID == id) return (*it);

	// If we didn't find the order then return NULL
	return NULL;
}

// Cancels an order, if order has either been completed or is no longer valid
void iConsumesOrders::CancelOrder(Order::ID_TYPE id, bool perform_maintenance)
{
	// Try to locate this order in the queue by its ID
	Order *order = GetOrder(id);

	// If the order does exist, pass to the overloaded method and cancel it now
	if (order) CancelOrder(order, perform_maintenance);
}

// Cancels an order, if order has either been completed or is no longer valid
void iConsumesOrders::CancelOrder(Order *order, bool perform_maintenance)
{
	// Parameter check
	if (!order) return;

	// Make sure we do actually have this order in our queue
	int index = FindInVector<Order*>(Orders, order);
	if (index < 0) return;

	// Call the internal method to remove this order by its index in the order queue
	CancelOrderAtIndex(index, perform_maintenance);
}

// Cancels the order at the specified index.  Private method for internal use
void iConsumesOrders::CancelOrderAtIndex(OrderQueue::size_type index, bool perform_maintenance)
{
	// Make sure the index is valid
	if (index >= Orders.size()) return;
	Order *order = Orders.at(index);

	// Remove the order from this entity's queue
	RemoveFromVectorAtIndex<Order*>(Orders, index);

	// We also want to deallocate the order object; orders cannot be transferred between entities
	if (order)
	{
		order->ID = 0; delete order; order = NULL;
	}

	// If the maintenance flag is set, check the full order queue to see if this freed up any dependencies
	if (perform_maintenance) MaintainOrderQueue();
}

// Clears the entire order queue, leaving the entity in an idle state
void iConsumesOrders::CancelAllOrders(void)
{
	// Iterate through the queue and deallocate each order in turn
	iConsumesOrders::OrderQueue::iterator it_end = Orders.end();
	for (iConsumesOrders::OrderQueue::iterator it = Orders.begin(); it != it_end; ++it)
	{
		// Deallocate this order
		if (*it) { (*it)->ID = 0; delete (*it); (*it) = NULL; }
	}

	// Now clear the order queue itself, since its contents are all now invalid anyway
	Orders.clear();
}

// Processes each item in the order queue in parallel (unless an item has active dependency, in which case it is not executed).  'interval' is time since last execution
void iConsumesOrders::ProcessOrderQueue(float interval)
{
	Order *order;
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
		order = Orders[i];
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
void iConsumesOrders::MaintainOrderQueue(void)
{
	int depcount = 0;
	OrderQueue::size_type id = 0, n;

	// Reset the counter since last maintenance of the order queue
	TimeSinceLastMaintenance = 0.0f;

	// Pass 1: Loop through the order queue and record any active dependencies 
	// Also use this first pass to remove any orders that are no longer active.  This is why we loop rather than iterate
	n = Orders.size();
	for (OrderQueue::size_type i = 0; i < n; ++i)
	{
		// Get a handle to the item
		Order *order = Orders.at(i);

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
				// If this is the first dependency we are finding then initialise the dependency vector now
				if (depcount == 0) m_dependencycheck.clear();

				// Record this item in the dependency vector and increase the count of dependencies found
				m_dependencycheck.push_back(DependencyInfo(order, order->Dependency, false));
				++depcount;
			}
		}
	}

	// If we have no active dependencies then we can quit here
	if (depcount == 0) return;

	// Pass 2: Loop back through the order queue and flag any dependencies that are still active
	iConsumesOrders::OrderQueue::iterator it2_end = Orders.end();
	for (iConsumesOrders::OrderQueue::iterator it2 = Orders.begin(); it2 != it2_end; ++it2)
	{
		// Make sure this is a valid order
		if (!(*it2)) continue;

		// Check the ID of this item against the vector of outstanding dependencies
		id = (*it2)->ID;
		iConsumesOrders::DependencyInfoStruct::iterator it3_end = m_dependencycheck.end();
		for (iConsumesOrders::DependencyInfoStruct::iterator it3 = m_dependencycheck.begin(); it3 != it3_end; ++it3)
			if ((*it3).dependency == id)
				(*it3).found = true;			// We have found the item this is dependent on, so set found = true
	}

	// Finally, check the dependency vector for any items where found == false.  
	iConsumesOrders::DependencyInfoStruct::iterator it4_end = m_dependencycheck.end();
	for (iConsumesOrders::DependencyInfoStruct::iterator it4 = m_dependencycheck.begin(); it4 != it4_end; ++it4)
	{
		// If any items exist with found == false, remove their dependency so they can now be executed
		if ((*it4).found == false) (*it4).order->Dependency = 0;
	}
}

// Finds and returns all orders that are currently being executed, i.e. are active and have no dependencies before they can execute
void iConsumesOrders::GetAllExecutingOrders(std::vector<Order*> & outOrders)
{
	Order *order;
	iConsumesOrders::OrderQueue::iterator it_end = Orders.end();
	for (iConsumesOrders::OrderQueue::iterator it = Orders.begin(); it != it_end; ++it)
	{
		order = (*it);
		if (order && order->Active && order->Dependency == 0) outOrders.push_back(order);
	}
}

// Destructor
iConsumesOrders::~iConsumesOrders(void)
{
	// Cancel any orders still in the queue to deallocate any associated memory
	CancelAllOrders();
}
