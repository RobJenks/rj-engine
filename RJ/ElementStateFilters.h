#pragma once

#ifndef __ElementStateFilterH__
#define __ElementStateFilterH__

#include "Utility.h"

class ElementStateFilters
{

public:

	typedef bitstring ElementStateFilter;

	// Filter that does not exclude any property types
	static ElementStateFilter ALL_PROPERTIES;

	// Defines the set of properties that can be set by complex ship sections
	static ElementStateFilter SECTION_PROPERTIES;

	// Defines the set of properties that can be set by complex ship tiles
	static ElementStateFilter TILE_PROPERTIES;

	// Filter that excludes all property types
	static ElementStateFilter NO_PROPERTIES;
};




#endif