#pragma once

#include "Power.h"
#include "EnvironmentMap.h"
class iSpaceObjectEnvironment;

class EnvironmentPowerMap
{
public:

	// Type used to store power levels throughout the environment.   Uses a MaximumValue blend mode so that
	// if elements are connected to two power sources they will take & propogate the highest power level
	typedef EnvironmentMap<Power::Type, EnvironmentMapBlendMode::BlendMaximumValue>	PowerMap;

	// Constructor
	CMPINLINE EnvironmentPowerMap(void) : EnvironmentPowerMap(NULL) { }
	EnvironmentPowerMap(iSpaceObjectEnvironment *environment);

	// Initialises the power map
	void Initialise(void);

	// Returns the Power level for a specific element
	CMPINLINE Power::Type													GetPowerLevel(int element_id) const { return m_map.Data[element_id]; }
	CMPINLINE Power::Type													GetPowerLevel(const INTVECTOR3 & location) const
	{
		const INTVECTOR3 & size = GetSize();
		return GetPowerLevel(ELEMENT_INDEX_EX(location.x, location.y, location.z, size));
	}

	// Returns the size of the Power map
	CMPINLINE INTVECTOR3													GetSize(void) const { return m_map.GetMapSize(); }

	// Returns the set of all power sources in the environment
	void																	DeterminePowerSources(std::vector<PowerMap::MapCell> & outSources);

	// Revalidates the map against its underlying environment and attempts to update it to account for
	// any changes in the environment (e.g. structural changes).  Returns true in case of success.  Returns
	// false if the environment has changed too significantly and requires a full map rebuild
	bool																	RevalidateMap(void);

	// Performs an update of the power map for the specified time interval
	void																	Update(float timedelta);



private:

	ObjectReference<iSpaceObjectEnvironment>	m_environment;
	PowerMap									m_map;

};




