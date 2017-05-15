#pragma once

#include "CompilerSettings.h"
#include "Direction.h"


class EnvironmentHullBreachDetails
{
public:

	CMPINLINE EnvironmentHullBreachDetails(void) : m_target_element(-1), m_adjacency_direction(Direction::_Count) { }

	CMPINLINE EnvironmentHullBreachDetails(int target_element, Direction adjacency_direction) :
		m_target_element(target_element), m_adjacency_direction(adjacency_direction) { }

	CMPINLINE int GetTargetElement(void) const { return m_target_element; }
	CMPINLINE Direction GetTargetDirection(void) const { return m_adjacency_direction; }


private:
	int									m_target_element;
	Direction							m_adjacency_direction;

};

