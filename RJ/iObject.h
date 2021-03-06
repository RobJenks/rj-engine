#pragma once

#ifndef __iObjectH__
#define __iObjectH__

#include <vector>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "GameVarsExtern.h"
#include "Utility.h"
#include "AlignedAllocator.h"
#include "HashFunctions.h"
#include "Octree.h"
#include "FrameFlag.h"
#include "iTakesDamage.h"
#include "Attachment.h"
#include "OrientedBoundingBox.h"
#include "GamePhysicsEngine.h"
#include "FadeEffect.h"
#include "HighlightEffect.h"
#include "Faction.h"
#include "AudioItem.h"
#include "AudioParameters.h"
#include "GameConsoleCommand.h"
#include "ModelInstance.h"
#include "InstanceFlags.h"
class ArticulatedModel;
struct BasicProjectile;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class iObject : public iTakesDamage
{
public:

	// Static record of the highest ID value in existence, for assigning new objects upon registration
	static Game::ID_TYPE						InstanceCreationCount;

	// Enumeration of flags maintained within the iObject class (still using bools, haven't implemented this yet)
	/*enum ObjectFlags
	{
		OF_Simulated		= (0x000000),
		OF_PositionUpdated	= (1 << 0),
		OF_StandardObject	= (1 << 1),
		OF_Visible			= (1 << 2),
		OF_SimulationHub	= (1 << 3),
		OF_OBBInvalidated	= (1 << 4),
		OF_IsEnvironment	= (1 << 5)
	};*/

	// Enumeration of possible object simulation states
	enum ObjectSimulationState { NoSimulation = 0, StrategicSimulation, TacticalSimulation, FullSimulation };

	// Define a standard type for the collection of all attachments from this object to others
	typedef std::vector<Attachment<iObject*>, AlignedAllocator<Attachment<iObject*>, 16>>	AttachmentSet;

	// Static modifier for the size of the collision margin around an object's collision sphere.  2.0x the collision sphere
	// to ensure all cases are caught.  If current object's sphere is larger then 
	static const float							COLLISION_SPHERE_MARGIN;

	// Enumeration of all object types
	enum ObjectType {
		Unknown = 0, ShipObject, SimpleShipObject, ComplexShipObject, ComplexShipSectionObject,
		SpaceEmitterObject, ActorObject, CapitalShipPerimeterBeaconObject, ProjectileObject, LightSourceObject
	};

	// Enumeration of object classes
	enum ObjectClass { UnknownObjectClass = 0, SpaceObjectClass, EnvironmentObjectClass };

	// Constructor
	iObject(void);

	// Method to return the unique ID of this object, or to request a new ID (e.g. when copying objects)
	CMPINLINE Game::ID_TYPE					GetID(void)	const						{ return m_id; }
	void									AssignNewUniqueID(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void									InitialiseCopiedObject(iObject *source);

	// Methods to get and set the key string properties for this object
	CMPINLINE std::string					GetCode(void) const						{ return m_code; }
	CMPINLINE HashVal						GetCodeHash(void) const					{ return m_codehash; }
	CMPINLINE std::string					GetName(void) const						{ return m_name; }
	CMPINLINE void							SetName(const std::string & name)		{ m_name = name; }

	// Sets the object code
	void									SetCode(const std::string & code);

	// Returns the type of space object being considered
	CMPINLINE iObject::ObjectType			GetObjectType(void) const				{ return m_objecttype; }
	CMPINLINE iObject::ObjectClass			GetObjectClass(void) const				{ return m_objectclass; }
	CMPINLINE bool							IsEnvironment(void) const				{ return m_isenvironment; }
	CMPINLINE bool							IsShip(void) const
	{
		return (m_objecttype == iObject::ObjectType::SimpleShipObject || m_objecttype == iObject::ObjectType::ComplexShipObject);
	}

	// Methods to get/set the instance code, and to determine whether it has been overridden manually
	CMPINLINE const std::string &			GetInstanceCode(void) const				{ return m_instancecode; }
	CMPINLINE HashVal						GetInstanceCodeHash(void) const			{ return m_instancecodehash; }
	void									DetermineInstanceCode(void);
	bool									OverrideInstanceCode(const std::string & icode);
	bool									TestForOverrideOfInstanceCode(void) const;

	// Override the unique ID of this object.  Not advisable unless done in a controlled manner
	void									ForceOverrideUniqueID(Game::ID_TYPE new_id, bool derive_new_instance_code);

	// Flag indicating whether this is a 'standard', centrally-maintained template object
	CMPINLINE bool							IsStandardObject(void) const			{ return m_standardobject; }
	CMPINLINE void							SetIsStandardObject(bool standard)		{ m_standardobject = standard; }

	// Position and orientation; all objects exist somewhere in the world
	CMPINLINE XMVECTOR 						GetPosition(void) const					{ return m_position; }
	CMPINLINE XMFLOAT3						GetPositionF(void) const				{ return m_positionf; }
	CMPINLINE void							SetPosition(const FXMVECTOR pos)
	{
		m_position = pos;
		XMStoreFloat3(&m_positionf, m_position);

		FlagSpatialDataChange();
		CollisionOBB.Invalidate();
	}
	CMPINLINE void							SetPosition(const XMFLOAT3 & pos)
	{
		m_positionf = pos;
		m_position = XMLoadFloat3(&m_positionf);

		FlagSpatialDataChange();
		CollisionOBB.Invalidate();
	}
	CMPINLINE void							AddDeltaPosition(const FXMVECTOR delta)
	{
		SetPosition(XMVectorAdd(m_position, delta));
	}
	CMPINLINE void							ChangePosition(const FXMVECTOR delta)
	{
		AddDeltaPosition(delta);
	}

	CMPINLINE const XMVECTOR				GetOrientation(void) const				{ return m_orientation; }
	CMPINLINE void							SetOrientation(const FXMVECTOR orient)
	{
		m_orientation = orient;
		FlagSpatialDataChange();
		CollisionOBB.Invalidate();
	}
	CMPINLINE void							SetOrientation(const XMFLOAT4 & orient) { SetOrientation(XMLoadFloat4(&orient)); }


	CMPINLINE void							SetPositionAndOrientation(const FXMVECTOR pos, const FXMVECTOR orient)
	{
		m_position = pos; m_orientation = orient;
		FlagSpatialDataChange();
		CollisionOBB.Invalidate();
	}
	CMPINLINE void							SetPositionAndOrientation_NoInvalidation(const FXMVECTOR pos, const FXMVECTOR orient)
	{
		m_position = pos; m_orientation = orient;
		FlagSpatialDataChange();
		CollisionOBB.Invalidate();
	}

	CMPINLINE void							ChangeOrientation(const FXMVECTOR rot)
	{
		// Multiply orientation D3DXQUATERNIONs to generate the new D3DXQUATERNION
		SetOrientation(XMQuaternionMultiply(rot, m_orientation));
	}

	CMPINLINE void							AddDeltaOrientation(const FXMVECTOR dq)
	{
		// Add the incremental quaternion
		SetOrientation(XMVectorAdd(m_orientation, dq));
	}

	CMPINLINE void							ChangeOrientation(const XMFLOAT4 & rot)	{ ChangeOrientation(XMLoadFloat4(&rot)); }
	CMPINLINE void							AddDeltaOrientation(const XMFLOAT4 & dq) { AddDeltaOrientation(XMLoadFloat4(&dq)); }

	// Rotate the object about one of its axes
	CMPINLINE void							RotateAboutX(float rad) { ChangeOrientation(XMQuaternionRotationAxis(RIGHT_VECTOR, rad)); }
	CMPINLINE void							RotateAboutY(float rad) { ChangeOrientation(XMQuaternionRotationAxis(UP_VECTOR, rad)); }
	CMPINLINE void							RotateAboutZ(float rad) { ChangeOrientation(XMQuaternionRotationAxis(FORWARD_VECTOR, rad)); }

	// Methods to retrieve the (automatically-maintained) orientation matrix and its inverse
	CMPINLINE const XMMATRIX				GetOrientationMatrix(void) const { return m_orientationmatrix; }
	CMPINLINE const XMMATRIX				GetInverseOrientationMatrix(void) const { return m_inverseorientationmatrix; }

	// The world matrix of this object
	CMPINLINE XMMATRIX						GetWorldMatrix(void) const { return m_worldmatrix; }
	CMPINLINE XMMATRIX						GetInverseWorldMatrix(void)	const { return m_inverseworld; }
	CMPINLINE void RJ_XM_CALLCONV			SetWorldMatrix(const FXMMATRIX m)
	{
		// Store the new world matrix
		m_worldmatrix = m;

		// Calculate the inverse world matrix for rendering efficiency
		m_inverseworld = XMMatrixInverse(NULL, m_worldmatrix);
	}

	// Derives a new object world matrix
	CMPINLINE void							DeriveNewWorldMatrix(void);

	// Return the previous-frame world transform, where applicable.  If it was not calculated last frame, returns the current transform
	CMPINLINE XMMATRIX						GetLastWorldMatrix(void) const 
	{ 
		return (m_worldcurrent.IsSet() ? m_lastworld : m_worldmatrix);
	}


	// Indicates whether we should skip world matrix derivation for this object, because it will be derived elsewhere
	// For example, for environment objects which have their world-space data calculated by their parent
	CMPINLINE bool							OverridesWorldMatrixDerivation(void) const { return m_overrides_world_derivation; }

	// Method to force an immediate recalculation of player position/orientation, for circumstances where we cannot wait until the
	// end of the frame (e.g. for use in further calculations within the same frame that require the updated data)
	// TODO: No longer needs to be virtual, since iObject can handle all logic itself?
	virtual void							RefreshPositionImmediate(void)
	{
		DeriveNewWorldMatrix();
	}

	// Core iObject method to simulate any object.  Passes control down the hierarchy to virtual SimulateObject() method during execution
	void									Simulate(void);

	// All objects must expose a method to simulate themselves.  Flag indicates whether they are allowed to simulate movement (vs if the object is attached)
	virtual void							SimulateObject(void) = 0;

	// Notifies our spatial partitioning tree node that the object has possibly moved, so that it can be re-assessed if required
	CMPINLINE void							UpdateSpatialPartitioningTreePosition(void)
	{
		if (m_treenode) m_treenode->ItemMoved(this, m_position);
	}

	// Method that indicates whether the object requires a post-simulation update or not.  Usually means the object position/orientation has changed.
	// Use this flag to minimise the number of virtual function calls required where there is nothing to be done
	CMPINLINE bool							IsPostSimulationUpdateRequired(void) const			
	{ 
		// Only perform an update if the class implements post-simulation updates
		return m_canperformpostsimulationupdate;
	}

	// Set the flag that indicates whether this object requires a post-simulation update
	CMPINLINE void							SetPostSimulationUpdateFlag(bool perform_update) { m_canperformpostsimulationupdate = perform_update; }

	// Objects can also expose a method to update object state following a simulation.  Some objects will not require this method; others
	// will need to e.g. update the position of any contained objects following a change in position
	virtual void							PerformPostSimulationUpdate(void) { }

	// Effects that can be activated on this object
	FadeEffect								Fade;					// Allows the object to be faded in and out
	HighlightEffect							Highlight;              // Allows a highlight to be applied over object textures

	// Updates the object before it is rendered.  Called only when the object enters the render queue (i.e. not when it is out of view)
	void                                    PerformRenderUpdate(void);

	// The simulation state determines what level of simulation (if any) should be run for this object
	CMPINLINE ObjectSimulationState			SimulationState(void) const							{ return m_simulationstate; }
	CMPINLINE ObjectSimulationState			RequestedSimulationState(void) const				{ return m_nextsimulationstate; }
	CMPINLINE bool							SimulationStateChangePending(void) const			{ return m_nextsimulationstate != m_simulationstate; }
	void									SetSimulationState(ObjectSimulationState state);

	// Handles a change to the simulation state.  The simulation state determines what level of simulation (if any) should be run for this object.  Any changes
	// requested to the state are stored in m_nextsimulationstate.  In the next cycle, if nextstate != currentstate we run this method to handle the change.
	// This allows the simulation manager to make multiple changes in a cycle (e.g. an object moving from system to system would have state of Simulated > 
	// No Simulation > Simulation) and only implement the effects afterwards when all states are final.  This method can however be called directly if we
	// want to force the change to take effect immediately within the cycle
	void									SimulationStateChanged(void);

	// Subclasses must implement a method to handle any change in simulation state
	virtual void							SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate) = 0;
	
	// Flag determining whether the object is visible
	CMPINLINE bool							IsVisible(void) const				{ return m_visible; }
	CMPINLINE void							SetIsVisible(bool visible)			{ m_visible = visible; }

	// Flag determining whether this object is a 'simulation hub', around which the environment is fully simulated.  Typically for players.
	CMPINLINE bool							IsSimulationHub(void) const			{ return m_simulationhub; }
	void									SetAsSimulationHub(void);
	void									RemoveSimulationHub(void);

	// Flag indicating whether the object has been rendered this frame
	CMPINLINE bool							IsRendered(void) const				{ return m_rendered.IsSet(); }
	CMPINLINE void							MarkAsRendered(void)				{ m_rendered.Set(); }

	// Instance-rendering flags for this object
	InstanceFlags							InstanceFlags;

	// Indicates whether this object is a shadow-caster
	CMPINLINE bool							IsShadowCaster(void) const					{ return InstanceFlags.GetFlag(InstanceFlags::INSTANCE_FLAG_SHADOW_CASTER); }
	CMPINLINE void							SetShadowCastingState(bool shadow_caster)	{ InstanceFlags.SetFlag(InstanceFlags::INSTANCE_FLAG_SHADOW_CASTER); }

	// Virtual shutdown method that must be implemented by all objects
	virtual void							Shutdown(void);

	// The size of this object in world coordinates
	void									SetSize(const FXMVECTOR size, bool preserve_proportions = true);
	CMPINLINE void							SetSize(const XMFLOAT3 & size, bool preserve_proportions = true)	{ SetSize(XMLoadFloat3(&size), preserve_proportions); }
	CMPINLINE XMVECTOR						GetSize(void) const					{ return m_size; }
	CMPINLINE const XMFLOAT3 &				GetSizeF(void) const				{ return m_sizef; }
	CMPINLINE float							GetSizeRatio(void) const			{ return m_size_ratio; }
	CMPINLINE Game::BoundingVolumeType		MostAppropriateBoundingVolumeType(void) const { return m_best_bounding_volume; }

	// We can set the object size with a single parameter; the largest object dimension will be scaled
	// to this value, maintaining proportions, based on the underlying model size.  If the object has no 
	// model then the size will be uniformly scaled by this value
	void									SetSize(float max_dimension);
	CMPINLINE void							SetMaxSize(float max_dimension) { SetSize(max_dimension); }		// For DEBUG_FN which can't handle the overload

	// Reference to the instance of this model and any per-instance data
	CMPINLINE ModelInstance &				GetModelInstance(void)				{ return m_model; }

	// The model used for rendering this object (or NULL if object is non-renderable)
	CMPINLINE Model *						GetModel(void) 						{ return m_model.GetModel(); }
	void									SetModel(Model *model);

    // The articulated model used for rendering this object (or NULL if not applicable)
	CMPINLINE ArticulatedModel *            GetArticulatedModel(void)                       { return m_articulatedmodel; }
	CMPINLINE void                          SetArticulatedModel(ArticulatedModel *model)    { m_articulatedmodel = model; }

	// The faction this object belongs to, or Faction::NullFaction (0) for non-affiliated objects
	CMPINLINE Faction::F_ID					GetFaction(void) const				{ return m_faction; }
	CMPINLINE void							SetFaction(Faction::F_ID id)		{ m_faction = id; }

	// Return or set the offset translation required to centre the object model about its local origin
	CMPINLINE XMVECTOR						GetCentreOffsetTranslation(void) const					{ return m_centreoffset; }
	CMPINLINE void							SetCentreOffsetTranslation(const FXMVECTOR offset)		{ m_centreoffset = offset; }
	
	// Collision detection data
	CMPINLINE Game::CollisionMode			GetCollisionMode(void) const					{ return m_collisionmode; }	
	CMPINLINE void							SetCollisionMode(Game::CollisionMode mode)		{ m_collisionmode = mode; }	
	CMPINLINE Game::ColliderType			GetColliderType(void) const						{ return m_collidertype; }
	CMPINLINE void							SetColliderType(Game::ColliderType type)		{ m_collidertype = type; }

	// We store the collision sphere data per object for runtime access efficiency, considering it only requires a few floating point values
	float									GetCollisionSphereRadius(void) const			{ return m_collisionsphereradius; }
	float									GetCollisionSphereRadiusSq(void) const			{ return m_collisionsphereradiussq; }
	float									GetCollisionSphereMarginRadius(void) const		{ return m_collisionspheremarginradius; }	

	// Set a new collision sphere radius, recalcuating all derived fieds
	void									SetCollisionSphereRadius(float radius);
	void									SetCollisionSphereRadiusSq(float radius_sq);

	// Method called when a projectile (not an object, but a basic projectile) collides with this object
	// Handles any damage or effects of the collision, and also triggers rendering of appropriate effects if required
	void									HandleProjectileImpact(BasicProjectile & proj, const GamePhysicsEngine::OBBIntersectionData & impact);

	// Narrowphase-specific collision data.  We make this public to allow direct & fast manipulation of the data
	OrientedBoundingBox						CollisionOBB;				// The hierarchy of oriented bounding boxes that make up our collision mesh

	// Forces an immediate update of the object OBB.  Not normally required; OBB will be updated when it is required and when it has
	// been invalidated by some other action
	CMPINLINE void							ForceOBBUpdate(void)							{ CollisionOBB.UpdateFromObject(*this); }

	// Retrieve and set the spatial partitioning tree node this object belongs to
	CMPINLINE Octree<iObject*> *			GetSpatialTreeNode(void) const						{ return m_treenode; }
	CMPINLINE void							SetSpatialTreeNode(Octree<iObject*> * node)			{ m_treenode = node; }

	// Returns the disposition of this object towards the target object, based on our respective factions and 
	// any other modifiers (e.g. if the objects have individually attacked each other)
	Faction::FactionDisposition				GetDispositionTowardsObject(const iObject *obj) const;

	// Query or set the visibility-testing mode for this object
	CMPINLINE VisibilityTestingModeType		GetVisibilityTestingMode(void) const						{ return m_visibilitytestingmode; }
	CMPINLINE void							SetVisibilityTestingMode(VisibilityTestingModeType mode)	{ m_visibilitytestingmode = mode; }

	// Event raised when the geometry for this object changes
	void									GeometryChanged(void);

	// Ambient audio for the object
	CMPINLINE AudioParameters				GetAmbientAudio(void) const									{ return m_ambient_audio; }
	CMPINLINE bool							HasAmbientAudio(void) const									{ return (m_ambient_audio.AudioId != 0U); }
	void									SetAmbientAudio(AudioParameters audio);


	// Methods to retrieve/update attachment details; first, attachments to a parent object, where we are the child
	CMPINLINE iObject *							GetParentObject(void) const									{ return m_parentobject; }
	CMPINLINE bool								HaveParentAttachment(void) const							{ return m_parentobject != NULL; }
	void										DetachFromParent(void);
	CMPINLINE void								SetParentObjectDirect(iObject *parent)						{ m_parentobject = parent; }

	// Methods to update/retrieve attachment details for any child objects, where we are the parent
	CMPINLINE int								GetChildObjectCount(void) const								{ return m_childcount; }
	CMPINLINE bool								HasChildAttachments(void) const								{ return (m_childcount != 0); }
	CMPINLINE AttachmentSet &					GetChildObjects(void)										{ return m_childobjects; }
	void										AddChildAttachment(iObject *child);
	void										AddChildAttachment(iObject *child, const FXMVECTOR posoffset, const FXMVECTOR orientoffset);
	bool										HaveChildAttachment(iObject *child);
	Attachment<iObject*>						RetrieveChildAttachmentDetails(iObject *child);
	void										RemoveChildAttachment(iObject *child);

	// Generates a debug output of all child objects attached to this one
	std::string									ListChildren(void) const;

	// Method to update the position of any attached child objects
	void										UpdatePositionOfChildObjects(void);

	// Releases all attachments from this object
	void										ReleaseAllAttachments(void);

	// Methods for setting and testing the object update flag
	CMPINLINE bool								Simulated(void) const			{ return m_simulated.IsSet(); }
	CMPINLINE void								SetSimulatedFlag(void)			{ m_simulated.Set(); }
	CMPINLINE void								ClearSimulatedFlag(void)		{ m_simulated.Clear(); }

	// Returns a flag indicating whether this object is allowed to simulate its own movement this simulation cycle
	// It may not be if, for example, it is attached to some other parent object
	CMPINLINE bool								CanSimulateMovement(void) const
	{
		return ( !PositionUpdated() && !HaveParentAttachment() );
	}

	// Methods for setting and testing the position update flag
	CMPINLINE bool								PositionUpdated(void) const		{ return m_posupdated.IsSet(); }
	CMPINLINE void								SetPositionUpdated(void)		{ m_posupdated.Set(); }
	CMPINLINE void								ClearPositionUpdatedFlag(void)	{ m_posupdated.Clear(); }


	// Methods for setting and testing the flag that indicates whether spatial data (pos or orient) has changed since the last frame
	// NOTE: this must remain as bool, rather than using a FrameFlag, since the flag may persist across frames until the next time it is tested
	CMPINLINE bool								SpatialDataChanged(void) const	{ return m_spatialdatachanged; }
	CMPINLINE void								FlagSpatialDataChange(void)		{ m_spatialdatachanged = true; }
	CMPINLINE void								ClearSpatialChangeFlag(void)	{ m_spatialdatachanged = false; }

	// Methods for managing the list of objects that this object will NOT collide with
	CMPINLINE bool								HasCollisionExclusions(void) const		{ return (m_nocollision_count != 0); }
	CMPINLINE int								GetCollisionExclusionCount(void) const	{ return m_nocollision_count; }
	void										AddCollisionExclusion(Game::ID_TYPE object);
	void										RemoveCollisionExclusion(Game::ID_TYPE object);
	
	// Flag that indicates whether the object is currently visible.  Set by the core engine each render pass, so the flag
	// will always relate to object visibility LAST frame
	CMPINLINE bool								IsCurrentlyVisible(void) const			{ return m_currentlyvisible.IsSet(); }
	CMPINLINE void								MarkAsVisible(void)						{ m_currentlyvisible.Set(); }
	CMPINLINE void								RemoveCurrentVisibilityFlag(void)		{ m_currentlyvisible.Clear(); }
	//CMPINLINE void							SetCurrentVisibilityFlag(bool v)		{ m_currentlyvisible = v; }

	// Renormalise any object spatial data, following a change to the object position/orientation
	CMPINLINE void								RenormaliseSpatialData(void)
	{
		m_orientation = XMQuaternionNormalizeEst(m_orientation);
	}

	// Each object has a threshold travel distance (sq) per frame, above which they are considered a fast-mover that needs to be handled
	// via continuous collision detection (CCD) rather than normal discrete collision testing.  This value is recalculated whenever the
	// object size is set; it is a defined percentage of the smallest extent in each dimension (min(x,y,z).
	CMPINLINE float								GetFastMoverThresholdSq(void) const		{ return m_fastmoverthresholdsq; }

	// Indicates whether this object is excluded from colliding with the specified object
	CMPINLINE bool								CollisionExcludedWithObject(Game::ID_TYPE object)
	{
		// Perform a simple linear search; objects will only ever have a handful of exclusions (or in most cases, none)
		for (int i = 0; i < m_nocollision_count; ++i) if (m_nocollision[i] == object) return true;
		return false;
	}

	// The 'hardness' value of the object, default 1.0, that acts as e.g. a collision 
	// penetration multiplier.  Acts as a proxy for e.g. force per cross-sectional area
	CMPINLINE float								GetHardness(void) const				{ return m_hardness; }
	CMPINLINE void								SetHardness(float h)				{ m_hardness = h; }


	// Passthrough methods to the FadeEffect component to allow invocation via console command
	CMPINLINE void								FadeToAlpha(float timeperiod, float alpha)		{ Fade.FadeToAlpha(timeperiod, alpha); }
	CMPINLINE void								SetFadeAlpha(float alpha)						{ Fade.SetFadeAlpha(alpha); }

	// Show a debug visualisation of a ray intersection test with this object
	void										DebugRenderRayIntersectionTest(const BasicRay & world_ray);

	// Static method to return the string representation of an object type
	// @Dependency iObject::ObjectType
	static std::string							TranslateObjectTypeToString(iObject::ObjectType type);

	// Static methods to translate between object simulation states and their string representations
	// @Dependency iObject::ObjectSimulationState
	static std::string							TranslateSimulationStateToString(ObjectSimulationState state);
	static ObjectSimulationState				TranslateSimulationStateFromString(const std::string & state);

	// Static method to detemine the object class of an object; i.e. whether it is space- or environment-based
	// @Dependency iObject::ObjectType
	static ObjectClass							DetermineObjectClass(const iObject & object);

	// Static record of how each simulation state compares to each other
	static const ComparisonResult				SimStateRelations[][4];

	// Static method to compare simulation states and return a value indicating their relation to one another
	CMPINLINE static ComparisonResult			CompareSimulationStates(ObjectSimulationState state1, ObjectSimulationState state2)
	{
		return iObject::SimStateRelations[state1][state2];
	}

	// Moves the object to the same location, orientation, velocity etc. as the specified object.  Primarily used 
	// to perform in-place swaps of objects
	void										MoveToObjectPosition(const iObject *target_object);

	// Output debug data on the object.  Acts from this point in the hierarchy downwards
	//std::string									DebugOutput(void) const;


	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void										ProcessDebugCommand(GameConsoleCommand & command);

	// Override string stream operator
	friend std::ostream & operator<<(std::ostream &os, const iObject & obj) 
	{
		return os << obj.GetInstanceCode() << " [ID=" << obj.GetID() << ", Type=" << iObject::TranslateObjectTypeToString(obj.GetObjectType()) << "]";
	}

	// Custom debug string function
	CMPINLINE std::string						DebugString(void) const
	{
		return concat(m_instancecode)(" [ID=")(m_id)(", Type=")(iObject::TranslateObjectTypeToString(m_objecttype))("]").str();
	}

	// Custom debug string function which allows subclass data to be inserted (avoids adding another entry to the vtable
	// for a pure debug function)
	CMPINLINE std::string						DebugString(const std::string & detail) const
	{
		return concat(m_instancecode)(" [ID=")(m_id)(", Type=")(iObject::TranslateObjectTypeToString(m_objecttype))(" [")(detail)("]]").str();
	}

	// Custom debug string function which determines the subclass of this object and calls that subclass method directly.  
	// Ugly but avoids having to add an additional vtable entry for a pure debug function
	// @Dependency iObject::ObjectType
	std::string									DebugSubclassString(void) const;

	// Destructor
	virtual ~iObject(void) = 0;


protected:

	// Generates a new unique ID that has not yet been assigned to an object
	CMPINLINE static Game::ID_TYPE		GenerateNewObjectID(void)	 { return ++(iObject::InstanceCreationCount); }

	// Protected method to set the object type, which also derives and stores the object class
	void								SetObjectType(iObject::ObjectType type);

	// Sets the object instance code.  Protected to ensure that data is kept in sync.  Will handle any notification of 
	// updates to the central data collections.  Returns a value indicating whether the code could be successfully set
	bool								SetInstanceCode(const std::string & instance_code);

	// Output debug data on the object.  Internal method that passes a stringbuilder up the hierarchy for more efficient construction
	//void								DebugOutput(std::ostringstream &ss) const;


	iObject::ObjectType					m_objecttype;					// The type of object, set by each subclass on creation
	iObject::ObjectClass				m_objectclass;					// The class of object, set by each subclass on creation
	bool								m_isenvironment;				// Flag indicating whether this object is itself an environnment

	Game::ID_TYPE						m_id;							// Unique ID of this object
	std::string							m_code;							// Unique string code of the object type
	HashVal								m_codehash;						// Hash value of the object code, used for more efficient comparison
	std::string							m_name;							// Descriptive string name for the object
	std::string							m_instancecode;					// The unique instance code of the object; concatenation of [code]_[id].  Can be manually overriden
	HashVal								m_instancecodehash;				// Hash value of the instance code, used for more efficient comparison
	bool								m_standardobject;				// Flag indicating whether this is a 'standard', centrally-maintained template object
	Faction::F_ID						m_faction;						// ID of the faction this object belongs to; will be 0 for the null faction if it has no affiliation
	ModelInstance						m_model;						// Pointer to the static model for this object, which is stored in 
																		// the central collection.  Can be NULL if non-renderable.
	ArticulatedModel *                  m_articulatedmodel;             // The articulated model to use for this object, if relevant.  If NULL, the object
                                                                        // will use its static model by default

	AXMVECTOR							m_position;						// Position of the object in world space
	XMFLOAT3							m_positionf;					// Maintain copy of the position in accessible XMFLOAT3 format as well, for convenience
	AXMVECTOR							m_orientation;					// Object orientation
	AXMMATRIX							m_orientationmatrix;			// Precise orientation matrix for the object, incorporating base orientation and any adjustments
	AXMMATRIX							m_inverseorientationmatrix;		// Inverse oriented matrix, precalculated for efficiency

	Octree<iObject*> *					m_treenode;						// Stores a pointer to the spatial partitioning node we belong to

	ObjectSimulationState				m_simulationstate;				// Value indicating the extent of simulation (if any) that should be applied to this object
	ObjectSimulationState				m_nextsimulationstate;			// Any change to simulation state is stored here and takes effect on the next simulation cycle
	bool								m_visible;						// Flag indicating whether the object is rendered (may still be simulated)
	bool								m_simulationhub;				// Flag indicating whether this object forms a simulation hub
	bool								m_spatialdatachanged;			// Flag indicating whether the object position or orientation has changed this frame

	FrameFlag							m_simulated;					// Flag indicating whether the object was simulated this frame (may not include position update, if it is attached to something)
	FrameFlag							m_posupdated;					// Flag indicating whether the object position has been updated this frame (may not have been simulated, if it was moved via attachment)
	FrameFlag							m_currentlyvisible;				// Flag indicating whether the object is visible (prior frame); use to avoid render-related updates when object is not visible
	FrameFlag							m_rendered;						// Flag indicating whether the object was rendered this frame
	

	// Populated by the subclass; indicates whether any post-simulation update is implemented by the class
	bool								m_canperformpostsimulationupdate;

	AXMVECTOR							m_size;							// Size of the object in world coordinates
	XMFLOAT3							m_sizef;						// Local float representation of the object size
	float								m_size_ratio;					// Ratio of the object's largest dimension to its smallest
	Game::BoundingVolumeType			m_best_bounding_volume;			// The most appropriate bounding volume type, based on this object's size & properties
	AXMVECTOR							m_centreoffset;					// Any required offset to centre the object model about its local origin
	
	AXMMATRIX							m_worldmatrix;					// World matrix used for rendering this object
	AXMMATRIX							m_inverseworld;					// The inverse world matrix, precalculated for rendering efficiency
	bool								m_overrides_world_derivation;	// Flag indicating whether the object subclass will handle world matrix derivation
																		// instead of via the base object-level logic

	FrameFlag							m_worldcurrent;					// Indicates whether the world matrix was calculated this frame (and that m_lastworld is also relevant)
	XMMATRIX							m_lastworld;					// World transform from the prior frame.  Relevant if m_worldcalculated == true

	Game::CollisionMode					m_collisionmode;				// Value indicating how/whether this object collides with others
	float								m_collisionsphereradius;		// Radius of the object collision sphere
	float								m_collisionsphereradiussq;		// Squared radius of the collision sphere for runtime efficiency
	float								m_collisionspheremarginradius;	// Includes a margin to catch edge cases that may otherwise be missed
	Game::ColliderType					m_collidertype;					// Indicates whether this is an active or passive collider

	VisibilityTestingModeType			m_visibilitytestingmode;		// The method used to test visibility of this object

	AudioParameters						m_ambient_audio;				// Ambient audio for this object
	
	AttachmentSet						m_childobjects;					// Vector of any attachments from this object to child objects
	int									m_childcount;					// The number of child attachments, if any)
	iObject *							m_parentobject;					// A reference to our parent attachment, if any
	
	std::vector<Game::ID_TYPE>			m_nocollision;					// Array of any other objects that this object will NOT collide with
	int									m_nocollision_count;			// The number of objects that the object will not collide with
	float								m_hardness;						// The "hardness" of the object, default 1.0, that acts as e.g. a collision 
																		// penetration multiplier.  Acts as a proxy for e.g. force per cross-sectional area

	// Each object has a threshold travel distance (sq) per frame, above which they are considered a fast-mover that needs to be handled
	// via continuous collision detection (CCD) rather than normal discrete collision testing.  This value is recalculated whenever the
	// object size is set; it is a defined percentage of the smallest extent in each dimension (min(x,y,z)).
	float								m_fastmoverthresholdsq;
};

// Derives a new object world matrix
CMPINLINE void							iObject::DeriveNewWorldMatrix(void)
{
	if (OverridesWorldMatrixDerivation()) return;

	m_orientationmatrix = XMMatrixRotationQuaternion(m_orientation);
	m_inverseorientationmatrix = XMMatrixInverse(NULL, m_orientationmatrix);

	// Store the previous transform if it is genuinely from the prior frame, for use in 
	// calculating velocity buffers at render-time
	if (m_worldcurrent.WasSetInPriorFrame())
	{
		m_lastworld = m_worldmatrix;
	}

	// World = (BaseModelWorld * Rotation * Translation)
	SetWorldMatrix(XMMatrixMultiply(XMMatrixMultiply(
		m_model.GetWorldMatrix(), 
		m_orientationmatrix), 
		XMMatrixTranslationFromVector(m_position)));

	// Record the fact that we have calculated the new world matrix
	m_worldcurrent.Set();
}






#endif