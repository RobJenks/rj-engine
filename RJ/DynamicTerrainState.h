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

	// Numeric ID that can be assigned to the state if required, but is not mandatory.  Default zero
	CMPINLINE int										GetStateID(void) const { return m_state_id; }
	CMPINLINE void										SetStateID(int id) { m_state_id = id; }

	// State definition can set the terrain model
	void												AssignStateStaticTerrain(const std::string & static_terrain_definition);
	CMPINLINE bool										HasAssignedStaticTerrain(void) const { return m_sets_static_terrain_def; }
	CMPINLINE std::string								GetAssignedStaticTerrain(void) const { return m_static_terrain_def; }

	// Default destructor
	~DynamicTerrainState(void);

private:

	// String identifier for this state
	std::string											m_state_code;

	// Numeric ID that can be assigned to the state if required, but is not mandatory.  Default zero
	int													m_state_id;

	// Properties that may be set by the state, including a flag where required to indicate that the 
	// property is being set (e.g. to null, and not just at default of null)
	bool												m_sets_static_terrain_def;
	std::string											m_static_terrain_def;

};
