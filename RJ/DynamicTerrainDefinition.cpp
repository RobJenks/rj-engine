#include "DynamicTerrain.h"
#include "DynamicTerrainDefinition.h"

// Default constructor
DynamicTerrainDefinition::DynamicTerrainDefinition(void)
	:
	m_code(NullString), m_prototype(NULL)
{
}

// Create a new dynamic terrain object based upon this definition
DynamicTerrain * DynamicTerrainDefinition::Create(void) const
{
	// We must have a valid prototype
	if (m_prototype == NULL) return NULL;

	// Create a new instance based upon our prototype
	DynamicTerrain *instance = m_prototype->Clone();

	// Apply other post-instantiation changes as required

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



