#pragma once

#include "Power.h"
#include "EnvironmentMap.h"
class iSpaceObjectEnvironment;

class EnvironmentPowerMap
{
public:

	// Type used to store power levels throughout the environment
	typedef EnvironmentMap<Power::Type, EnvironmentMapBlendMode::BlendReplaceDestination>	PowerMap;

	// Constructor
	CMPINLINE EnvironmentPowerMap(void) : EnvironmentPowerMap(NULL) { }
	EnvironmentPowerMap(iSpaceObjectEnvironment *environment);


private:

	iSpaceObjectEnvironment *					m_environment;
	PowerMap									m_map;

};




