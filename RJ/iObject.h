#pragma once

#ifndef __iObjectH__
#define __iObjectH__

#include <vector>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "GameVarsExtern.h"
#include "Utility.h"
#include "HashFunctions.h"
#include "Octree.h"
#include "OrientedBoundingBox.h"
#include "ObjectAttachment.h"
class Model;

class iObject
{
public:

	// Static record of the highest ID value in existence, for assigning new objects upon registration
	static Game::ID_TYPE					InstanceCreationCount;

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
	enum ObjectSimulationState				{ NoSimulation = 0, StrategicSimulation, TacticalSimulation, FullSimulation };

	// Define a standard type for the collection of all attachments from this object to others
	typedef std::vector<ObjectAttachment>	AttachmentSet;

	// Static modifier for the size of the collision margin around an object's collision sphere.  2.0x the collision sphere
	// to ensure all cases are caught.  If current object's sphere is larger then 
	static const float						COLLISION_SPHERE_MARGIN;

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
	CMPINLINE D3DXVECTOR3					GetPosition(void) const							{ return m_position; }
	CMPINLINE void							SetPosition(const D3DXVECTOR3 & pos)					
	{ 
		m_position = pos; 
		m_spatialdatachanged = true;
		CollisionOBB.Invalidate();
	}
	CMPINLINE void							AddDeltaPosition(const D3DXVECTOR3 & delta)		{ SetPosition(m_position + delta); }

	CMPINLINE D3DXQUATERNION				GetOrientation(void) const						{ return m_orientation; }
	CMPINLINE void							SetOrientation(const D3DXQUATERNION & orient)			
	{ 
		m_orientation = orient; 
		m_spatialdatachanged = true;
		CollisionOBB.Invalidate();
	}
	CMPINLINE void							SetOrientation(const D3DXQUATERNION *orient)	
	{ 
		m_orientation = *orient; 
		m_spatialdatachanged = true;
		CollisionOBB.Invalidate();
	}

	// The world matrix of this object
	CMPINLINE D3DXMATRIX *					GetWorldMatrix(void)				{ return &m_worldmatrix; }
	CMPINLINE D3DXMATRIX *					GetInverseWorldMatrix(void)			{ return &m_inverseworld; }
	CMPINLINE void							SetWorldMatrix(D3DXMATRIX &m)
	{
		// Store the new world matrix and calculate the inverse world matrix for rendering efficiency
		m_worldmatrix = m;
		D3DXMatrixInverse(&m_inverseworld, NULL, &m_worldmatrix);
		//CollisionOBB.Invalidate();
	}
	CMPINLINE void							SetWorldMatrix(D3DXMATRIX *m) 
	{
		// Store the new world matrix and calculate the inverse world matrix for rendering efficiency
		m_worldmatrix = *m;
		D3DXMatrixInverse(&m_inverseworld, NULL, &m_worldmatrix);
		//CollisionOBB.Invalidate();
	}

	// Method to force an immediate recalculation of player position/orientation, for circumstances where we cannot wait until the
	// end of the frame (e.g. for use in further calculations within the same frame that require the updated data)
	virtual void							RefreshPositionImmediate(void) = 0;

	// All objects must expose a method to simulate themselves.  Flag indicates whether they are allowed to simulate movement (vs if the object is attached)
	virtual void							SimulateObject(bool PermitMovement) = 0;

	// Method that indicates whether the object requires a post-simulation update or not.  Usually means the object position/orientation has changed.
	// Use this flag to minimise the number of virtual function calls required where there is nothing to be done
	CMPINLINE bool							IsPostSimulationUpdateRequired(void) const			{ return m_spatialdatachanged /* && ... && ... */; }

	// All objects should also expose a method to update object state following a simulation.  Some objects will not require this method; others
	// will need to e.g. update their world transforms, or update the position of any contained objects following a change in position
	virtual void							PerformPostSimulationUpdate(void) = 0;

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
	virtual void							Shutdown(void) = 0;

	// The size of this object in world coordinates
	CMPINLINE D3DXVECTOR3					GetSize(void) const					{ return m_size; }
	void									SetSize(const D3DXVECTOR3 & size);

	// The model used for rendering this object (or NULL if object is non-renderable)
	CMPINLINE Model *						GetModel(void)						{ return m_model; }
	CMPINLINE void							SetModel(Model *model)				{ m_model = model; }

	// Return or set the offset translation required to centre the object model about its local origin
	CMPINLINE D3DXVECTOR3					GetCentreOffsetTranslation(void) const					{ return m_centreoffset; }
	CMPINLINE void							SetCentreOffsetTranslation(const D3DXVECTOR3 & offset)	{ m_centreoffset = offset; }
	
	// Collision detection data
	CMPINLINE Game::CollisionMode			GetCollisionMode(void) const					{ return m_collisionmode; }	
	CMPINLINE void							SetCollisionMode(Game::CollisionMode mode)		{ m_collisionmode = mode; }	

	// We store the collision sphere data per object for runtime access efficiency, considering it only requires a few floating point values
	float									GetCollisionSphereRadius(void) const			{ return m_collisionsphereradius; }
	float									GetCollisionSphereRadiusSq(void) const			{ return m_collisionsphereradiussq; }
	float									GetCollisionSphereMarginRadius(void) const		{ return m_collisionspheremarginradius; }	

	// Virtual method, called when this object collides with another
	virtual void							CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact) = 0;

	// Narrowphase-specific collision data.  We make this public to allow direct & fast manipulation of the data
	OrientedBoundingBox						CollisionOBB;				// The hierarchy of oriented bounding boxes that make up our collision mesh

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
	CMPINLINE AttachmentSet *					GetChildObjects(void)										{ return &m_childobjects; }
	CMPINLINE void								AddChildAttachment(iObject *child);
	void										AddChildAttachment(iObject *child, const D3DXVECTOR3 & posoffset, const D3DXQUATERNION & orientoffset);
	bool										HaveChildAttachment(iObject *child);
	ObjectAttachment							RetrieveChildAttachmentDetails(iObject *child);
	void										RemoveChildAttachment(iObject *child);

	// Method to update the position of any attached child objects
	void										UpdatePositionOfChildObjects(void);

	// Releases all attachments from this object
	void										ReleaseAllAttachments(void);

	// Methods for setting and testing the object update flag
	CMPINLINE bool								Simulated(void) const			{ return m_simulated; }
	CMPINLINE void								SetSimulatedFlag(bool flag)		{ m_simulated = flag; }

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
	CMPINLINE void								ResetSimulationFlags(void)		{ m_simulated = m_posupdated = m_spatialdatachanged = false; }

	// Static methods which register and deregister an object from the global space object collection, using the unique ID as a lookup key
	CMPINLINE static void						RegisterObject(iObject *obj);
	CMPINLINE static void						UnregisterObject(iObject *obj);

	// Method which processes all pending register/unregister requests to update the global collection.  Executed once per frame
	static void									UpdateGlobalObjectCollection(void);

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

	D3DXVECTOR3							m_position;						// Position of the object in world space
	D3DXQUATERNION						m_orientation;					// Object orientation
	Model *								m_model;						// Returns a pointer to the model for this object, which is stored in 
																		// the central collection.  Can be NULL if non-renderable.


	ObjectSimulationState				m_simulationstate;				// Value indicating the extent of simulation (if any) that should be applied to this object
	ObjectSimulationState				m_nextsimulationstate;			// Any change to simulation state is stored here and takes effect on the next simulation cycle
	bool								m_visible;						// Flag indicating whether the object is rendered (may still be simulated)
	bool								m_simulationhub;				// Flag indicating whether this object forms a simulation hub

	bool								m_simulated;					// Flag indicating whether the object has been simulated (may not include position update, if it is attached to something)
	bool								m_posupdated;					// Flag indicating whether the object position has been updated (may not have been simulated, if it was moved via attachment)
	bool								m_spatialdatachanged;			// Flag indicating whether the object position or orientation has changed since the previous frame

	D3DXVECTOR3							m_size;							// Size of the object in world coordinates
	D3DXVECTOR3							m_centreoffset;					// Any required offset to centre the object model about its local origin
	
	D3DXMATRIX							m_worldmatrix;					// World matrix used for rendering this object
	D3DXMATRIX							m_inverseworld;					// The inverse world matrix, precalculated for rendering efficiency
	
	Game::CollisionMode					m_collisionmode;				// Value indicating how/whether this object collides with others
	float								m_collisionsphereradius;		// Radius of the object collision sphere
	float								m_collisionsphereradiussq;		// Squared radius of the collision sphere for runtime efficiency
	float								m_collisionspheremarginradius;	// Includes a margin to catch edge cases that may otherwise be missed

	VisibilityTestingModeType			m_visibilitytestingmode;		// The method used to test visibility of this object

	std::vector<ObjectAttachment>		m_childobjects;					// Vector of any attachments from this object to child objects
	int									m_childcount;					// The number of child attachments, if any)
	iObject *							m_parentobject;					// A reference to our parent attachment, if any
	
	Game::ID_TYPE *						m_nocollision;					// Array of any other objects that this object will NOT collide with
	int									m_nocollision_count;			// The number of objects that the object will not collide with
	int									m_nocollision_capacity;			// The capacity of the collision exclusion array

	// Each object has a threshold travel distance (sq) per frame, above which they are considered a fast-mover that needs to be handled
	// via continuous collision detection (CCD) rather than normal discrete collision testing.  This value is recalculated whenever the
	// object size is set; it is a defined percentage of the smallest extent in each dimension (min(x,y,z).
	float								m_fastmoverthresholdsq;


};

CMPINLINE void iObject::RegisterObject(iObject *obj)
{
	// Add to the registration list for addition to the global collection in the next cycle
	Game::RegisterList.push_back(obj);
}

CMPINLINE void iObject::UnregisterObject(iObject *obj)
{
	// Check whether this object is in fact registered with the global collection
	if (obj && Game::Objects.count(obj->GetID()) > 0)
	{
		// If it is, add it's ID to the shutdown list for removal in the next cycle.  Add the ID rather than
		// the object since this unregistering could be requested as part of object shutdown, and when the
		// unregister list is next processed the object may no longer be valid.
		Game::UnregisterList.push_back(obj->GetID());
	}
}


#endif