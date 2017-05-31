#include "iSpaceObjectEnvironment.h"
#include "ComplexShipTile.h"
#include "EnvironmentPowerMap.h"



// Constructor
EnvironmentPowerMap::EnvironmentPowerMap(iSpaceObjectEnvironment *environment)	
	:
	m_environment(environment),
	m_map(environment != NULL ? environment->GetElementSize() : ONE_INTVECTOR3)
{
	Initialise();
}

// Initialises the power map
void EnvironmentPowerMap::Initialise(void)
{
	// Ensure our parent environment is still valid
	if (!m_environment()) return;

	// Initialise the map to a default starting state
	m_map.InitialiseCellValues(DefaultValues<Power::Type>::NullValue());
	m_map.SetTransmissionProperties((bitstring)Power::POWER_TRANSMISSION_PROPERTY);
	m_map.SetBlockingProperties(Power::POWER_BLOCKING_PROPERTIES);
	m_map.SetZeroThreshold((Power::Type)0);
	m_map.SetEmissionBehaviour(PowerMap::EmissionBehaviour::EmissionRemainsInSource);
}


// Returns the set of all power sources in the environment
void EnvironmentPowerMap::DeterminePowerSources(std::vector<PowerMap::MapCell> & outSources)
{
	// Locate all power generator tiles in the environment
	iSpaceObjectEnvironment *env = m_environment();
	iContainsComplexShipTiles::ComplexShipTileCollection tiles = env->GetTilesOfType(D::TileClass::Power);
	iContainsComplexShipTiles::ComplexShipTileCollection::const_iterator it_end = tiles.end();
	for (iContainsComplexShipTiles::ComplexShipTileCollection::const_iterator it = tiles.begin(); it != it_end; ++it)
	{
		// For now, the source will be considered to emit from local (0,0,0) within the tile.
		// TODO: add an offset here in future to allow multi-tile power generation tiles which emit from a specific local element
		outSources.push_back(PowerMap::MapCell(
			env->GetElementIndex((*it).value->GetElementLocation())));
	}
}

// Revalidates the map against its underlying environment and attempts to update it to account for
// any changes in the environment (e.g. structural changes).  Returns true in case of success.  Returns
// false if the environment has changed too significantly and requires a full map rebuild
bool EnvironmentPowerMap::RevalidateMap(void)
{
	// Power map does not maintain state between updates.  As a result, there is no benefit in perfoming an 
	// incremental rebuild.  Simply return false so that a full build is performed, at which point the full 
	// power state will be recalculated
	return false;
}



