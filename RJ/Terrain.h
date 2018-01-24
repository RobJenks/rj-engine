#pragma once

#ifndef __TerrainH__
#define __TerrainH__

#include "DX11_Core.h"
#include "GameDataExtern.h"
#include "Utility.h"
#include "FrameFlag.h"
#include "OrientedBoundingBox.h"
#include "iTakesDamage.h"
#include "RepairableObject.h"
#include "DynamicTerrainInteractionType.h"
class Model;
class ArticulatedModel;
class TerrainDefinition;
class EnvironmentTree;
class DynamicTerrain;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Terrain : public ALIGN16<Terrain>, public iTakesDamage, public RepairableObject
{
public:

	// Enuemration of possible sources that can generate terrain data
	enum											TerrainSourceType
	{
		NoSource = 0,
		SourcedFromModel
	};

	// Static record of the highest ID value in existence, for assigning to new tiles upon registration
	static Game::ID_TYPE							InstanceCreationCount;

	// Method to generate a new unique ID, called for each new tile being instantiated
	static Game::ID_TYPE							GenerateNewUniqueID(void)	{ return (++InstanceCreationCount); }

	// Default constructor
	Terrain();

	// Static method to create a new terrain object.  If no definition is provided, a null-model (pure collision volume) will be created
	CMPINLINE static Terrain *						Create(void) { return Terrain::Create(NULL); }
	CMPINLINE static Terrain *						Create(const std::string & def) { return Terrain::Create(D::TerrainDefinitions.Get(def)); }
	static Terrain *								Create(const TerrainDefinition *def);

	// Initialise a newly-created terrain based on the given definition
	void											InitialiseNewTerrain(const TerrainDefinition *def);

	// Get/set methods for key fields
	CMPINLINE Game::ID_TYPE							GetID(void) const								{ return m_id; }
	CMPINLINE void									SetID(Game::ID_TYPE id)							{ m_id = id; }

	CMPINLINE const TerrainDefinition *				GetDefinition(void) const						{ return m_definition; }
	void											SetDefinition(const TerrainDefinition *d);
	void											SetDefinition(const std::string & def);

	CMPINLINE iSpaceObjectEnvironment *				GetParentEnvironment(void)						{ return m_parent; }
	CMPINLINE const iSpaceObjectEnvironment *		GetParentEnvironment(void) const				{ return m_parent; }
	void											SetParentEnvironment(iSpaceObjectEnvironment *env);
	
	CMPINLINE XMVECTOR								GetPosition(void) const							{ return m_data.Centre; }
	CMPINLINE XMVECTOR								GetEnvironmentPosition(void) const				{ return m_data.Centre; }
	void											SetPosition(const FXMVECTOR pos);

	CMPINLINE XMVECTOR								GetOrientation(void) const						{ return m_orientation; }
	void											SetOrientation(const FXMVECTOR orient);
	void											ChangeOrientation(const FXMVECTOR delta);

	CMPINLINE const OrientedBoundingBox::CoreOBBData &	GetOBBData(void) const						{ return m_data; }

	CMPINLINE XMMATRIX								GetWorldMatrix(void) const						{ return m_worldmatrix; }
	CMPINLINE void RJ_XM_CALLCONV						SetWorldMatrix(const FXMMATRIX m)				{ m_worldmatrix = m; }

	CMPINLINE XMFLOAT3								GetExtentF(void) const							{ return m_data.ExtentF; }
	CMPINLINE XMVECTOR								GetExtentV(void) const							{ return m_data.ExtentV; }
	CMPINLINE void									GetExtent(AXMVECTOR_P(&outExtent)[3]) const		
	{ 
		outExtent[0] = m_data.Extent[0]; 
		outExtent[1] = m_data.Extent[1];
		outExtent[2] = m_data.Extent[2];
	}
	void											SetExtent(const FXMVECTOR e);

	CMPINLINE float									GetCollisionRadius(void) const					{ return m_collisionradius; }
	CMPINLINE float									GetCollisionSphereRadius(void) const			{ return m_collisionradius; }
	CMPINLINE float									GetCollisionRadiusSq(void) const				{ return m_collisionradiussq; }		// Read-only; derived from extent
	CMPINLINE float									GetCollisionSphereRadiusSq(void) const			{ return m_collisionradiussq; }		// Read-only; derived from extent

	CMPINLINE INTVECTOR3							GetElementLocation(void) const					{ return m_element_location; }
	CMPINLINE INTVECTOR3							GetElementRangeMin(void) const					{ return m_element_min; }
	CMPINLINE INTVECTOR3							GetElementRangeMax(void) const					{ return m_element_max; }
	
	// Returns a value indicating whether this terrain object spans multiple elements
	CMPINLINE bool									SpansMultipleElements(void) const				{ return m_multielement; }
	
	// Returns a value indicating whether this terrain object overlaps the specified element
	bool											OverlapsElement(const INTVECTOR3 & el) const;

	// Return a reference to the (shared, const) static model assigned to this terrain, if one exists, or NULL if not
	bool											HasStaticModel(void) const;
	const Model *									GetStaticModel(void) const;

	// Special case where terrain may have a per-instance articulated model, instead of the default reference to
	// a static model in the terrain definition
	CMPINLINE bool									HasArticulatedModel(void) const					{ return (m_articulated_model != NULL); }
	CMPINLINE ArticulatedModel *					GetArticulatedModel(void) const					{ return m_articulated_model; }
	void											SetArticulatedModel(ArticulatedModel *m);

	// Return a flag indicating whether this model has a model of any kind (and therefore is renderable)
	CMPINLINE bool									HasModel(void) const							{ return m_has_model; }

	// If this is a dynamic terrain object it can be promoted to access greater functionality
	CMPINLINE bool									IsDynamic(void) const							{ return m_isdynamic; }
	CMPINLINE DynamicTerrain *						ToDynamicTerrain(void)							{ return (DynamicTerrain*)this; }
	CMPINLINE const DynamicTerrain *				ToDynamicTerrain(void) const					{ return (const DynamicTerrain*)this; }

	// Indicates whether this terrain object is data-enabled
	CMPINLINE bool									IsDataEnabled(void) const						{ return m_dataenabled; }

	// Indicates whether this object can be interacted with
	CMPINLINE bool									IsUsable(void) const							{ return m_usable; }

	// Pointer to the environment tree node this terrain object resides in
	CMPINLINE EnvironmentTree *						GetEnvironmentTreeNode(void)					{ return m_env_treenode; }
	CMPINLINE void									SetEnvironmentTreeNode(EnvironmentTree *node)	{ m_env_treenode = node; }

	// Return or set the link to a parent tile, if one exists.  An ID of zero indicates there is no link
	CMPINLINE Game::ID_TYPE							GetParentTileID(void) const						{ return m_parenttile; }
	CMPINLINE void									SetParentTileID(Game::ID_TYPE ID)				{ m_parenttile = ID; }
	CMPINLINE bool									HasParentTile(void) const						{ return (m_parenttile != 0L); }

	// Return or set types of terrain source
	CMPINLINE TerrainSourceType						GetSourceType(void) const						{ return m_sourcetype; }
	CMPINLINE void									SetSourceType(TerrainSourceType source_type)	{ m_sourcetype = source_type; }
	

	// Flag indicating whether the terrain object has been rendered this frame
	CMPINLINE bool									IsRendered(void) const							{ return m_rendered.IsSet(); }
	CMPINLINE void									MarkAsRendered(void)							{ m_rendered.Set(); }

	// Flag indicating whether the terrain was rendered in this or the prior frame; necessary since most simulation
	// taking place in frame N will be using render-related data from the render cycle at the end of 
	// frame N-1 (and rendering of frame N will not take place until after simulation of frame N is complete)
	CMPINLINE bool									WasRenderedSincePriorFrame(void) const			{ return m_rendered.WasSetSincePriorFrame(); }

	// Retrieve or modify the health of this terrain object
	CMPINLINE float									GetHealth(void) const							{ return m_health; }
	void											SetHealth(float h)								{ m_health = h; }

	// Called by objects that are attempting to interact with the object.  Returns a flag indicating whether any 
	// successful interaction was possible.  Forwards control to virtual subclass methods where terrain object is eligible
	CMPINLINE bool									AttemptInteraction(iObject *interacting_object, DynamicTerrainInteractionType interaction_type);

	// Determines the vertices of the surrounding collision volume
	void											DetermineCollisionBoxVertices(iSpaceObjectEnvironment *parent, AXMVECTOR_P(&pOutVertices)[8]) const;

	// Method to postpone object updates; can be used to postpone updates until the end of multiple small adjustments
	void											PostponeUpdates(void)							{ m_postponeupdates = true; }

	// Method to resume updates following a postponement.  Will perform a recalculation upon resume
	void											ResumeUpdates(void)								
	{
		m_postponeupdates = false; 
		RecalculatePositionalData();
	}

	// Terrain object mass
	CMPINLINE float									GetMass(void) const								{ return m_mass; }
	CMPINLINE void									SetMass(float m)								{ m_mass = m; }

	// Object hardness; used as a proxy for penetration testing / force per cross-sectional area in an impact
	CMPINLINE float									GetHardness(void) const							{ return m_hardness; }
	CMPINLINE void									SetHardness(float h)							{ m_hardness = h; }

	// Returns the impact resistance of this object, i.e. the remaining force it can withstand from physical 
	// impacts, with an impact point at the specified element
	float											GetImpactResistance(void) const;


	// Recalculates the positional data for this terrain following a change to its primary pos/orient data
	void											RecalculatePositionalData(void);

	// Applies a highlight effect to the terrain object
	void											Highlight(const XMFLOAT3 & colour, float alpha) const;
	void											Highlight(const XMFLOAT4 & colour) const;

	// Event triggered upon destruction of the entity
	void											DestroyObject(void);

	// Shutdown method to deallocate resources and remove the terrain object
	void											Shutdown(void);
	
	// Custom debug string function
	std::string										DebugString(void) const;

	// Creates a copy of the terrain object and returns a pointer.  Uses default copy constructor and modifies result
	Terrain *										Copy(void) const;

	// Default destructor
	~Terrain();

protected:

	Game::ID_TYPE							m_id;								// Unique ID of the terrain object
	iSpaceObjectEnvironment *				m_parent;							// Parent environment that contains this terrain object
	const TerrainDefinition *				m_definition;						// Pointer to the definition of this terrain type, which includes model details etc.  
																				// Can be null for collision regions that have no associated visible terrain
	TerrainSourceType						m_sourcetype;						// The source that generated this terrain; default is no-owner

	OrientedBoundingBox::CoreOBBData		m_data;								// Core OBB data, used for terrain collision.  Also contains object position (as 'Centre')
	AXMVECTOR								m_orientation;						// Orientation relative to the environment
	AXMMATRIX								m_worldmatrix;						// Relative world matrix for the terrain object

	float									m_collisionradius;					// Collision radius is derived based upon the object size
	float									m_collisionradiussq;				// Precalculated squared collision radius for broadphase testing

	INTVECTOR3								m_element_location;					// Location of the terrain centre in element space
	INTVECTOR3								m_element_min, m_element_max;		// Store the range of elements that this terrain object spans
	bool									m_multielement;						// Flag indicating whether we span >1 element, for render-time efficiency

	ArticulatedModel *						m_articulated_model;				// Special case: if a terrain object has a per-instance articulated model, instead of the normal
																				// reference to a static model in the terrain definition

	bool									m_has_model;						// Flag indicating whether this terrain has any kind of renderable model

																				// TODO: All these bool flags can be consolidated to a bitstring if needed
	bool									m_isdynamic;						// If this is a dynamic terrain object it can be promoted to access greater functionality

	bool									m_dataenabled;						// Flag which indicates whether the object is data-enabled (default: false)
	bool									m_usable;							// Flag which indicates whether the object can be interacted with (default: false)

	FrameFlag								m_rendered;							// Flag indicating whether the terrain object has been rendered this frame

	EnvironmentTree *						m_env_treenode;						// Pointer to the environment tree node holding this object

	Game::ID_TYPE							m_parenttile;						// Store the ID of the tile that 'owns' this object, if relevant

	float									m_health;							// Current state of the terrain object
	float									m_mass;								// Mass of the object
	float									m_hardness;							// Measure of object strength against an impact.  Used as a proxy for penetration/force per cross-sectional area

	bool									m_postponeupdates;					// Flag indicating whether the terrain object should postpone updates until it is released again
																				// Used to make multiple adjustments without recalculating derived data each time


	// Clone method for regular static terrain objects; not applicable for dynamic terrain otherwise dynamic terrain data will be lost
	Terrain *								Clone(void) const;					

	// Update the flag which indicates whether this terrain has a model of any kind (and therefore is renderable)
	CMPINLINE void							UpdateModelFlag(void) { m_has_model = (HasStaticModel() || HasArticulatedModel()); }
};



#endif