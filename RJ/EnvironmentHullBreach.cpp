#include <assert.h>
#include "EnvironmentHullBreach.h"


// Constructor
EnvironmentHullBreach::EnvironmentHullBreach(void)
	: m_element_index(-1), m_breach_count(0)
{
}

// Constructor
EnvironmentHullBreach::EnvironmentHullBreach(int element_index)
	: m_element_index(element_index), m_breach_count(0)
{
}

// Record a new breach at this location
void EnvironmentHullBreach::AddBreach(const EnvironmentHullBreachDetails & details)
{
	m_breaches[m_breach_count++] = details;
	assert(m_breach_count < EnvironmentHullBreach::MAX_BREACH_COUNT);
}


// Returns the index of a breach to the specified element, if one exists, or -1 if not
int	EnvironmentHullBreach::GetBreachIndexToElement(int target_element) const
{
	// Perform a simple linear search since the collection will be tiny
	for (int i = 0; i < m_breach_count; ++i)
	{
		if (m_breaches[i].GetTargetElement() == target_element) return i;
	}

	return -1;
}

// Returns the index of a breach in the specified direction, if one exists, or -1 if not
int EnvironmentHullBreach::GetBreachInDirection(Direction direction) const
{
	// Perform a simple linear search since the collection will be tiny
	for (int i = 0; i < m_breach_count; ++i)
	{
		if (m_breaches[i].GetTargetDirection() == direction) return i;
	}

	return -1;
}






