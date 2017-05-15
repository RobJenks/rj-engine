#include <assert.h>

#include "EnvironmentHullBreaches.h"


// Constructor
EnvironmentHullBreaches::EnvironmentHullBreaches()
{
}

// Record a new hull breach from 'element_index' in the direction of 'direction' towards destroyed element 'destroyed_element'
void EnvironmentHullBreaches::RecordHullBreach(int element_index, Direction direction, int destroyed_element)
{
	// Determine the index into our breach collection
	int index = GetBreachAtLocation(element_index);
	if (index == -1)
	{
		// No breach has been recorded for this location yet, so create a new entry
		m_breaches.push_back(EnvironmentHullBreach(element_index));
		index = m_breaches.size() - 1;
	}

	// Add a new entry, assuming it has not already been recorded
	if (!m_breaches[index].HaveBreachToElement(destroyed_element))
	{
		m_breaches[index].AddBreach(EnvironmentHullBreachDetails(destroyed_element, direction));
	}
}

// Reset the record of all hull breaches
void EnvironmentHullBreaches::Reset(void)
{
	m_breaches.clear();
}

// Returns a reference to a particular breach
EnvironmentHullBreach & EnvironmentHullBreaches::Get(int breach_index)
{
	assert(breach_index >= 0 && breach_index < (int)m_breaches.size());
	return m_breaches[breach_index];
}

// Determines whether a breach is present at the given location, and returns the breach_index if there is
// Returns -1 if there is no breach at the specified element
int	EnvironmentHullBreaches::GetBreachAtLocation(int element_index)
{
	// Perform a simple linear search for a breach at this element, given that the collection will always be very small
	int n = m_breaches.size();
	for (int i = 0; i < n; ++i)
	{
		if (m_breaches[i].GetElementIndex() == element_index) return i;
	}

	return -1;
}



