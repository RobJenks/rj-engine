#include "iObject.h"
#include "ComplexShipElement.h"
#include "TileConnections.h"
#include "Oxygen.h"

// Initialise static class constants
const Oxygen::Type Oxygen::BASE_CONSUMPTION_PER_ELEMENT = (Oxygen::Type)0.005f;
const Oxygen::Type Oxygen::BASE_CONSUMPTION_PER_ACTOR = (Oxygen::Type)0.05f;
Oxygen::Type Oxygen::BASE_TRANSMISSION_LIMIT = (Oxygen::Type)2.0f;		// TODO: Make const after debugging
Oxygen::Type Oxygen::BASE_OXYGEN_FALLOFF = (Oxygen::Type)0.05f;			// TODO: Make const after debugging

const ComplexShipElement::PROPERTY Oxygen::OXYGEN_TRANSMISSION_PROPERTY = ComplexShipElement::PROPERTY::PROP_WALKABLE;
const TileConnections::TileConnectionType Oxygen::OXYGEN_TILE_TRANSMISSION_PROPERTY = TileConnections::TileConnectionType::Walkable;

const bitstring Oxygen::OXYGEN_BLOCKING_PROPERTIES = (bitstring)ComplexShipElement::NULL_PROPERTIES;		// None at this point


unsigned int Oxygen::OXYGEN_UPDATE_INTERVAL_FULL_SIMULATION = 1000U;				// ms // TODO: Make const after debugging
const unsigned int Oxygen::OXYGEN_UPDATE_INTERVAL_TACTICAL_SIMULATION = 5000U;		// ms
const unsigned int Oxygen::OXYGEN_UPDATE_INTERVAL_STRATEGIC_SIMULATION = 20000U;	// ms
const unsigned int Oxygen::OXYGEN_UPDATE_INTERVAL_NO_SIMULATION = 60000U;			// ms


// Returns the oxygen update interval for a specific simulation state
unsigned int Oxygen::GetOxygenUpdateInterval(iObject::ObjectSimulationState simulation_state)
{
	// TODO: Can make this a lookup array in future
	switch (simulation_state)
	{
		case iObject::ObjectSimulationState::FullSimulation:		return Oxygen::OXYGEN_UPDATE_INTERVAL_FULL_SIMULATION;
		case iObject::ObjectSimulationState::TacticalSimulation:	return Oxygen::OXYGEN_UPDATE_INTERVAL_TACTICAL_SIMULATION;
		case iObject::ObjectSimulationState::StrategicSimulation:	return Oxygen::OXYGEN_UPDATE_INTERVAL_STRATEGIC_SIMULATION;
		default:													return Oxygen::OXYGEN_UPDATE_INTERVAL_NO_SIMULATION;
	}
}