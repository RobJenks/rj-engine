#include "Utility.h"
#include "DynamicTerrainState.h"

// Default constructor
DynamicTerrainState::DynamicTerrainState(void)
	:
	m_state_code(NullString), 
	m_model_code(NullString), m_sets_model(false)
{
}


// Assign a model to this state definition
void DynamicTerrainState::AssignStateModel(const std::string & model)
{
	m_model_code = model;
	m_sets_model = true;
}



// Default destructor
DynamicTerrainState::~DynamicTerrainState(void)
{
}