#include "iSpaceObjectEnvironment.h"
#include "EnvironmentPowerMap.h"



// Constructor
EnvironmentPowerMap::EnvironmentPowerMap(iSpaceObjectEnvironment *environment)	
	:
	m_environment(environment),
	m_map(environment != NULL ? environment->GetElementSize() : ONE_INTVECTOR3)
{
}

