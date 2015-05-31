#include <vector>
#include <algorithm>
#include "Utility.h"
#include "GameDataExtern.h"
#include "ScheduledObject.h"

#include "CentralScheduler.h"

// Initialise static counter for assigning unique IDs to scheduled jobs
CentralScheduler::ID_TYPE CentralScheduler::CurrentSchedulerID = 0UL;

// Initialise static comparator for binary search comparisons
CentralScheduler::_ScheduleIDComparator CentralScheduler::ScheduleIDComparator = CentralScheduler::_ScheduleIDComparator();

// Default constructor
CentralScheduler::CentralScheduler(void)
{
	// Initialise all values to their defaults
	m_lastinfrequentupdate = 0U;
}

CentralScheduler::ID_TYPE CentralScheduler::ScheduleFrequentUpdates(ScheduledObject *object, int interval_ms)	
{
	// Parameter check
	if (!object) return 0UL;

	// Schedule the task
	m_schedule_frequent.push_back(ScheduledItemDetails(object, max(interval_ms, 1UL)));
	return CentralScheduler::CurrentSchedulerID;								// This will be the ID just assigned
}

// Add an item for infrequent (every "InfrequentUpdateEvaluationFrequency" ms) evaluation
CentralScheduler::ID_TYPE CentralScheduler::ScheduleInfrequentUpdates(ScheduledObject *object, int interval_ms)	
{
	// Parameter check
	if (!object) return 0UL;

	// Schedule the task
	m_schedule_infrequent.push_back(ScheduledItemDetails(object, max(interval_ms, 1UL)));
	return CentralScheduler::CurrentSchedulerID;								// This will be the ID just assigned
}


// Central scheduling method.  Updates refresh counters and calls update methods where appropriate
void CentralScheduler::RunScheduler(void)
{
	// Check all frequently-scheduled objects, which are evaluated every frame
	it_end = m_schedule_frequent.end();
	for (it = m_schedule_frequent.begin(); it != it_end; ++it)
	{
		if ((it->TimeSinceLastUpdate += Game::ClockDelta) >= it->UpdateInterval)
		{
			it->Object->Update();
			it->TimeSinceLastUpdate = 0U;
		}
	}

	// Determine whether we also need to update infrequently-evaluated objects
	if ((m_lastinfrequentupdate += Game::ClockDelta) >= CentralScheduler::InfrequentUpdateEvaluationFrequency)
	{
		it_end = m_schedule_infrequent.end();
		for (it = m_schedule_infrequent.begin(); it != it_end; ++it)
		{
			if ((it->TimeSinceLastUpdate += m_lastinfrequentupdate) >= it->UpdateInterval)
			{
				it->Object->UpdateInfrequent();
				it->TimeSinceLastUpdate = 0U;
			}
		}
		m_lastinfrequentupdate = 0U;
	}
}

// Remove an item based on its schedule ID
void CentralScheduler::RemoveFrequentUpdate(ID_TYPE id)
{
	// We can perform a "lower-bound" binary search for the element since IDs are stored sequentially
	it = std::lower_bound(m_schedule_frequent.begin(), m_schedule_frequent.end(), id, CentralScheduler::ScheduleIDComparator);

	// The lower bound (binary search) function will return the first element >= to the target value, so test it here to be sure
	if (it->ID == id)
	{
		// Determine the index of this element by pointer arithmetic and remove it from the vector
		RemoveFromVectorAtIndex<ScheduledItemDetails>(m_schedule_frequent, (it - m_schedule_frequent.begin()));
	}
}

// Remove an item based on its schedule ID
void CentralScheduler::RemoveInfrequentUpdate(ID_TYPE id)
{
	// We can perform a "lower-bound" binary search for the element since IDs are stored sequentially
	it = std::lower_bound(m_schedule_infrequent.begin(), m_schedule_infrequent.end(), id, CentralScheduler::ScheduleIDComparator);

	// The lower bound (binary search) function will return the first element >= to the target value, so test it here to be sure
	if (it->ID == id)
	{
		// Determine the index of this element by pointer arithmetic and remove it from the vector
		RemoveFromVectorAtIndex<ScheduledItemDetails>(m_schedule_infrequent, (it - m_schedule_infrequent.begin()));
	}
}

// Remove an item based on the object reference itself.  Pass to 
void CentralScheduler::RemoveFrequentUpdate(ScheduledObject *object)
{
	int n = m_schedule_frequent.size();
	for (int i = 0; i < n; i++)
	{
		if (m_schedule_frequent[i].Object == object)
		{
			// The RemoveFromVectorAtIndex method keeps items in sequential order - it does not use swap/pop - so 
			// we can simply call this method to remove items without breaking the rule that items must be sequential by ID
			RemoveFromVectorAtIndex<ScheduledItemDetails>(m_schedule_frequent, i);
			return;
		}
	}
}

// Remove an item based on the object reference itself.  Slower than removing based on the ID since requires linear search
void CentralScheduler::RemoveInfrequentUpdate(ScheduledObject *object)
{
	int n = m_schedule_infrequent.size();
	for (int i = 0; i < n; i++)
	{
		if (m_schedule_infrequent[i].Object == object)
		{
			// The RemoveFromVectorAtIndex method keeps items in sequential order - it does not use swap/pop - so 
			// we can simply call this method to remove items without breaking the rule that items must be sequential by ID
			RemoveFromVectorAtIndex<ScheduledItemDetails>(m_schedule_infrequent, i);
			return;
		}
	}
}



CentralScheduler::~CentralScheduler(void)
{
}
