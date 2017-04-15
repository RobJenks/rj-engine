#pragma once

#ifndef __EnvironmentOxygenMapH__
#define __EnvironmentOxygenMapH__

#include "CompilerSettings.h"
#include "ObjectReference.h"
#include "Oxygen.h"
#include "EnvironmentMap.h"


class EnvironmentOxygenMap
{
public:

	// Types used to store oxygen levels throughout the environment
	typedef EnvironmentMap<Oxygen::Type, EnvironmentMapBlendMode::BlendAdditive>	OxygenMap;

	// Constructor
	EnvironmentOxygenMap(void) : EnvironmentOxygenMap(NULL) { }
	EnvironmentOxygenMap(iSpaceObjectEnvironment *environment);

	// Rebuilds the map, e.g. if the parent environment size/shape changes
	void																	RebuildMap(void);

	// Performs an update of the oxygen map for the specified time interval
	void																	Update(float timedelta);

	// Destructor
	~EnvironmentOxygenMap(void);

private:

	ObjectReference<iSpaceObjectEnvironment>								m_environment;
	OxygenMap																m_map;

	// Initialises the oxygen map
	void																	Initialise(void);

	// Returns the set of all oxygen sources in the environment
	void																	DetermineOxygenSources(float timedelta, std::vector<OxygenMap::MapCell> & outSources);

	// Determines the current oxygen consumption level (including background decline level)
	float																	DetermineOxygenConsumption(void);

};


#endif









