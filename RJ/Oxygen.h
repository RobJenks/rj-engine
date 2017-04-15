#pragma once


class Oxygen
{
public:

	// Base type used to represent oxygen levels
	typedef float Type;

	// Constants relating to oxygen simulation
	static const Oxygen::Type BASE_CONSUMPTION_PER_ELEMENT;		// Base oxygen loss per environment element /sec
	static const Oxygen::Type BASE_CONSUMPTION_PER_ACTOR;		// Base oxygen consumption per environment object /sec
	static const Oxygen::Type BASE_TRANSMISSION_LIMIT;			// Maximum volume of oxygen that can be transmitted between elements /sec
};


