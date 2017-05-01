#include "iObject.h"
#include "Oxygen.h"

// Initialise static class constants
const Oxygen::Type Oxygen::BASE_CONSUMPTION_PER_ELEMENT = (Oxygen::Type)0.6f;
const Oxygen::Type Oxygen::BASE_CONSUMPTION_PER_ACTOR = (Oxygen::Type)2.0f;
const Oxygen::Type Oxygen::BASE_TRANSMISSION_LIMIT = (Oxygen::Type)2.0f;
const Oxygen::Type Oxygen::BASE_OXYGEN_FALLOFF = (Oxygen::Type)1.0f;

const unsigned int Oxygen::OXYGEN_UPDATE_INTERVAL_FULL_SIMULATION = 1000U;			// ms
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