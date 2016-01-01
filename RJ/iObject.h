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
#include "iTakesDamage.h"
#include "Attachment.h"
#include "OrientedBoundingBox.h"
#include "GamePhysicsEngine.h"
#include "FadeEffect.h"
#include "HighlightEffect.h"
#include "Faction.h"
class Model;
class ArticulatedModel;
struct BasicProjectile;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class iObject : public ALIGN16<iObject>, public iTakesDamage
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
	enum ObjectSimulationState					{ NoSimulation = 0, StrategicSimulation, TacticalSimulation, FullSimulation };

	// Define a standard type for the collection of all attachments from this object to others
	typedef std::vector<Attachment<iObject*>, AlignedAllocator<Attachment<iObject*>, 16>>	AttachmentSet;

	// Static modifier for the size of the collision margin around an object's collision sphere.  2.0x the collision sphere
	// to ensure all cases are caught.  If current object's sphere is larger then 
	static const float							COLLISION_SPHERE_MARGIN;

	// Enumeration of all object types
	enum ObjectType {	Unknown = 0, ShipObject, SimpleShipObject, ComplexShipObject, ComplexShipSectionObject, 
						SpaceEmitterObject, ActorObject, CapitalShipPerimeterBeaconObject, ProjectileObject };

	// Enumeration of object classes
	enum ObjectClass { UnknownObjectClass = 0, SpaceObjectClass, EnvironmentObjectClass };
	
	// Constructor
	iObject(void);

	// Method to return the unique ID of this object, or to request a new ID (e.g. when copying objects)
	CMPINLINE Game::ID_TYPE					GetID(void)	const				{ return m_id; }
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
	void									OverrideInstanceCode(const std::string & icode);
	bool									TestForOverrideOfInstanceCode(void) const;

	// Flag indicating whether this is a 'standard', centrally-maintained template object
	CMPINLINE bool							IsStandardObject(void) const			{ return m_standardobject; }
	CMPINLINE void							SetIsStandardObject(bool standard)		{ m_standardobject = standard; }

	// Position and orientation; all objects exist somewhere in the world
	CMPINLINE const XMVECTOR 				GetPosition(void) const							{ return m_position; }
	CMPINLINE void							SetPosition(const FXMVECTOR pos)					
	{ 
		m_position = pos; 
		XMStoreFloat3(&m_positionf, m_position);

		m_spatialdatachanged = true;
		CollisionOBB.Invalidate();
	}
	CMPINLINE void							SetPosition(const XMFLOAT3 & pos)
	{
		m_positionf = pos;
		m_position = XMLoadFloat3(&m_positionf);

		m_spatialdatachanged = true;
		CollisionOBB.Invalidate();
	}
	CMPINLINE void							AddDeltaPosition(const FXMVECTOR delta)		
	{ 
		SetPosition(XMVectorAdd(m_position, delta));
	}

	CMPINLINE const XMVECTOR				GetOrientation(void) const						{ return m_orientation; }
	CMPINLINE void							SetOrientation(const FXMVECTOR orient)			
	{ 
		m_orientation = orient; 
		m_spatialdatachanged = true;
		CollisionOBB.Invalidate();
	}

	CMPINLINE void							SetPositionAndOrientation(const FXMVECTOR pos, const FXMVECTOR orient)
	{
		m_position = pos; m_orientation = orient;
		m_spatialdatachanged = true;
		CollisionOBB.Invalidate();
	}
	CMPINLINE void							SetPositionAndOrientation_NoInvalidation(const FXMVECTOR pos, const FXMVECTOR orient)
	{
		m_position = pos; m_orientation = orient;
		m_spatialdatachanged = true;
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


	// Methods to retrieve the (automatically-maintained) orientation matrix and its inverse
	CMPINLINE const XMMATRIX				GetOrientationMatrix(void) const		{ return m_orientationmatrix; }
	CMPINLINE const XMMATRIX				GetInverseOrientationMatrix(void) const	{ return m_inverseorientationmatrix; }

	// The world matrix of this object
	CMPINLINE XMMATRIX						GetWorldMatrix(void)				{ return m_worldmatrix; }
	CMPINLINE XMMATRIX						GetInverseWorldMatrix(void)			{ return m_inverseworld; }
	CMPINLINE void XM_CALLCONV				SetWorldMatrix(const FXMMATRIX m)
	{
		// Store the new world matrix
		m_worldmatrix = m;

		// Calculate the inverse world matrix for rendering efficiency
		m_inverseworld = XMMatrixInverse(NULL, m_worldmatrix);
	}

	// Derives a new object world matrix
	CMPINLINE void							DeriveNewWorldMatrix(void);

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
		return m_canperformpostsimulationupdate && m_spatialdatachanged /* && ... && ... */; 
	}

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

	// Virtual shutdown method that must be implemented by all objects
	virtual void							Shutdown(void);

	// The size of this object in world coordinates
	void									SetSize(const FXMVECTOR size);
	CMPINLINE XMVECTOR						GetSize(void) const					{ return m_size; }
	CMPINLINE const XMFLOAT3 &				GetSizeF(void) const				{ return m_sizef; }
	CMPINLINE float							GetSizeRatio(void) const			{ return m_size_ratio; }
	CMPINLINE Game::BoundingVolumeType		MostAppropriateBoundingVolumeType(void) const { return m_best_bounding_volume; }

	// The model used for rendering this object (or NULL if object is non-renderable)
	CMPINLINE Model *						GetModel(void)						{ return m_model; }
	CMPINLINE void							SetModel(Model *model)				{ m_model = model; }

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

	// Virtual method, called when this object collides with another
	virtual void							CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact) = 0;

	// Method called when a projectile (not an object, but a basic projectile) collides with this object
	// Handles any damage or effects of the collision, and also triggers rendering of appropriate effects if required
	void									HandleProjectileImpact(BasicProjectile & proj, GamePhysicsEngine::OBBIntersectionData & impact);

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

	// Method to update the position of any attached child objects
	void										UpdatePositionOfChildObjects(void);

	// Releases all attachments from this object
	void										ReleaseAllAttachments(void);

	// Methods for setting and testing the object update flag
	CMPINLINE bool								Simulated(void) const			{ return m_simulated; }
	CMPINLINE void								SetSimulatedFlag(bool flag)		{ m_simulated = flag; }

	// Returns a flag indicating whether this object is allowed to simulate its own movement this simulation cycle
	// It may not be if, for example, it is attached to some other parent object
	CMPINLINE bool								CanSimulateMovement(void) const
	{
		return ( !PositionUpdated() && !HaveParentAttachment() );
	}

	// Methods for setting and testing the position update flag
	CMPINLINE bool								PositionUpdated(void) const		{ return m_posupdated; }
	CMPINLINE void								SetPositionUpdated(bool flag)	{ m_posupdated = flag; }

	// Methods for setting and testing the flag that indicates whether spatial data (pos or orient) has changed since the last frame
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
	CMPINLINE bool								IsCurrentlyVisible(void) const			{ return m_currentlyvisible; }
	CMPINLINE void								MarkAsVisible(void)						{ m_currentlyvisible = true; }
	CMPINLINE void								RemoveCurrentVisibilityFlag(void)		{ m_currentlyvisible = false; }
	CMPINLINE void								SetCurrentVisibilityFlag(bool v)		{ m_currentlyvisible = v; }

	// Renormalise any object spatial data, following a change to the object position/orientation
	CMPINLINE void								RenormaliseSpatialData(void)
	{
		// Normalise every frame if the object is visible, or every *_FULLSIM changes when the object is being fully-simulated
		// but is not currently visible.  Aside from that, we do not bother renormalising to save cycles
		if (m_currentlyvisible)
		{
			m_orientation = XMQuaternionNormalizeEst(m_orientation);
		}
		else if (m_simulationstate == iObject::ObjectSimulationState::FullSimulation && 
				 ++m_orientchanges >= iObject::ORIENT_NORMALISE_THRESHOLD_FULLSIM)
		{
			m_orientation = XMQuaternionNormalizeEst(m_orientation);
			m_orientchanges = 0;
		}
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

	// Resets the simulation flags; called at the start of a simulation cycle in which this object is being simulated
	// TODO: set all at once via one bitwise call once flags are swtiched to the bitwise method?
	CMPINLINE void								ResetSimulationFlags(void)		
	{ 
		m_simulated = m_posupdated /*= m_spatialdatachanged */= false; 
	}

	// Static methods to translate between object simulation states and their string representations
	static std::string							TranslateSimulationStateToString(ObjectSimulationState state);
	static ObjectSimulationState				TranslateSimulationStateFromString(const std::string & state);

	// Static method to detemine the object class of an object; i.e. whether it is space- or environment-based
	static ObjectClass							DetermineObjectClass(const iObject & object);

	// Static record of how each simulation state compares to each other
	static const ComparisonResult				SimStateRelations[][4];

	// Static method to compare simulation states and return a value indicating their relation to one another
	CMPINLINE static ComparisonResult			CompareSimulationStates(ObjectSimulationState state1, ObjectSimulationState state2)
	{
		return iObject::SimStateRelations[state1][state2];
	}

	// Destructor
	virtual ~iObject(void);


protected:

	// Generates a new unique ID that has not yet been assigned to an object
	CMPINLINE static Game::ID_TYPE		GenerateNewObjectID(void)	 { return ++(iObject::InstanceCreationCount); }

	// Protected method to set the object type, which also derives and stores the object class
	void								SetObjectType(iObject::ObjectType type);

	// Sets the object instance code.  Protected to ensure that data is kept in sync.  Will handle any notification of 
	// updates to the central data collections
	void								SetInstanceCode(const std::string & instance_code);

	// Threshold (number of changes) for internally renormalising object quaternions, depending on state
	static const int					ORIENT_NORMALISE_THRESHOLD_FULLSIM = 100;
	//static const int					ORIENT_NORMALISE_THRESHOLD_DISTANT = 250;

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
	Model *								m_model;						// Pointer to the static model for this object, which is stored in 
																		// the central collection.  Can be NULL if non-renderable.
	ArticulatedModel *                  m_articulatedmodel;             // The articulated model to use for this object, if relevant.  If NULL, the object
                                                                        // will use its static model by default

	AXMVECTOR							m_position;						// Position of the object in world space
	XMFLOAT3							m_positionf;					// Maintain copy of the position in accessible XMFLOAT3 format as well, for convenience
	AXMVECTOR							m_orientation;					// Object orientation
	AXMMATRIX							m_orientationmatrix;			// Precise orientation matrix for the object, incorporating base orientation and any adjustments
	AXMMATRIX							m_inverseorientationmatrix;		// Inverse oriented matrix, precalculated for efficiency
	int									m_orientchanges;				// The number of orientation changes we have performed since normalising the quaternion

	Octree<iObject*> *					m_treenode;						// Stores a pointer to the spatial partitioning node we belong to

	ObjectSimulationState				m_simulationstate;				// Value indicating the extent of simulation (if any) that should be applied to this object
	ObjectSimulationState				m_nextsimulationstate;			// Any change to simulation state is stored here and takes effect on the next simulation cycle
	bool								m_visible;						// Flag indicating whether the object is rendered (may still be simulated)
	bool								m_simulationhub;				// Flag indicating whether this object forms a simulation hub

	bool								m_simulated;					// Flag indicating whether the object has been simulated (may not include position update, if it is attached to something)
	bool								m_posupdated;					// Flag indicating whether the object position has been updated (may not have been simulated, if it was moved via attachment)
	bool								m_spatialdatachanged;			// Flag indicating whether the object position or orientation has changed since the previous frame
	bool								m_currentlyvisible;				// Flag indicating whether the object is visible (last frame); use to avoid render-related updates when object is not visible
	
	// Populated by the subclass; indicates whether any post-simulation update is implemented by the class
	bool								m_canperformpostsimulationupdate;

	AXMVECTOR							m_size;							// Size of the object in world coordinates
	XMFLOAT3							m_sizef;						// Local float representation of the object size
	float								m_size_ratio;					// Ratio of the object's largest dimension to its smallest
	Game::BoundingVolumeType			m_best_bounding_volume;			// The most appropriate bounding volume type, based on this object's size & properties
	AXMVECTOR							m_centreoffset;					// Any required offset to centre the object model about its local origin
	
	AXMMATRIX							m_worldmatrix;					// World matrix used for rendering this object
	AXMMATRIX							m_inverseworld;					// The inverse world matrix, precalculated for rendering efficiency
	
	Game::CollisionMode					m_collisionmode;				// Value indicating how/whether this object collides with others
	float								m_collisionsphereradius;		// Radius of the object collision sphere
	float								m_collisionsphereradiussq;		// Squared radius of the collision sphere for runtime efficiency
	float								m_collisionspheremarginradius;	// Includes a margin to catch edge cases that may otherwise be missed
	Game::ColliderType					m_collidertype;					// Indicates whether this is an active or passive collider

	VisibilityTestingModeType			m_visibilitytestingmode;		// The method used to test visibility of this object

	AttachmentSet						m_childobjects;					// Vector of any attachments from this object to child objects
	int									m_childcount;					// The number of child attachments, if any)
	iObject *							m_parentobject;					// A reference to our parent attachment, if any
	
	std::vector<Game::ID_TYPE>			m_nocollision;					// Array of any other objects that this object will NOT collide with
	int									m_nocollision_count;			// The number of objects that the object will not collide with

	// Each object has a threshold travel distance (sq) per frame, above which they are considered a fast-mover that needs to be handled
	// via continuous collision detection (CCD) rather than normal discrete collision testing.  This value is recalculated whenever the
	// object size is set; it is a defined percentage of the smallest extent in each dimension (min(x,y,z)).
	float								m_fastmoverthresholdsq;
};

// Derives a new object world matrix
CMPINLINE void							iObject::DeriveNewWorldMatrix(void)
{
	m_orientationmatrix = XMMatrixRotationQuaternion(m_orientation);
	m_inverseorientationmatrix = XMMatrixInverse(NULL, m_orientationmatrix);
	SetWorldMatrix(XMMatrixMultiply(m_orientationmatrix, XMMatrixTranslationFromVector(m_position)));
}






#endif