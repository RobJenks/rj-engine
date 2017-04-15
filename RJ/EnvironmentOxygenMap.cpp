#include "Oxygen.h"
#include "iSpaceObjectEnvironment.h"
#include "ComplexShipElement.h"
#include "iContainsComplexShipTiles.h"
#include "CSLifeSupportTile.h"

#include "EnvironmentOxygenMap.h"


// Constructor
EnvironmentOxygenMap::EnvironmentOxygenMap(iSpaceObjectEnvironment *environment) 
	: 
	m_environment(environment), 
	m_map(environment != NULL ? environment->GetElementSize() : ONE_INTVECTOR3)
{

}

// Initialises the oxygen map
void EnvironmentOxygenMap::Initialise(void)
{
	// Ensure our parent environment is still valid
	if (!m_environment()) return;

	// Initialise the map to a default starting state
	m_map.InitialiseCellValues(DefaultValues<Oxygen::Type>::NullValue());
	m_map.SetValueConstraints((Oxygen::Type)0, (Oxygen::Type)100);
	m_map.SetTransmissionProperties(ComplexShipElement::PROPERTY::PROP_WALKABLE);
	m_map.SetZeroThreshold((Oxygen::Type)0);
	m_map.SetFalloffMethod(EnvironmentMapFalloffMethod<Oxygen::Type>::EnvironmentMapFalloffMethod()
		.WithAbsoluteFalloff((Oxygen::Type)1)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<Oxygen::Type>::FalloffTransmissionType::Distance));
	m_map.SetEmissionBehaviour(OxygenMap::EmissionBehaviour::EmissionRemovedFromSource);
}

// Rebuilds the map, e.g. if the parent environment size/shape changes
void EnvironmentOxygenMap::RebuildMap(void)
{
	// Ensure our parent environment is still valid
	if (!m_environment()) return;

	// Recreate the underlying map
	m_map = OxygenMap(m_environment()->GetElementSize());

	// Initialise to default state
	Initialise();
}


// Performs an update of the oxygen map for the specified time interval
void EnvironmentOxygenMap::Update(float timedelta)
{
	// Parameter checks
	iSpaceObjectEnvironment *env = m_environment();
	if (!env || timedelta <= 0.0f) return;

	// Retrieve required information from the environment
	std::vector<OxygenMap::MapCell> sources;
	DetermineOxygenSources(timedelta, sources);
	float consumption = DetermineOxygenConsumption();

	// Initiate an update of the underlying map
	m_map
		.BeginUpdate()
		.WithPreserveExistingData()
		.WithAdditiveModifierToExistingData(-consumption * timedelta)
		.WithTransferLimit(Oxygen::BASE_TRANSMISSION_LIMIT)
		.WithSourceCells(sources)
		.Execute(env->GetElements());
}


// Returns the set of all oxygen sources in the environment
void EnvironmentOxygenMap::DetermineOxygenSources(float timedelta, std::vector<OxygenMap::MapCell> & outSources)
{
	// Locate all oxygen generator tiles in the environment
	iSpaceObjectEnvironment *env = m_environment();
	iContainsComplexShipTiles::ComplexShipTileCollection tiles = env->GetTilesOfType(D::TileClass::LifeSupport);
	iContainsComplexShipTiles::ComplexShipTileCollection::const_iterator it_end = tiles.end();
	for (iContainsComplexShipTiles::ComplexShipTileCollection::const_iterator it = tiles.begin(); it != it_end; ++it)
	{
		// For now, the source will be considered to emit from local (0,0,0) within the tile.
		// TODO: add an offset here in future to allow multi-tile life support tiles which emit from a specific local element
		CSLifeSupportTile *tile = (CSLifeSupportTile*)(*it).value;
		outSources.push_back(OxygenMap::MapCell(
			env->GetElementIndex(tile->GetElementLocation()),
			(tile->OxygenLevel.Value * timedelta)));
	}
}

// Determines the current oxygen consumption level (including background decline level)
float EnvironmentOxygenMap::DetermineOxygenConsumption(void)
{
	// Base consumption is a fixed rate per WALKABLE environment element
	iSpaceObjectEnvironment *env = m_environment();
	float consumption	= env->ElementsWithProperty(ComplexShipElement::PROPERTY::PROP_WALKABLE)
						* Oxygen::BASE_CONSUMPTION_PER_ELEMENT;

	// Add the per-actor consumption.  TODO: currently assumes all actors consume oxygen, and
	// that all actors consume the same amount
	consumption += (env->GetEnvironmentObjectCount() * Oxygen::BASE_CONSUMPTION_PER_ACTOR);

	// Return total calculated consumption
	return consumption;
}

// Destructor
EnvironmentOxygenMap::~EnvironmentOxygenMap(void)
{

}



