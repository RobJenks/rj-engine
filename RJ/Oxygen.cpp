#include "iObject.h"
#include "ComplexShipElement.h"
#include "Oxygen.h"

// Initialise static class constants
const Oxygen::Type Oxygen::BASE_CONSUMPTION_PER_ELEMENT = (Oxygen::Type)0.005f * 0;
const Oxygen::Type Oxygen::BASE_CONSUMPTION_PER_ACTOR = (Oxygen::Type)0.05f * 0;
Oxygen::Type Oxygen::BASE_TRANSMISSION_LIMIT = (Oxygen::Type)1.0f;		// TODO: Make const after debugging
Oxygen::Type Oxygen::BASE_OXYGEN_FALLOFF = (Oxygen::Type)0.25f;			// TODO: Make const after debugging

const ComplexShipElement::PROPERTY Oxygen::OXYGEN_TRANSMISSION_PROPERTY = ComplexShipElement::PROPERTY::PROP_ACTIVE;
const bitstring Oxygen::OXYGEN_BLOCKING_PROPERTIES = 0U;

unsigned int Oxygen::OXYGEN_UPDATE_INTERVAL_FULL_SIMULATION = 1000U;			// ms // TODO: Make const after debugging
const unsigned int Oxygen::OXYGEN_UPDATE_INTERVAL_TACTICAL_SIMULATION = 5000U;		// ms
const unsigned int Oxygen::OXYGEN_UPDATE_INTERVAL_STRATEGIC_SIMULATION = 20000U;	// ms
const unsigned int Oxygen::OXYGEN_UPDATE_INTERVAL_NO_SIMULATION = 60000U;			// ms


// Returns the oxygen update interval for a specific simulation state
unsigned int Oxygen::GetOxygenUpdateInterval(iObject::ObjectSimulationState simulation_state)
{
	switch (simulation_state)
	{
		case iObject::ObjectSimulationState::FullSimulation:		return Oxygen::OXYGEN_UPDATE_INTERVAL_FULL_SIMULATION;
		case iObject::ObjectSimulationState::TacticalSimulation:	return Oxygen::OXYGEN_UPDATE_INTERVAL_TACTICAL_SIMULATION;
		case iObject::ObjectSimulationState::StrategicSimulation:	return Oxygen::OXYGEN_UPDATE_INTERVAL_STRATEGIC_SIMULATION;
		default:													return Oxygen::OXYGEN_UPDATE_INTERVAL_NO_SIMULATION;
	}
}