#pragma once

#include <string>


struct DynamicTerrainState
{
public:

	// Default constructor
	DynamicTerrainState(void);

	// String identifier for the state
	CMPINLINE std::string								GetStateCode(void) const { return m_state_code; }
	CMPINLINE void										SetStateCode(const std::string & code) { m_state_code = code; }

	// State definition can set the terrain model
	void												AssignStateModel(const std::string & model);
	CMPINLINE bool										HasAssignedModel(void) const { return m_sets_model; }
	CMPINLINE std::string								GetAssignedModelCode(void) const { return m_model_code; }

	// Default destructor
	~DynamicTerrainState(void);

private:

	// String identifier for this state
	std::string											m_state_code;

	// Properties that may be set by the state, including a flag where required to indicate that the 
	// property is being set (e.g. to null, and not just at default of null)
	bool												m_sets_model;
	std::string											m_model_code;

};
