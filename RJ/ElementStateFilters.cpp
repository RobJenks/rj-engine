#include "ComplexShipElement.h"
#include "ElementStateFilters.h"


// Filter that does not exclude any property types
ElementStateFilters::ElementStateFilter ElementStateFilters::ALL_PROPERTIES = ~((bitstring)0U);								// 111...111

// Defines the set of properties that can be set by complex ship sections
ElementStateFilters::ElementStateFilter ElementStateFilters::SECTION_PROPERTIES =
	(ComplexShipElement::PROPERTY::PROP_ACTIVE | ComplexShipElement::PROPERTY::PROP_BUILDABLE);								// Only core active & buildable properties

// Defines the set of properties that can be set by complex ship tiles
ElementStateFilters::ElementStateFilter ElementStateFilters::TILE_PROPERTIES = ~(ElementStateFilters::SECTION_PROPERTIES);	// Exact inverse of section properties

// Filter that excludes all property types
ElementStateFilters::ElementStateFilter ElementStateFilters::NO_PROPERTIES = (bitstring)0U;									// 000...000
