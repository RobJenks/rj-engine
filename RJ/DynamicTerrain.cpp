#include "DynamicTerrainDefinition.h"
#include "DynamicTerrain.h"


// Default constructor
DynamicTerrain::DynamicTerrain(void)
	: 
	m_dynamic_terrain_def(NULL)
{
	// Enable relevant flags on this terrain object
	m_isdynamic = true;
	m_dataenabled = true;
	m_usable = true;
}

// Static method to instantiate a new dynamic terrain object based upon its string definition code.  Shortcut
// to avoid calling through the definition object itself
DynamicTerrain * DynamicTerrain::Create(const std::string & code)
{
	const DynamicTerrainDefinition *def = D::DynamicTerrainDefinitions.Get(code);
	if (def != NULL)
	{
		return def->Create();
	}

	return NULL;
}

// Set the state of this dynamic terrain object
void DynamicTerrain::SetState(const std::string & state)
{
	// Attempt to locate this state; quit immediately if it is not valid
	const DynamicTerrainDefinition *def = GetDynamicTerrainDefinition();	if (!def) return;
	const DynamicTerrainState *state_def = def->GetStateDefinition(state);	if (!state_def) return;

	// Apply the effects of this state on the terrain object
	ApplyState(state_def);
}

// Apply a particular state definition to this terrain object
void DynamicTerrain::ApplyState(const DynamicTerrainState *state)
{
	if (!state) return;

	// Update static terrain definition, if applicable
	if (state->HasAssignedStaticTerrain())
	{
		this->SetDefinition(state->GetAssignedStaticTerrain());
	}

	// Update ...

}

// Returns the terrain to its default state, if one is specified in the terrain definition
void DynamicTerrain::ReturnToDefaultState(void)
{
	const DynamicTerrainDefinition *def = GetDynamicTerrainDefinition();	
	if (!def) return;

	SetState(def->GetDefaultState());
}

// Invoke the default state transition from our current state, if one is defined
void DynamicTerrain::ExecuteDefaultStateTransition(void)
{
	const DynamicTerrainDefinition *def = GetDynamicTerrainDefinition();
	if (!def) return;

	std::string next_state = def->GetDefaultStateTransition(GetState());
	if (next_state != NullString) SetState(next_state);
}

// Event raised when an entity tries to interact with this object
bool DynamicTerrain::OnUsed(iObject *user)
{
	// Default behaviour if none is set by subclasses
	return false;
}