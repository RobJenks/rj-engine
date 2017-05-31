#pragma once

#include "Utility.h"
#include "ComplexShipElement.h"

class Power
{
public:

	// Base type used to represent power state.  State is only ever either 0 or 1
	typedef int Type;

	// Constants relating to power simulation
	static const ComplexShipElement::PROPERTY POWER_TRANSMISSION_PROPERTY;					// We only use a single property for efficiency
	static const bitstring POWER_BLOCKING_PROPERTIES;										// Can be a combination of properties if required

	static unsigned int POWER_UPDATE_INTERVAL_FULL_SIMULATION;
	static const unsigned int POWER_UPDATE_INTERVAL_TACTICAL_SIMULATION;
	static const unsigned int POWER_UPDATE_INTERVAL_STRATEGIC_SIMULATION;
	static const unsigned int POWER_UPDATE_INTERVAL_NO_SIMULATION;

	// Returns the power update interval for a specific simulation state
	static unsigned int GetPowerUpdateInterval(iObject::ObjectSimulationState simulation_state);
};