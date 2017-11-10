#pragma once

#include "Terrain.h"
#include "DataEnabledObject.h"
#include "UsableObject.h"
#include "DynamicTerrainClassData.h"
class TerrainDefinition;
class DynamicTerrainDefinition;
struct DynamicTerrainState;


class DynamicTerrain : public Terrain, public DataEnabledObject, public UsableObject
{
public:

	// Static method to instantiate a new dynamic terrain object based upon its string definition code
	static DynamicTerrain *						Create(const std::string & code);


	// The dynamic terrain definition associated with this instance
	CMPINLINE const DynamicTerrainDefinition *	GetDynamicTerrainDefinition(void) const { return m_dynamic_terrain_def; }
	CMPINLINE void								SetDynamicTerrainDefinition(const DynamicTerrainDefinition *def) { m_dynamic_terrain_def = def; }

	// Event raised when an entity tries to interact with this object
	virtual bool								OnUsed(iObject *user);

	// Set a property of this dynamic terrain object; should be overridden by any subclasses that wish to use them
	CMPINLINE virtual void						SetProperty(const std::string & key, const std::string & value) { };

	// Return the state that this terrain object is currently in
	CMPINLINE std::string						GetState(void) const { return m_state; }

	// Set the state of this dynamic terrain object
	CMPINLINE void								SetState(const std::string & state);

	// Returns the terrain to its default state, if one is specified in the terrain definition
	void										ReturnToDefaultState(void);

	// Invoke the default state transition from our current state, if one is defined
	void										ExecuteDefaultStateTransition(void);


	// Pure virtual clone method for this object type; automatically implemented for subclasses
	// by the DYNAMIC_TERRAIN_CLASS macro
	virtual DynamicTerrain *					Clone(void) const = 0;


protected:

	// Pointer to the dynamic terrain definition for this object
	const DynamicTerrainDefinition *			m_dynamic_terrain_def;

	// ID of the state that this object is currently in
	std::string									m_state;

	// Default constructor
	DynamicTerrain(void);

	// Apply a particular state definition to this terrain object
	void										ApplyState(const DynamicTerrainState *state);


};
