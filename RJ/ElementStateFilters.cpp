#include "ComplexShipElement.h"
#include "ElementStateFilters.h"


// Filter that excludes all property types
ElementStateFilters::ElementStateFilter ElementStateFilters::NO_PROPERTIES = (bitstring)0U;									// 000...000

// Filter that does not exclude any property types
ElementStateFilters::ElementStateFilter ElementStateFilters::ALL_PROPERTIES = ~(ElementStateFilters::NO_PROPERTIES);		// 111...111



// Defines the set of properties that are automatically-defined, e.g. by the element state or other properties
ElementStateFilters::ElementStateFilter ElementStateFilters::AUTO_PROPERTIES = (
	ComplexShipElement::PROPERTY::PROP_DESTROYED);

// Defines the set of properties that can be set by complex ship sections
ElementStateFilters::ElementStateFilter ElementStateFilters::SECTION_PROPERTIES = (
	ComplexShipElement::PROPERTY::PROP_ACTIVE |
	ComplexShipElement::PROPERTY::PROP_BUILDABLE);

// Defines the set of properties that can be set by complex ship tiles
ElementStateFilters::ElementStateFilter ElementStateFilters::TILE_PROPERTIES = (
	ComplexShipElement::PROP_WALKABLE | 
	ComplexShipElement::PROPERTY::PROP_TRANSMITS_POWER | 
	ComplexShipElement::PROPERTY::PROP_TRANSMITS_DATA);

