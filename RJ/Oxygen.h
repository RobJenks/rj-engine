#pragma once

#include "iObject.h"

class Oxygen
{
public:

	// Base type used to represent oxygen levels
	typedef float Type;

	// Constants relating to oxygen simulation
	static const Oxygen::Type BASE_CONSUMPTION_PER_ELEMENT;		// Base oxygen loss per environment element /sec
	static const Oxygen::Type BASE_CONSUMPTION_PER_ACTOR;		// Base oxygen consumption per environment object /sec
	static const Oxygen::Type BASE_TRANSMISSION_LIMIT;			// Maximum volume of oxygen that can be transmitted between elements /sec
	static const Oxygen::Type BASE_OXYGEN_FALLOFF;				// Base oxygen falloff per second

	static const unsigned int OXYGEN_UPDATE_INTERVAL_FULL_SIMULATION;
	static const unsigned int OXYGEN_UPDATE_INTERVAL_TACTICAL_SIMULATION;
	static const unsigned int OXYGEN_UPDATE_INTERVAL_STRATEGIC_SIMULATION;
	static const unsigned int OXYGEN_UPDATE_INTERVAL_NO_SIMULATION;

	// Returns the oxygen update interval for a specific simulation state
	static unsigned int GetOxygenUpdateInterval(iObject::ObjectSimulationState simulation_state);
};


