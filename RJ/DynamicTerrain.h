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
	static DynamicTerrain *							Create(const std::string & code);

	// Initialise base class properties of a newly-created dynamic terrain object.  Primarily responsible
	// for initialising per-instance data; general data should all be replicated via the clone copy construction
	void											InitialiseDynamicTerrainBase(void);

	// The dynamic terrain definition associated with this instance
	CMPINLINE const DynamicTerrainDefinition *		GetDynamicTerrainDefinition(void) const { return m_dynamic_terrain_def; }
	CMPINLINE void									SetDynamicTerrainDefinition(const DynamicTerrainDefinition *def) { m_dynamic_terrain_def = def; }

	// Event raised when an entity tries to interact with this object
	virtual bool									OnUsed(iObject *user);

	// Set a property of this dynamic terrain object
	void											SetProperty(const std::string & key, const std::string & value);

	// Set a property of this dynamic terrain object.  Called once the base class has registered the property.  Should 
	// be overridden by any subclasses that wish to use properties
	CMPINLINE virtual void							SetDynamicTerrainProperty(const std::string & key, const std::string & value) { };

	// Return the state that this terrain object is currently in
	CMPINLINE std::string							GetState(void) const { return m_state; }

	// Set the state of this dynamic terrain object
	CMPINLINE void									SetState(const std::string & state);

	// Returns the terrain to its default state, if one is specified in the terrain definition
	void											ReturnToDefaultState(void);

	// Invoke the default state transition from our current state, if one is defined
	void											ExecuteDefaultStateTransition(void);

	// Clear all properties that have been set on this object
	void											ClearDynamicTerrainProperties(void);

	// Pure virtual clone method for this object type; automatically implemented for subclasses
	// by the DYNAMIC_TERRAIN_CLASS macro
	virtual DynamicTerrain *						Clone(void) const = 0;


protected:

	// Pointer to the dynamic terrain definition for this object
	const DynamicTerrainDefinition *				m_dynamic_terrain_def;

	// ID of the state that this object is currently in
	std::string										m_state;

	// Set of properties that have been assigned to this object (and which will be propogated to any new
	// objects that are cloned to this one)
	std::unordered_map<std::string, std::string>	m_properties;

	// Default constructor
	DynamicTerrain(void);

	// Apply a particular state definition to this terrain object
	void											ApplyState(const DynamicTerrainState *state);

	// Clone the properties of this instance to another, specified instance
	void											ClonePropertiesToTarget(DynamicTerrain *target) const;


};
