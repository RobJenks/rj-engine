#pragma once 

#include <vector>
#include "CompilerSettings.h"
#include "Direction.h"
#include "EnvironmentHullBreach.h"
#include "EnvironmentHullBreachDetails.h"


class EnvironmentHullBreaches
{
public:

	// Custom collection types
	typedef std::vector<EnvironmentHullBreach>						collection_type;
	typedef std::vector<EnvironmentHullBreach>::size_type			size_type;

	// Constructor
	EnvironmentHullBreaches();

	// Record a new hull breach from 'element_index' in the direction of 'direction' towards destroyed element 'destroyed_element'
	void RecordHullBreach(int element_index, Direction direction, int destroyed_element);

	// Reset the record of all hull breaches
	void Reset(void);

	// Return the collection of all hull breaches
	CMPINLINE std::vector<EnvironmentHullBreach> &					Items(void) { return m_breaches; }

	// Returns a reference to a particular breach
	EnvironmentHullBreach &											Get(EnvironmentHullBreaches::size_type breach_index);

	// Returns the number of hull breaches that are currently active
	CMPINLINE EnvironmentHullBreaches::size_type					GetBreachCount(void) const { return m_breaches.size(); }

	// Determines whether a breach is present at the given location, and returns the breach_index if there is
	// Returns -1 if there is no breach at the specified element
	int																GetBreachAtLocation(int element_index);

private:

	std::vector<EnvironmentHullBreach>								m_breaches;

};



