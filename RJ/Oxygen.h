#pragma once

#include "ComplexShipElement.h"
#include "TileConnections.h"
#include "iObject.h"

class Oxygen
{
public:

	// Base type used to represent oxygen levels
	typedef float Type;

	// Constants relating to oxygen simulation
	static const Oxygen::Type BASE_CONSUMPTION_PER_ELEMENT;		// Base oxygen loss per environment element /sec
	static const Oxygen::Type BASE_CONSUMPTION_PER_ACTOR;		// Base oxygen consumption per environment object /sec
	static Oxygen::Type BASE_TRANSMISSION_LIMIT;				// Maximum volume of oxygen that can be transmitted between elements /sec
	static Oxygen::Type BASE_OXYGEN_FALLOFF;					// Base oxygen falloff per second

	static const ComplexShipElement::PROPERTY OXYGEN_TRANSMISSION_PROPERTY;					// We only use a single property for efficiency
	static const TileConnections::TileConnectionType OXYGEN_TILE_TRANSMISSION_PROPERTY;		// Tile connections (currently) use a different property set
	static const bitstring OXYGEN_BLOCKING_PROPERTIES;										// Can be a combination of properties if required

	static unsigned int OXYGEN_UPDATE_INTERVAL_FULL_SIMULATION;
	static const unsigned int OXYGEN_UPDATE_INTERVAL_TACTICAL_SIMULATION;
	static const unsigned int OXYGEN_UPDATE_INTERVAL_STRATEGIC_SIMULATION;
	static const unsigned int OXYGEN_UPDATE_INTERVAL_NO_SIMULATION;

	// Returns the oxygen update interval for a specific simulation state
	static unsigned int GetOxygenUpdateInterval(iObject::ObjectSimulationState simulation_state);
};


