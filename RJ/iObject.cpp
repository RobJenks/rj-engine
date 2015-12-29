#include "GameVarsExtern.h"
#include "GameDataExtern.h"
#include "GameObjects.h"
#include "SimulationStateManager.h"
#include "Octree.h"
#include "CapitalShipPerimeterBeacon.h"
#include "FactionManagerObject.h"
#include "Faction.h"
#include "BasicProjectile.h"
#include "BasicProjectileDefinition.h"

#include "iObject.h"

// Static record of the current maximum ID for space objects; allows all space objects to be given a uniquely-identifying reference
Game::ID_TYPE iObject::InstanceCreationCount = 0;

// Static modifier for the size of the collision margin around an object's collision sphere.  Allows us to catch edge cases that may otherwise be missed
const float	iObject::COLLISION_SPHERE_MARGIN = 2.0f;

// Static collection that records the relation between different simulation states (relation is from row to column value)
const ComparisonResult iObject::SimStateRelations[][4] = 
{							/*   NoSimulation  */			/* Strategic */					/* Tactical */					/* Full Simulation */
	/* No Simulation */		{ ComparisonResult::Equal,		 ComparisonResult::LessThan,	ComparisonResult::LessThan,		ComparisonResult::LessThan },
	/* Strategic Sim */		{ ComparisonResult::GreaterThan, ComparisonResult::Equal,		ComparisonResult::LessThan,		ComparisonResult::LessThan }, 
	/* Tactical Sim */		{ ComparisonResult::GreaterThan, ComparisonResult::GreaterThan, ComparisonResult::Equal,		ComparisonResult::LessThan },
	/* Full Simulation */	{ ComparisonResult::GreaterThan, ComparisonResult::GreaterThan, ComparisonResult::GreaterThan,	ComparisonResult::Equal }
};

// Constructor; assigns a unique ID to this object
iObject::iObject(void) :	m_objecttype(iObject::ObjectType::Unknown), 
							m_objectclass(iObject::ObjectClass::UnknownObjectClass),
							m_isenvironment(false)
{
	// Initialise basic key fields to their default values
	m_code = "";
	m_name = "";
	m_instancecode = "";
	m_model = NULL;
	m_articulatedmodel = NULL;
	m_codehash = m_instancecodehash = 0U;
	m_standardobject = false;
	m_faction = Faction::NullFaction;
	m_simulationhub = false;
	m_visible = true;
	m_position = NULL_VECTOR;
	m_positionf = NULL_FLOAT3;
	m_orientation = ID_QUATERNION;
	m_orientationmatrix = m_inverseorientationmatrix = ID_MATRIX;
	m_worldmatrix = m_inverseworld = ID_MATRIX;
	m_treenode = NULL;
	m_centreoffset = NULL_VECTOR;
	m_orientchanges = 0;
	m_nocollision_count = 0;

	// Initialise this object with a unique ID
	AssignNewUniqueID();

	// All objects begin outside of the simulation, and will only be added once the simulation state is updated for the first time
	m_simulationstate = m_nextsimulationstate = ObjectSimulationState::NoSimulation;

	// No attachments to start with
	m_childcount = 0;
	m_parentobject = NULL;

	// Set collision parameters
	CollisionOBB.Parent = this;
	SetCollisionMode(Game::CollisionMode::NoCollision);
	SetColliderType(Game::ColliderType::ActiveCollider);
	SetVisibilityTestingMode(VisibilityTestingModeType::UseBoundingSphere);

	// Set all flags to default values
	m_simulated = m_posupdated = m_spatialdatachanged = m_currentlyvisible = false;

	// Subclass will set this value if it implements a PerformPostSimulationUpdate() method
	m_canperformpostsimulationupdate = false;

	// Set a default value for size
	SetSize(XMVectorReplicate(1.0f));

	// By default, the root collision box will auto-fit to the model bounds.  Add after SetSize() so 
	// that we don't redundantly calculate this twice (since if AutoFit==true, SetSize() will trigger
	// a recalculation as well)
	CollisionOBB.SetAutoFitMode(true);
}

// Assigns a new unique ID to this object
void iObject::AssignNewUniqueID(void)
{
	// Generate a new unique ID
	m_id = GenerateNewObjectID();

	// Generate a new instance code based on this ID
	DetermineInstanceCode();
}

// Sets the object code
void iObject::SetCode(const std::string & code)
{
	// First, test whether the instance code has been overriden manually (since it will otherwise be affected by this code change)
	bool overriden = TestForOverrideOfInstanceCode();

	// Set the new code and calculate the hash
	m_code = code; m_codehash = HashString(m_code);

	// If the ship instance code has not been set manually, recalculate it now
	if (!overriden) DetermineInstanceCode();
}

// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void iObject::InitialiseCopiedObject(iObject *source)
{
	// Assign a new unique ID for this object, so that it doesn't retain the ID of its copy source.  Set the 
	// instance code to a null string beforehand, otherwise if the source object had overridden its instance
	// code the new object would also inherit it, and would not generate a new one since the code is overridden
	m_instancecode = NullString;
	AssignNewUniqueID();

	// Simulation state will always begin as "no simulation"
	m_simulationstate = iObject::ObjectSimulationState::NoSimulation;
	m_nextsimulationstate = iObject::ObjectSimulationState::NoSimulation;

	// We do not begin within any spatial partitioning tree node
	m_treenode = NULL;

	// Remove the 'standard' flag from items following a copy
	m_standardobject = false;

	// Remove any attachments that were applied to the parent object
	m_parentobject = NULL;
	m_childobjects.clear();
	m_childcount = 0;

	// Remove any collision exclusions from this object; will not be the same as the parent
	m_nocollision.clear();
	m_nocollision_count = 0;

	// Deep-copy the object collision data
	OrientedBoundingBox::CloneOBBHierarchy(source->CollisionOBB, CollisionOBB, this);
}

// Sets the simulation state of this object.  Pending state change will be recorded, and it will then be actioned on the 
// next cycle.  Change can be made immediately by directly invoking SimulationStateChanged(), but this could have 
// unintended consequences if called within the game object update loop (which uses iterators)
void iObject::SetSimulationState(ObjectSimulationState state)
{
	if (m_objecttype == iObject::ObjectType::SpaceEmitterObject)
	{
		int a = 1 + Game::ClockMs;
	}

	// Record the requested change in state
	m_nextsimulationstate = state; 

	// Special case: if the current state is "No Simulation", the object will not be checked next cycle for any pending state
	// change (since objects not being simulated are not even registered as objects).  We should therefore directly register
	// the object here, which means it will be added next cycle, and the state will also then be checked & actioned next cycle
	if (m_simulationstate == iObject::ObjectSimulationState::NoSimulation) Game::RegisterObject(this);
}

// Handles a change to the simulation state.  The simulation state determines what level of simulation (if any) should be run for this object.  Any changes
// requested to the state are stored in m_nextsimulationstate.  In the next cycle, if nextstate != currentstate we run this method to handle the change.
// This allows the simulation manager to make multiple changes in a cycle (e.g. an object moving from system to system would have state of Simulated > 
// No Simulation > Simulation) and only implement the effects afterwards when all states are final.  This method can however be called directly if we
// want to force the change to take effect immediately within the cycle
void iObject::SimulationStateChanged(void)
{
	// If no change is being made then return immediately
	if (m_nextsimulationstate == m_simulationstate) return;

	// Change the object state.  Store the previous simulation state so that events can consider both previous & new event in the transition
	iObject::ObjectSimulationState prevstate = m_simulationstate;
	m_simulationstate = m_nextsimulationstate;

	/* Now handle any effects of the change */

	// If we are moving from 'no simulation' to any other state where we need to be simulated, we need to add this object into the central object collection
	if (prevstate == iObject::ObjectSimulationState::NoSimulation)	// and we know state != m_state, therefore we will now be simulated in some way
	{
		// Register with the central object collection
		Game::RegisterObject(this);
	}

	// Conversely, if we are moving to a state where we are no longer simulated then remove from the central object collection
	if (m_simulationstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// Remove from the central object simulation
		Game::UnregisterObject(this);
	}

	// We also need to pass the change in simulation state to any attached objects
	std::vector<Attachment<iObject*>>::iterator it_end = m_childobjects.end();
	for (std::vector<Attachment<iObject*>>::iterator it = m_childobjects.begin(); it != it_end; ++it)
	{
		if ((*it).Child) (*it).Child->SetSimulationState(m_simulationstate);
	}

	// Finally, trigger the virtual state change event so that subclasses can handle the change as appropriate
	SimulationStateChanged(prevstate, m_simulationstate);
}


// Protected method to set the object type, which also derives and stores the object class
void iObject::SetObjectType(iObject::ObjectType type)
{
	// Store the object type
	m_objecttype = type;

	// Derive and store the object class
	m_objectclass = iObject::DetermineObjectClass(*this);
}

// Core iObject method to simulate any object.  Passes control down the hierarchy to virtual SimulateObject() method during execution
void iObject::Simulate(void)
{
	// Only simulate the object if it has not already been simulated
	if (m_simulated == false)
	{
		// Call the virtual subclass function to fully simulate the object
		Game::ID_TYPE id = m_id;
		SimulateObject();

		// Objects can be destroyed within their simulation cycle; perform a check here to ensure the object is still active
		if (Game::ObjectExists(id) == false) return;

		// Set the "simulated" flag for this frame
		m_simulated = true;

		// Update the position of any child objects, if we have any (checking count!=0 here, instead of in function, avoids unnecessary function calls)
		if (m_childcount != 0) UpdatePositionOfChildObjects();
	}

	// If the object position changed during this frame (regardless of whether it was simulated) then 
	// recalculate any derived data (e.g. world matrices)
	if (m_spatialdatachanged)
	{
		// Perform base object updates required when the object moves, e.g. ensuring quaternions are normalised,
		// and deriving a new world transform for the object
		RenormaliseSpatialData();
		DeriveNewWorldMatrix();

		// Update our position in the spatial partitioning tree
		if (m_treenode) m_treenode->ItemMoved(this, m_position);

		// Perform a post-simulation update if required, and if available in the class in question
		if (IsPostSimulationUpdateRequired()) PerformPostSimulationUpdate();

		// Clear the flag that indicates spatial data was changed, since we have now responded to it
		m_spatialdatachanged = false;
	}
}

// Updates the object before it is rendered.  Called only when the object enters the render queue (i.e. not when it is out of view)
void iObject::PerformRenderUpdate(void)
{
	// Update any render effects that may be active on the object
	if (Fade.IsActive()) Fade.Update();
}

// Returns the disposition of this object towards the target object, based on our respective factions and 
// any other modifiers (e.g. if the objects have individually attacked each other)
Faction::FactionDisposition iObject::GetDispositionTowardsObject(const iObject *obj) const
{
	// Make sure the target object is valid
	if (!obj) return Faction::FactionDisposition::Neutral;

	// Use the disposition of our faction to the other object as a basis
	Faction::FactionDisposition disp = Game::FactionManager.GetDisposition(m_faction, obj->GetFaction());

	// Consider any other factors, e.g. if the objects have individually attacked each other

	// Return the disposition value
	return disp;
}

// Assigns this object as a new simulation hub, around which the universe will be fully simulated
void iObject::SetAsSimulationHub(void)
{
	// Only take action if we were not previously a simulation hub
	if (!m_simulationhub)
	{
		// Set the flag
		m_simulationhub = true;

		// Notify the state manager that a simulation hub has been added
		Game::StateManager.SimulationHubEnteringEnvironment(this);
	}
}

// Removes the simulation hub designation from this object, so it will no longer cause the universe to simulate fully around it
void iObject::RemoveSimulationHub(void)
{
	// Only take action if we were previously a simulation hub
	if (m_simulationhub)
	{
		// Set the flag
		m_simulationhub = false;

		// Notify the state manager that a simulation hub has left
		Game::StateManager.SimulationHubLeavingEnvironment(this);
	}
}

// Virtual shutdown method that must be implemented by all objects
void iObject::Shutdown(void)
{
	// Release any attachments from this object to others
	ReleaseAllAttachments();

	// If this object is part of any mutual collision exclusions, remove from the other object in the pair
	iObject *obj;
	for (int i = 0; i < m_nocollision_count; ++i)
	{
		obj = Game::GetObjectByID(m_nocollision[i]);
		if (obj) obj->RemoveCollisionExclusion(m_id);
	}

	// De-register from the global collection before the object is finally deallocated.  The per-frame cleanup
	// code will then process the unregister list and deallocate/delete this object
	Game::UnregisterObject(this);
}


// Default destructor
iObject::~iObject(void)
{
}

// Set the size of this object.  Recalculates any dependent fields, e.g. those involved in collision detection
void iObject::SetSize(const FXMVECTOR size)
{
	// Store the size parameter
	m_size = size;

	// Make sure the supplied parameter is valid; if any dimension is <= epsilon then assign a default size
	if (XMVector3AnyTrue(XMVectorLess(m_size, Game::C_EPSILON_V)))
		m_size = XMVectorReplicate(1.0f);

	// Recalculate the collision sphere radii for broadphase collision detection
	m_collisionsphereradiussq = DetermineCuboidBoundingSphereRadiusSq(m_size); 
	m_collisionsphereradius = sqrtf(m_collisionsphereradiussq);

	// Also calculate a collision sphere including margin, to catch edge cases that could potentially otherwise be missed
	m_collisionspheremarginradius = (m_collisionsphereradius * iObject::COLLISION_SPHERE_MARGIN);

	// Store the local float representation to allow us to perform per-component tests at runtime
	XMStoreFloat3(&m_sizef, m_size);

	// Recalculate the object fast mover threshold
	m_fastmoverthresholdsq = (min(m_sizef.x, min(m_sizef.y, m_sizef.z)) * Game::C_OBJECT_FAST_MOVER_THRESHOLD);
	m_fastmoverthresholdsq *= m_fastmoverthresholdsq;

	// Recalculate the object size ratio; the ratio of the object's largest dimension to its smallest.  Can be used
	// to e.g. select between different intersection tests.  For example, OBB is more appropriate for ratios closer to 1
	float mindimension = min(min(m_sizef.x, m_sizef.y), m_sizef.z);
	float maxdimension = max(max(m_sizef.x, m_sizef.y), m_sizef.z);
	m_size_ratio = (maxdimension / mindimension);
	m_best_bounding_volume = GamePhysicsEngine::DetermineBestBoundingVolumeTypeForObject(this);

	// Invalidate the OBB based on this change in size
	CollisionOBB.Invalidate();

	// Set/update the OBB extent to match this new size, if the OBB is set to auto-calculate
	if (CollisionOBB.AutoFitObjectBounds())
	{
		CollisionOBB.UpdateExtentFromSize(m_size);
	}
}

// Sets the object instance code.  Protected to ensure that data is kept in sync.  Will handle any notification of 
// updates to the central data collections
void iObject::SetInstanceCode(const std::string & instance_code)
{
	// Parameter check
	if (instance_code == NullString) return;

	// We will only update the central collection if the current instance code is not null.  If it is a null string, 
	// the object is being created and so does not yet exist in the central collection.  Store so we can test later
	std::string old_code = m_instancecode;

	// Store the new instance code
	m_instancecode = instance_code;

	// Hash the instance code for more efficient lookups
	m_instancecodehash = HashString(m_instancecode);

	// Notify the central collection of this update if required
	if (old_code != NullString) Game::NotifyChangeOfObjectInstanceCode(this, old_code);
}

// Determines the instance code that should be assigned to this object
void iObject::DetermineInstanceCode(void)
{
	// Instance code should be a concatenation of the object code and unique ID; wil ensure unique assignment of instance codes
	std::string code = concat(m_code)("_")(m_id).str();
	SetInstanceCode(code);
}

// Overrides the object instance code with a custom string value
void iObject::OverrideInstanceCode(const std::string & icode)
{
	// Ensure the code is valid
	if (icode == "") return;

	// Update the instance code, which will take care of updating the central collection etc
	SetInstanceCode(icode);
}

bool iObject::TestForOverrideOfInstanceCode(void) const
{
	// Construct the expected instance code (in case of no override) and test against the actual instance code
	if (m_instancecode == NullString) return false;
	std::string expectedcode = concat(m_code)("_")(m_id).str();
	return (m_instancecode != expectedcode);
}

void iObject::UpdatePositionOfChildObjects(void)
{
	// We only need to update the position or orientation of any child objects if our own state has changed
	if (!m_spatialdatachanged) return;

	// Iterate over our child object attachments and update each child in turn
	AttachmentSet::iterator it_end = m_childobjects.end();
	for (AttachmentSet::iterator it = m_childobjects.begin(); it != it_end; ++it)
	{
		// Get a handle to the specific attachment
		Attachment<iObject*> & attach = (*it);

		// Only update this child if it has not already been updated; avoids infinite loops
		if (attach.Child && !(attach.Child->PositionUpdated()))
		{
			// Apply the effect of this attachment on the child
			attach.Apply();

			// Set the child update flag
			attach.Child->SetPositionUpdated(true);

			// Update all children of this child
			// TODO: This could lead to infinite loops if Parent >AttachedTo> Child >AttachedTo> Parent
			attach.Child->UpdatePositionOfChildObjects();
		}
	}
}

// Add a new attachment from this object to a child.  Sets default (zero) offset parameters.
void iObject::AddChildAttachment(iObject *child)
{
	AddChildAttachment(child, NULL_VECTOR3, NULL_VECTOR4);
}

// Add a new attachment from this object to a child, including parameters for offsetting the position & orientation relative 
// to the parent.  Breaks the attachment from the child side as well.
void iObject::AddChildAttachment(iObject *child, const FXMVECTOR posoffset, const FXMVECTOR orientoffset)
{
	// Parameter check
	if (!child) return;

	// Make sure the child object isn't already attached to an object; if so, break the attachment (this will break from both sides)
	if (child->HaveParentAttachment())
	{
		iObject *oldparent = child->GetParentObject();
		oldparent->RemoveChildAttachment(child);
	}

	// Make sure the parent object position is up-to-date before applying the attachment
	RefreshPositionImmediate();

	// Create a new child attachment, store it and recalculate the total attachment count.  Then apply it to update child position
	Attachment<iObject*> attach = Attachment<iObject*>(this, child, posoffset, orientoffset);
	m_childobjects.push_back(attach);
	m_childcount = (int)m_childobjects.size();
	attach.Apply();

	// Set the parent pointer in the child object to set the other side of the attachment
	child->SetParentObjectDirect(this);
}

// Removes a child attachment to this object, if one exists.  Breaks the attachment from the child side as well.
void iObject::RemoveChildAttachment(iObject *child)
{
	// Parameter check
	if (!child) return;

	// Attmept to remove from the vector of child attachments
	for (int i = 0; i < m_childcount; ++i)
	{
		if (m_childobjects[i].Child == child)
		{
			// We have located the attachment, so remove it from our child collection
			//RemoveFromVectorAtIndex<Attachment<iObject*>>(m_childobjects, i);
			RemoveFromVectorAtIndex<Attachment<iObject*>, iObject::AttachmentSet>(m_childobjects, i);

			// Remove the reverse child pointer to its parent
			child->SetParentObjectDirect(NULL);

			// Break out of the loop; no need to search further
			break;
		}
	}

	// Recalculate the child count for this object
	m_childcount = (int)m_childobjects.size();
}


// Special case method for removing all child attachments in one go; more efficient than multiple calls to RemoveChildAttachment
void iObject::ReleaseAllAttachments(void)
{
	// Loop through each child object in turn and remove the pointer back its parent (this object)
	for (int i = 0; i < m_childcount; ++i)
		if (m_childobjects[i].Child) m_childobjects[i].Child->SetParentObjectDirect(NULL);

	// Now clear the vector of child attachments from this object, to break the other half of every attachment
	m_childobjects.clear();
	m_childcount = 0;
}


// Determines whether an attachment exists to the specified child object
bool iObject::HaveChildAttachment(iObject *child)
{
	// Search the attachment vector for this object
	for (int i = 0; i < m_childcount; ++i)
		if (m_childobjects[i].Child == child) return true;

	return false;
}

// Returns details on the attachment to the specified child object (if there is one).  Returns a NULL struct (with NULL object pointers
// and offsets) if no attachment exists.  Returns a new object by value so some overheard
Attachment<iObject*> iObject::RetrieveChildAttachmentDetails(iObject *child)
{
	// Search the vector of attachments for this object
	for (int i = 0; i < m_childcount; ++i)
		if (m_childobjects[i].Child == child) return m_childobjects[i];

	// No attachment exists, so return NULL data
	return Attachment<iObject*>(NULL, NULL, NULL_VECTOR, ID_QUATERNION);
}

// Detaches this object from any parent object.  Simply calls the RemoveChildAttachment method on the parent object.
// This will also set our parent pointer to NULL following removal.
void iObject::DetachFromParent(void)
{
	if (m_parentobject)
	{
		m_parentobject->RemoveChildAttachment(this);
	}
}

// Add a new exclusion, preventing this object from colliding with the designated object
void iObject::AddCollisionExclusion(Game::ID_TYPE object)
{
	// We only want to add this exclusion if we don't already have it
	if (CollisionExcludedWithObject(object)) return;

	// Make sure we are not exceeding the capacity limit; if so, simply quit here
	if (m_nocollision_count >= Game::C_MAX_OBJECT_COLLISION_EXCLUSIONS) return;

	// Add to the collision exclusion array
	m_nocollision.push_back(object);
	m_nocollision_count = (int)m_nocollision.size();
}

// Remove an exclusion, allowing this object to collide with the specified object again
void iObject::RemoveCollisionExclusion(Game::ID_TYPE object)
{
	// Remove this collision exclusion if it exists in the array
	RemoveFromVector<Game::ID_TYPE>(m_nocollision, object);
	m_nocollision_count = (int)m_nocollision.size();
}

// Method called when a projectile (not an object, but a basic projectile) collides with this object
// Handles any damage or effects of the collision, and also triggers rendering of appropriate effects if required
void iObject::HandleProjectileImpact(BasicProjectile & proj, GamePhysicsEngine::OBBIntersectionData & impact)
{
	// The projectile must have a definition to provide the required data
	if (!proj.Definition) return;

	// Render an appropriate impact effect

	// Apply damage from the impact
	ApplyDamage(proj.Definition->GetProjectileDamage());
}


// Static method to translate from an object simulation state to its string representation
std::string iObject::TranslateSimulationStateToString(iObject::ObjectSimulationState state)
{
	switch (state)
	{
		case iObject::ObjectSimulationState::FullSimulation:		return "fullsimulation";
		case iObject::ObjectSimulationState::TacticalSimulation:	return "tacticalsimulation";
		case iObject::ObjectSimulationState::StrategicSimulation:	return "strategicsimulation";
		
		default:													return "nosimulation";
	}
}

// Static method to translate to an object simulation state from its string representation
iObject::ObjectSimulationState iObject::TranslateSimulationStateFromString(const std::string & state)
{
	if (state == "fullsimulation")					return iObject::ObjectSimulationState::FullSimulation;
	else if (state == "tacticalsimulation")			return iObject::ObjectSimulationState::TacticalSimulation;
	else if (state == "strategicsimulation")		return iObject::ObjectSimulationState::StrategicSimulation;

	else											return iObject::ObjectSimulationState::NoSimulation;
}

// Static method to detemine the object classs of an object; i.e. whether it is space- or environment-based
iObject::ObjectClass iObject::DetermineObjectClass(const iObject & object)
{
	// Object class is based upon the more granular object type
	switch (object.GetObjectType())
	{
		case iObject::ObjectType::ShipObject:
		case iObject::ObjectType::SimpleShipObject:
		case iObject::ObjectType::ComplexShipObject:
		case iObject::ObjectType::ComplexShipSectionObject:
		case iObject::ObjectType::SpaceEmitterObject:
		case iObject::ObjectType::CapitalShipPerimeterBeaconObject:
		case iObject::ObjectType::ProjectileObject:
			return iObject::ObjectClass::SpaceObjectClass;

		case iObject::ObjectType::ActorObject:
			return iObject::ObjectClass::EnvironmentObjectClass;

		default:
			return iObject::ObjectClass::UnknownObjectClass;
	}
}




