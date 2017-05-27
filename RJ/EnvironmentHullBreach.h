#pragma once

#include "CompilerSettings.h"
#include "Direction.h"
#include "EnvironmentHullBreachDetails.h"

class EnvironmentHullBreach
{
public:

	// Constructors
	EnvironmentHullBreach(void);
	EnvironmentHullBreach(int element_index);

	CMPINLINE int											GetElementIndex(void) const { return m_element_index; }
	CMPINLINE int											GetBreachCount(void) const { return m_breach_count; }
	CMPINLINE EnvironmentHullBreachDetails &				GetBreachDetails(int index) { return m_breaches[index]; }
	CMPINLINE EnvironmentHullBreachDetails *				GetBreachDetails(void) { return m_breaches; }

	// Returns the index of a breach to the specified element, if one exists, or -1 if not
	int														GetBreachIndexToElement(int target_element) const;

	// Indicates whether there is a breach to the specified element
	CMPINLINE bool											HaveBreachToElement(int target_element) const { return (GetBreachIndexToElement(target_element) != -1); }

	// Returns the index of a breach in the specified direction, if one exists, or -1 if not
	int														GetBreachInDirection(Direction direction) const;

	// Indicates whether there is a breach in the specified direction
	CMPINLINE bool											HaveBreachInDirection(Direction direction) const { return (GetBreachInDirection(direction) != -1); }

	// Record a new breach at this location
	void													AddBreach(const EnvironmentHullBreachDetails & details);


private:

	int m_element_index;									// Index of this element
	int m_breach_count;										// Number of breaches at this location

	// Collection of breach data to adjacent elements
	// TODO: this can be optimised to take less space; not all Direction:: values (in fact, only the 90-degree entries) are valid breach directions
	static const int										MAX_BREACH_COUNT = (int)Direction::_Count;
	EnvironmentHullBreachDetails							m_breaches[MAX_BREACH_COUNT];

};


