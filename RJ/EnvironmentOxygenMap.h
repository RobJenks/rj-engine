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
	typedef EnvironmentMap<Oxygen::Type, EnvironmentMapBlendMode::BlendAveraged>	OxygenMap;

	// Constructor
	EnvironmentOxygenMap(void) : EnvironmentOxygenMap(NULL) { }
	EnvironmentOxygenMap(iSpaceObjectEnvironment *environment);

	// Rebuilds the map, e.g. if the parent environment size/shape changes
	void																	RebuildMap(void);

	// Performs an update of the oxygen map for the specified time interval
	void																	Update(float timedelta);

	// Returns the oxygen level for a specific element
	CMPINLINE Oxygen::Type													GetOxygenLevel(int element_id) const { return m_map.Data[element_id]; }
	CMPINLINE Oxygen::Type													GetOxygenLevel(const INTVECTOR3 & location) const
	{
		const INTVECTOR3 & size = GetSize();
		return GetOxygenLevel(ELEMENT_INDEX_EX(location.x, location.y, location.z, size));
	}

	// Returns the size of the oxygen map
	CMPINLINE INTVECTOR3													GetSize(void) const { return m_map.GetMapSize(); }

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









