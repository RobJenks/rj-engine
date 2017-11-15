#pragma once

#include <string>
#include <unordered_map>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "DynamicTerrainState.h"
#include "PlayerInteractionType.h"
class DynamicTerrain;

class DynamicTerrainDefinition
{
public:

	// Default constructor
	DynamicTerrainDefinition(void);

	// Unique string code of the dynamic terrain definition
	CMPINLINE std::string					GetCode(void) const { return m_code; }
	CMPINLINE void							SetCode(const std::string & code) { m_code = code; }

	// Create a new dynamic terrain object based upon this definition
	DynamicTerrain *						Create(void) const;

	// Prototype terrain object used for instantiation
	CMPINLINE const DynamicTerrain *		GetPrototype(void) const { return m_prototype; }
	CMPINLINE void							SetPrototype(DynamicTerrain *prototype) { m_prototype = prototype; }

	// Assign a new state definition to this terrain 
	Result									AssignStateDefinition(const DynamicTerrainState & state_definition);

	// Default state for the terrain object (if applicable)
	CMPINLINE std::string					GetDefaultState(void) const { return m_default_state; }
	CMPINLINE void							SetDefaultState(const std::string & default_state) { m_default_state = default_state; }

	// Return details for the given state, or NULL if no such state is defined.  Pointer is valid only
	// at the current time and should not be retained for future use
	const DynamicTerrainState *				GetStateDefinition(const std::string & state) const;

	// Add a default state transition that can be applied by the object
	void									AddDefaultStateTransition(const std::string & state, const std::string next_state);
	
	// Return the default state transition for an object of this type, given the specified current state.  Returns 
	// an empty string if no transition is defined (the empty string is not a valid code for a state definition)
	std::string								GetDefaultStateTransition(const std::string & current_state) const;

	// The type of player interaction that is permitted by this object type
	CMPINLINE PlayerInteractionType			GetPermittedInteractionType(void) const { return m_permitted_interaction_type; }
	CMPINLINE void							SetPermittedInteractionType(PlayerInteractionType interaction_type) { m_permitted_interaction_type = interaction_type; }

	// Explicit shutdown method is not required for definition objects; all deallocation is performed in the destructor
	CMPINLINE void							Shutdown(void) { }

	// Default destructor
	~DynamicTerrainDefinition(void);

protected:

	// Unique string code of the dynamic terrain definition
	std::string											m_code;

	// Prototype terrain object used for instantiation
	DynamicTerrain *									m_prototype;

	// Set of states that the dynamic terrain object can be in, with associated changes to the terrain object itself
	std::unordered_map<std::string, DynamicTerrainState> m_states;

	// Default state associated with the terrain (if applicable)
	std::string											m_default_state;

	// Set of default state transitions that can be applied by the object
	std::unordered_map<std::string, std::string>		m_default_state_transitions;

	// The type of player interaction that is permitted by this object type
	PlayerInteractionType								m_permitted_interaction_type;

};