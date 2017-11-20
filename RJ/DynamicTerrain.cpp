#include "ArticulatedModel.h"
#include "iObject.h"
#include "DynamicTerrainDefinition.h"
#include "DynamicTerrain.h"


// Default constructor
DynamicTerrain::DynamicTerrain(void)
	: 
	m_dynamic_terrain_def(NULL), m_interacting_object(0U)
{
	// Enable relevant flags on this terrain object
	m_isdynamic = true;
	m_dataenabled = true;
	m_usable = true;

	// Initialise other fields
	m_interaction_in_progress.Clear();
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

// Initialise base class properties of a newly-created dynamic terrain object.  Primarily responsible
// for initialising per-instance data; general data should all be replicated via the clone copy construction
void DynamicTerrain::InitialiseDynamicTerrainBase(void)
{
	// Assign a new unique ID to this object, given that it will otherwise still have the ID of its clone-source
	SetID(Terrain::GenerateNewUniqueID());

	// Revert the parent environment of this object since it is not initially assigned to any parent
	SetParentEnvironment(NULL);

	// If any per-instance articulated model is present then clone it for this new instance
	if (GetArticulatedModel()) SetArticulatedModel(GetArticulatedModel()->Copy());

	// Clear all port data since it has been cloned from the source and is not valid within this object.  We do 
	// so silently since the data is not valid within this object, and we do not want to start raising disconnection
	// events within the parent environment that could break existing valid connections
	ClearAllDataPortDataSilently();

	// Revert the terrain object to its default state, if one if defined
	ReturnToDefaultState();
}

// Set the state of this dynamic terrain object
void DynamicTerrain::SetState(const std::string & state)
{
	// Attempt to locate this state; quit immediately if it is not valid
	const DynamicTerrainDefinition *def = GetDynamicTerrainDefinition();	if (!def) return;
	const DynamicTerrainState *state_def = def->GetStateDefinition(state);	if (!state_def) return;

	// Store the new state code
	m_state = state;

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

// Set a property of this dynamic terrain object
void DynamicTerrain::SetProperty(const std::string & key, const std::string & value)
{
	// Store the property; overwrite any equivalent key if it already exists
	m_properties[key] = value;

	// Invoke the virtual subclass method, so that derived classes can respond to the new property if required
	SetDynamicTerrainProperty(key, value);
}

// Reapply all properties of this dynamic terrain object
void DynamicTerrain::ApplyProperties(void)
{
	// Reapply each property in turn
	for (const auto & prop : m_properties)
	{
		SetProperty(prop.first, prop.second);
	}
}

// Clone the properties of this instance to another, specified instance
void DynamicTerrain::ClonePropertiesToTarget(DynamicTerrain *target) const 
{
	if (!target) return;

	// Clear all properties that may have been shallow-copied into the target during cloning
	target->ClearDynamicTerrainProperties();

	// Clone each property of the object in turn
	for (const auto & prop : m_properties)
	{
		target->SetProperty(prop.first, prop.second);
	}
}

// Clear all properties that have been set on this object
void DynamicTerrain::ClearDynamicTerrainProperties(void)
{
	m_properties.clear();
}

// Event raised after the dynamic terrain object is added to a new environment
void DynamicTerrain::AddedToEnvironment(iSpaceObjectEnvironment *environment)
{
	// Apply all terrain properties now we have been added to a new context
	ApplyProperties();

}

// Event raised after the dynamic terrain object is removed from an environment
void DynamicTerrain::RemovedFromEnvironment(iSpaceObjectEnvironment *environment)
{

}

// Method called by external parties attempting to interact with this object.  An 'extended' interaction is one in which the 
// interaction key is held while the user takes some other actions, e.g. moving the mouse.  Filters interaction attempts
// and will only allow those which match the permitted terrain interaction type 
bool DynamicTerrain::AttemptInteraction(iObject *interacting_object, DynamicTerrainInteraction interaction)
{
	const DynamicTerrainDefinition *def = GetDynamicTerrainDefinition();
	if (!def) return false;

	// Some objects will explicitly disallow interaction
	if (def->GetPermittedInteractionType() == DynamicTerrainInteractionType::None) return false;

	// The interaction is not allowed if another entity is already interacting with the object
	if (IsInteractionInProgress() && !(interacting_object && interacting_object->GetID() == m_interacting_object)) return false;

	// Disallow the interaction if it is not of a type permitted by the object definition
	if (interaction.GetType() != def->GetPermittedInteractionType()) return false;

	/*** Perform the interaction ***/
	bool result = OnUsed(interacting_object, std::move(interaction));
	
	// Update our properties to show that an interaction is now taking place
	m_interaction_in_progress.Set();
	m_interacting_object = (interacting_object ? interacting_object->GetID() : 0);

	// Return the interaction result
	return result;
}


// Event raised when an entity tries to interact with this object
bool DynamicTerrain::OnUsed(iObject *user, DynamicTerrainInteraction && interaction)
{
	// Default behaviour if none is set by subclasses
	return false;
}