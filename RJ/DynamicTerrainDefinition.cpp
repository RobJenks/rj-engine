#include "DynamicTerrain.h"
#include "DynamicTerrainDefinition.h"

// Default constructor
DynamicTerrainDefinition::DynamicTerrainDefinition(void)
	:
	m_code(NullString), m_prototype(NULL), m_default_state(NullString)
{
}

// Create a new dynamic terrain object based upon this definition
DynamicTerrain * DynamicTerrainDefinition::Create(void) const
{
	// We must have a valid prototype
	if (m_prototype == NULL) return NULL;

	// Create a new instance based upon our prototype
	DynamicTerrain *instance = m_prototype->Clone();

	// If the terrain has a default starting state then apply it now
	instance->ReturnToDefaultState();

	// Return the new instance
	return instance;
}

// Assign a new state definition to this terrain 
Result DynamicTerrainDefinition::AssignStateDefinition(const DynamicTerrainState & state_definition)
{
	// Validate parameter
	if (state_definition.GetStateCode().empty()) return ErrorCodes::CannotAssignDynamicTerrainStateWithNoCode;

	// Make sure a definition has not already been provided for this state
	if (m_states.find(state_definition.GetStateCode()) != m_states.end())
	{
		return ErrorCodes::CannotAssignDuplicateStateDefToDynamicTerrain;
	}

	// Store the new state definition and return success
	m_states[state_definition.GetStateCode()] = state_definition;
	return ErrorCodes::NoError;
}

// Return details for the given state, or NULL if no such state is defined.  Pointer is valid only
// at the current time and should not be retained for future use
const DynamicTerrainState * DynamicTerrainDefinition::GetStateDefinition(const std::string & state) const
{
	auto it = m_states.find(state);
	return (it != m_states.end() ? &((*it).second) : NULL);
}

// Add a default state transition that can be applied by the object
void DynamicTerrainDefinition::AddDefaultStateTransition(const std::string & state, const std::string next_state)
{
	// We record the state transition whether or not we have matching states, since the transitions
	// may be loaded before the states themselves.  We instead validate when looking up a transition
	// We should however make sure there are no duplicate transitions defined; every state must have
	// at most one default transition defined from it
	if (m_default_state_transitions.find(state) == m_default_state_transitions.end())
	{
		m_default_state_transitions[state] = next_state;
	}
}

// Return the default state transition for an object of this type, given the specified current state.  Returns 
// an empty string if no transition is defined (the empty string is not a valid code for a state definition)
std::string DynamicTerrainDefinition::GetDefaultStateTransition(const std::string & current_state) const
{
	// Make sure this is actually a valid state (whether or not we have a transition defined)
	if (m_states.find(current_state) == m_states.end()) return NullString;

	// Attempt to find a matching transition
	auto it = m_default_state_transitions.find(current_state);
	return (it != m_default_state_transitions.end() ? it->second : NullString);
}




// Default destructor
DynamicTerrainDefinition::~DynamicTerrainDefinition(void)
{
	// The parent object (generally an environment) is always in charge of deallocating the static/dynamic terrain objects 
	// which it owns.  In this case however we have a standalone terrain object being used as a prototype which belongs
	// to us, and not any definition.  We therefore deallocate the prototype terrain object when this definition is deallocated
	if (m_prototype)
	{
		SafeDelete(m_prototype);
	}
}



