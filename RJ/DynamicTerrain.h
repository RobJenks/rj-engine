#pragma once

#include "Terrain.h"
#include "DataEnabledObject.h"
#include "UsableObject.h"
#include "DynamicTerrainClassData.h"
#include "DynamicTerrainInteraction.h"
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

	// Method called by external parties attempting to interact with this object.  An 'extended' interaction is one in which the 
	// interaction key is held while the user takes some other actions, e.g. moving the mouse.  Filters interaction attempts
	// and will only allow those which match the permitted terrain interaction type 
	bool											AttemptInteraction(iObject *interacting_object, DynamicTerrainInteraction interaction);

	// Set a property of this dynamic terrain object
	void											SetProperty(const std::string & key, const std::string & value);

	// Return the state that this terrain object is currently in
	CMPINLINE std::string							GetState(void) const { return m_state; }

	// Set the state of this dynamic terrain object
	CMPINLINE void									SetState(const std::string & state);

	// Returns the terrain to its default state, if one is specified in the terrain definition
	void											ReturnToDefaultState(void);

	// Invoke the default state transition from our current state, if one is defined
	void											ExecuteDefaultStateTransition(void);

	// Reapply all properties of this dynamic terrain object
	void											ApplyProperties(void);

	// Clear all properties that have been set on this object
	void											ClearDynamicTerrainProperties(void);

	// Event raised after the dynamic terrain object is added to a new environment
	void											AddedToEnvironment(iSpaceObjectEnvironment *environment);

	// Event raised after the dynamic terrain object is removed from an environment
	void											RemovedFromEnvironment(iSpaceObjectEnvironment *environment);

	// Indicates whether an interaction is currently in progress with this object
	CMPINLINE bool									IsInteractionInProgress(void) const { return m_interaction_in_progress.WasSetSincePriorFrame(); }

	// Returns the ID of the object currently interacting with this terrain object, or 0 if no interaction is in progress
	CMPINLINE Game::ID_TYPE							GetInteractingObject(void) const { return (IsInteractionInProgress() ? m_interacting_object : 0); }

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

	// Flag which indicates whether the switch is currently being interacted with, and ID of the interacting object
	FrameFlag										m_interaction_in_progress;
	Game::ID_TYPE									m_interacting_object;

	// Default constructor
	DynamicTerrain(void);

	// Set a property of this dynamic terrain object.  Called once the base class has registered the property.  Should 
	// be overridden by any subclasses that wish to use properties
	CMPINLINE virtual void							SetDynamicTerrainProperty(const std::string & key, const std::string & value) { };

	// Event raised when an entity tries to interact with this object; should be implemented by derived classes
	virtual bool									OnUsed(iObject *user, DynamicTerrainInteraction && interaction);

	// Apply a particular state definition to this terrain object
	void											ApplyState(const DynamicTerrainState *state);

	// Clone the properties of this instance to another, specified instance
	void											ClonePropertiesToTarget(DynamicTerrain *target) const;


};
