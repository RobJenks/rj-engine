#include "Power.h"

// Constants relating to power simulation
const ComplexShipElement::PROPERTY Power::POWER_TRANSMISSION_PROPERTY = ComplexShipElement::PROPERTY::PROP_TRANSMITS_POWER;
const bitstring Power::POWER_BLOCKING_PROPERTIES = (bitstring)ComplexShipElement::NULL_PROPERTIES;

unsigned int Power::POWER_UPDATE_INTERVAL_FULL_SIMULATION = 1000U;				// ms // TODO: Make const after debugging
const unsigned int Power::POWER_UPDATE_INTERVAL_TACTICAL_SIMULATION = 5000U;	// ms
const unsigned int Power::POWER_UPDATE_INTERVAL_STRATEGIC_SIMULATION = 20000U;	// ms
const unsigned int Power::POWER_UPDATE_INTERVAL_NO_SIMULATION = 60000U;			// ms


// Returns the oxygen update interval for a specific simulation state
unsigned int Power::GetPowerUpdateInterval(iObject::ObjectSimulationState simulation_state)
{
	// TODO: Can make this a lookup array in future
	switch (simulation_state)
	{
		case iObject::ObjectSimulationState::FullSimulation:		return Power::POWER_UPDATE_INTERVAL_FULL_SIMULATION;
		case iObject::ObjectSimulationState::TacticalSimulation:	return Power::POWER_UPDATE_INTERVAL_TACTICAL_SIMULATION;
		case iObject::ObjectSimulationState::StrategicSimulation:	return Power::POWER_UPDATE_INTERVAL_STRATEGIC_SIMULATION;
		default:													return Power::POWER_UPDATE_INTERVAL_NO_SIMULATION;
	}
}
