#pragma once

#ifndef __CentralSchedulerH__
#define __CentralSchedulerH__

#include <vector>
class ScheduledObject;

class CentralScheduler
{
public:

	// Define the type used as a unique ID for all scheduled jobs.  Unique across all scheduled instances/processes
	typedef unsigned long		ID_TYPE;

	// Static counter used to assign unique IDs to scheduled tasks
	static CentralScheduler::ID_TYPE				CurrentSchedulerID;

	// Interval between evaluations of infrequently-updated objects (frequently updated objects are checked once per frame)
	static const unsigned int	InfrequentUpdateEvaluationFrequency = 2000;

	// Structure of scheduling information for an object
	struct ScheduledItemDetails
	{
		ID_TYPE					ID;						// Automatically assigned, sequential ID.  Can be used to locate items more quickly than linear search
		ScheduledObject *		Object;					// The object being scheduled
		unsigned int			UpdateInterval;			// Time (ms) between update of the object.  Set to zero to check every frame.
		unsigned int			TimeSinceLastUpdate;	// Time (ms) since the object was last checked

		ScheduledItemDetails(ScheduledObject *object, int interval_ms)
		{
			ID = ++CentralScheduler::CurrentSchedulerID;								// Assign next sequential unique ID
			Object = object; UpdateInterval = interval_ms; TimeSinceLastUpdate = 0U;
		}
	};

	// Default constructor
	CentralScheduler(void);

	// Add an item for frequent (once per frame) evaluation
	CMPINLINE ID_TYPE			ScheduleFrequentUpdates(ScheduledObject *object, int interval_ms)	
	{
		m_schedule_frequent.push_back(ScheduledItemDetails(object, interval_ms));
		return CentralScheduler::CurrentSchedulerID;								// This will be the ID just assigned
	}

	// Add an item for infrequent (every "InfrequentUpdateEvaluationFrequency" ms) evaluation
	CMPINLINE ID_TYPE			ScheduleInfrequentUpdates(ScheduledObject *object, int interval_ms)	
	{
		m_schedule_infrequent.push_back(ScheduledItemDetails(object, interval_ms));
		return CentralScheduler::CurrentSchedulerID;								// This will be the ID just assigned
	}

	// Central scheduling method.  Updates refresh counters and calls update methods where appropriate
	void						RunScheduler(void);

	// Remove an item based on its schedule ID
	void						RemoveFrequentUpdate(ID_TYPE id);
	void						RemoveInfrequentUpdate(ID_TYPE id);

	// Remove an item based on the object reference itself.  Slower than removing based on the ID since requires linear search
	void						RemoveFrequentUpdate(ScheduledObject *object);
	void						RemoveInfrequentUpdate(ScheduledObject *object);


	// Default destructor
	~CentralScheduler(void);

private:

	// Vector of items to be checked every frame.  Used for frequently-updated items, e.g. those updating every few frames.  Calls Update()
	std::vector<ScheduledItemDetails>				m_schedule_frequent;

	// Vector of items to be checked every 2000ms of game time.  Used for items not requiring real-time update.  Calls UpdateInfrequent()
	std::vector<ScheduledItemDetails>				m_schedule_infrequent;

	// Keep track of time since the last infrequent update
	unsigned int									m_lastinfrequentupdate;
	
	// Temporary iterators used for evaluating items each frame.  Allocated one here for efficiency
	std::vector<ScheduledItemDetails>::iterator		it, it_end;

	// Functor for sorting/searching jobs based on numeric ID
	static struct _ScheduleIDComparator
	{
		bool operator() (const ScheduledItemDetails & lhs, const ScheduledItemDetails & rhs) const { return lhs.ID < rhs.ID; }
		bool operator() (const CentralScheduler::ID_TYPE lhs, const ScheduledItemDetails & rhs) const { return lhs < rhs.ID; }
		bool operator() (const ScheduledItemDetails & lhs, const CentralScheduler::ID_TYPE rhs) const { return lhs.ID < rhs; }

	};

	// Static instance of the comparator for performing binary searches
	static _ScheduleIDComparator ScheduleIDComparator;
};






#endif