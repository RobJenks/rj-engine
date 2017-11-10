#include "Utility.h"
#include "DynamicTerrainState.h"

// Default constructor
DynamicTerrainState::DynamicTerrainState(void)
	:
	m_state_code(NullString), m_state_id(0), 
	m_static_terrain_def(NullString), m_sets_static_terrain_def(false)
{
}


// Assign a model to this state definition
void DynamicTerrainState::AssignStateStaticTerrain(const std::string & static_terrain_definition)
{
	m_static_terrain_def = static_terrain_definition;
	m_sets_static_terrain_def = true;
}



// Default destructor
DynamicTerrainState::~DynamicTerrainState(void)
{
}