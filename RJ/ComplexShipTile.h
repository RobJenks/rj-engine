#pragma once

#ifndef __ComplexShipTileH__
#define __ComplexShipTileH__

#include <new>
#include <vector>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "AlignedAllocator.h"
#include "Utility.h"
#include "FastMath.h"
#include "GameDataExtern.h"
#include "ModelData.h"
#include "iTakesDamage.h"
#include "RepairableObject.h"
#include "Model.h"
#include "ModelInstance.h"
#include "ComplexShipElement.h"
#include "TileConnections.h"
#include "Damage.h"
#include "FadeEffect.h"
#include "HighlightEffect.h"
#include "Power.h"
#include "GameConsoleCommand.h"
#include "ViewPortal.h"
#include "CompoundElementModel.h"
#include "InstanceFlags.h"
class TiXmlElement;
class ComplexShip;
class ComplexShipSection;
class Hardpoint;
class BoundingObject;
class Resource;
class ProductionCost;

#define DEBUG_LOGINSTANCECREATION

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ComplexShipTile : public ALIGN16<ComplexShipTile>, public iTakesDamage, public RepairableObject
{
public:

	// Static record of the highest ID value in existence, for assigning to new tiles upon registration
	static Game::ID_TYPE					InstanceCreationCount;

	// Method to generate a new unique ID, called for each new tile being instantiated
	static Game::ID_TYPE					GenerateNewUniqueID(void)	{ return (++InstanceCreationCount); }
		
	// Abstract method to return the type of tile this is.  Must be implemented by all subclasses
	virtual D::TileClass					GetClass(void) const			= 0;

	// Abstract method to make a copy of the tile and return it.  Must be implemented by all subclasses
	virtual ComplexShipTile		*			Copy(void) const				= 0;

	// Method to return the unique ID of this tile 
	CMPINLINE Game::ID_TYPE					GetID(void) const				{ return m_id; }

	// Retrieves or sets the simulation state for this tile
	CMPINLINE iObject::ObjectSimulationState		GetSimulationState(void) const								{ return m_simulationstate; }
	CMPINLINE void									SetSimulationState(iObject::ObjectSimulationState state)	{ m_simulationstate = state; }
	
	// Flag indicating whether this tile requires simulation time; while it does, the SimulateTile() method will be called every <interval>	
	CMPINLINE void						DeactivateSimulation(void)								{ m_requiressimulation = false; }
	CMPINLINE void						ActivateSimulation(void) 				
	{ 
		m_requiressimulation = true; 
		m_lastsimulation = Game::ClockMs;
	}
	CMPINLINE void						ActivateSimulation(unsigned int interval_ms)
	{
		m_requiressimulation = true;
		m_simulationinterval = interval_ms;
		m_lastsimulation = Game::ClockMs;
	}
	CMPINLINE void						SetTileSimulationRequired(bool simulation_required)		{ if (simulation_required) ActivateSimulation(); else DeactivateSimulation(); }

	// Simulation interval can be adjusted as required.  Time of last simulation is recorded every time the tile is simulated
	CMPINLINE bool						SimulationIsActive(void) const			{ return m_requiressimulation; }
	CMPINLINE unsigned int				LastSimulationTime(void) const			{ return m_lastsimulation; }
	CMPINLINE unsigned int				GetSimulationInterval(void) const		{ return m_simulationinterval; }
	CMPINLINE void						SetSimulationInterval(unsigned int ms)	{ m_simulationinterval = ms; }

	// Returns a value indicating whether this tile requires simulation; based both on the flag and also the time since last simulation
	CMPINLINE bool						RequiresSimulation(void) const			
	{ 
		return (m_requiressimulation && ((Game::ClockMs - m_lastsimulation) > m_simulationinterval));
	}
	
	// Main tile simulation method.  Passthrough to the subclass implementations, and simply maintains the update interval once it returns
	CMPINLINE void						SimulateTile(void)
	{
		// Perform tile simulation immediately
		PerformTileSimulation(Game::ClockMs - m_lastsimulation);

		// Update the time of last simulation
		m_lastsimulation = Game::ClockMs;
	}

	// Virtual method for tile subclasses to perform simulation.  Differs depending on the current simulation state of the tile
	virtual void						PerformTileSimulation(unsigned int delta_ms)			= 0;

	// Flag indicating whether the tile has been rendered this frame
	CMPINLINE bool						IsRendered(void) const { return m_rendered.IsSet(); }
	CMPINLINE void						MarkAsRendered(void) { m_rendered.Set(); }

	// Applies the effects of this tile on the underlying elements
	void								ApplyTile(void);

	// Subclass-implemented virtual method, called by base class method
	virtual void						ApplyTileSpecific(void) = 0;		

	// Abstract method to remove the contents of the tile to its parent objects.  Called upon removal.
//	void								UnapplyTile(void);					// Base class method
//	virtual void						UnapplyTileSpecific(void) = 0;		// Subclass-implemented virtual method, called by base class method

	// Methods to retrieve and set the tile location in element space
	CMPINLINE INTVECTOR3				GetElementLocation(void) const { return m_elementlocation; }
	CMPINLINE XMVECTOR					GetElementPosition(void) const { return m_elementposition; }
	CMPINLINE void						SetElementLocation(INTVECTOR3 loc) 
	{ 
		m_elementlocation = loc; 
		m_elementposition = Game::ElementLocationToPhysicalPosition(m_elementlocation);
		RecalculateTileData();
	}

	// Transforms a global element location to a tile-local location.  Assumes that the element does lie within this tile, 
	// or result is undefined 
	CMPINLINE INTVECTOR3				GetLocalElementLocation(const INTVECTOR3 & global_element_location) const
	{
		return (global_element_location - m_elementlocation);
	}

	// Returns the element index of a local element location in this tile, or -1 if the location is not valid
	CMPINLINE int						GetLocalElementIndex(const INTVECTOR3 & local_location) const
	{
		int id = ELEMENT_INDEX(local_location.x, local_location.y, local_location.z);
		return ((id >= 0 && id < (int)m_elementcount) ? id : -1);
	}

	// Methods to get and set the tile size in element space
	CMPINLINE INTVECTOR3				GetElementSize(void) const { return m_elementsize; }
	CMPINLINE INTVECTOR3 *				GetElementSizePointer(void) { return &m_elementsize; }
	void								SetElementSize(const INTVECTOR3 & size);
	CMPINLINE unsigned int				GetElementCount(void) const { return m_elementcount; }
	CMPINLINE XMVECTOR					GetWorldSize(void) const { return m_worldsize; }
	
	// Method which recalculates derived size data following a change to the element size (e.g. world size)
	void								ElementSizeChanged(void);

	// Methods to get and set the string code of this tile
	CMPINLINE std::string				GetCode(void) const { return m_code; }
	CMPINLINE void						SetCode(std::string code) { m_code = code; }

	// Methods to get and set the string name of this tile
	CMPINLINE std::string				GetName(void) const { return m_code; }
	CMPINLINE void						SetName(std::string name) { m_name = name; }

	// Methods to set or retrieve the flag determining whether this is a standard tile, or just an instance within some parent entity
	CMPINLINE bool						IsStandardTile(void) const { return m_standardtile; }
	CMPINLINE void						SetStandardTile(bool standard) { m_standardtile = standard; }

	// Methods to access and set the geometry of this ship tile
	CMPINLINE ModelInstance				GetModel(void) const { return m_model; }

	// Set the tile to use a single (specified) tile model, and perform any dependent initialisation
	void								SetSingleModel(Model *model);

	// Set the tile to use a multiple & variable-sized tile geometry.  Will deallocate any existing geometry and allocate 
	// sufficient space for geometry covering the tile element size
	void								SetMultipleModels(void);

	// Updates the object before it is rendered.  Called only when the object is processed in the render queue (i.e. not when it is out of view)
	void								PerformRenderUpdate(void);

	// Methods to retrieve and set the definition associated with this tile
	const ComplexShipTileDefinition *	GetTileDefinition(void) const;
	void								SetTileDefinition(const ComplexShipTileDefinition *definition);

	// Methods to get and set the tile class type
	CMPINLINE D::TileClass				GetTileClass(void) const		{ return m_classtype; }
	CMPINLINE int						GetTileClassIndex(void) const	{ return static_cast<int>(m_classtype); }
	CMPINLINE void						SetTileClass(D::TileClass cls)	{ m_classtype = cls; }

	// Methods to access compound model data
	CMPINLINE bool							HasCompoundModel(void) const		{ return m_multiplemodels; }
	CMPINLINE CompoundElementModel & 		GetCompoundModelSet(void)			{ return m_models; }
	CMPINLINE const CompoundElementModel &	GetCompoundModelSet(void) const		{ return m_models; }

	// Recalculate compound model data, including geometry-dependent calculations that are performed during the post-processing load sequence
	void								RecalculateCompoundModelData(void);

	// Gets or sets the rotation of this tile; contents are already rotated, this is mainly for geometry rendering & the SD
	CMPINLINE Rotation90Degree			GetRotation(void) const { return m_rotation; }
	void								SetRotation(Rotation90Degree rot);

	// Rotates the tile by the specified angle, adjusting all contents and geometry accordingly
	void								Rotate(Rotation90Degree rot);

	// Rotates all terrain objects associated with this tile by the specified angle
	void								RotateAllTerrainObjects(Rotation90Degree rotation);

	// Rotate a single terrain object owned by this tile by the given rotation, about the tile centre
	void								RotateTileTerrainObject(Terrain *terrain, Rotation90Degree rotation);

	// Transform all view portals by the same rotation
	void								RotateAllViewPortals(Rotation90Degree rot_delta);

	// Copies the basic properties of a tile from the given source
	void								CopyBasicProperties(const ComplexShipTile & source);

	// Instance rendering flags for this object
	InstanceFlags						InstanceFlags;

	// Mass of the tile
	CMPINLINE float						GetMass(void) const									{ return m_mass; }
	CMPINLINE void						SetMass(float m)									{ m_mass = m; }

	// 'Hardness' of the tile, used during collision & penetration tests.  Used to approximate e.g. force per cross-sectional area, 
	// or the density of external armour which can deflect a significant impact
	CMPINLINE float						GetHardness(void) const								{ return m_hardness; }
	CMPINLINE void						SetHardness(float h)								{ m_hardness = h; }

	// Returns the impact resistance of this tile, i.e. the remaining force it can withstand from physical 
	// impacts, with an impact point at the specified element
	float								GetImpactResistance(const ComplexShipElement & at_element) const;

	// Methods to get and recalculate the aggregate health value of this tile
	CMPINLINE float						GetAggregateHealth(void)							{ return m_aggregatehealth; }
	void								ElementHealthChanged(void);	
	void								RecalculateAggregateHealth(void);
	
	// Methods to begin and end construction on this tile, and also to test whether production is in progress right now
	void								StartConstruction(void);
	void								ConstructionComplete(void);
	bool								ConstructionIsInProgress(void);

	// Methods to add and remove progress towards the construction of this tile
	float								AddConstructionProgress(INTVECTOR3 element, const Resource *resource, float amountavailable, float timestep);
	float								RemoveConstructionProgress(INTVECTOR3 element, const Resource *resource, float amount, float timestep);

	// Deallocates any existing per-element construction progress for this tile
	void								DeallocatePerElementConstructionProgress(void);

	// Methods to get and set elements of this tile's constructed state
	CMPINLINE ProductionCost *			GetConstructedState(void)							{ return m_constructedstate; }
	CMPINLINE ProductionCost *			GetConstructedStateConst(void) const				{ return m_constructedstate; }
	ProductionCost *					GetElementConstructedState(INTVECTOR3 elpos);
	ProductionCost *					GetElementConstructedState(int x, int y, int z);
	
	// Sets the production cost & state of this tile; called upon tile creation
	void								InitialiseConstructionState(ProductionCost *state);

	// Methods to retrieve components of the element location, for efficiency
	CMPINLINE int						GetElementLocationX(void) const			{ return m_elementlocation.x; }
	CMPINLINE int						GetElementLocationY(void) const			{ return m_elementlocation.y; }
	CMPINLINE int						GetElementLocationZ(void) const			{ return m_elementlocation.z; }

	// Methods to set components of the element location directly, for efficiency
	CMPINLINE int						SetElementLocationX(int x)	{ m_elementlocation.x = x; RecalculateTileData(); }
	CMPINLINE int						SetElementLocationY(int y)	{ m_elementlocation.y = y; RecalculateTileData(); }
	CMPINLINE int						SetElementLocationZ(int z)	{ m_elementlocation.z = z; RecalculateTileData(); }
	
	// Methods to retrieve/set components of the element size, for efficiency
	CMPINLINE int						GetElementSizeX(void) const		{ return m_elementsize.x; }
	CMPINLINE int						GetElementSizeY(void) const		{ return m_elementsize.y; }
	CMPINLINE int						GetElementSizeZ(void) const		{ return m_elementsize.z; }
	CMPINLINE int						SetElementSizeX(int x)			{ m_elementsize.x = x; RecalculateTileData(); }
	CMPINLINE int						SetElementSizeY(int y)			{ m_elementsize.y = y; RecalculateTileData(); }
	CMPINLINE int						SetElementSizeZ(int z)			{ m_elementsize.z = z; RecalculateTileData(); }

	// Return the (local) centre point of the tile in world coordinates (e.g. for a 1x1x1 element tile, centre = [5.0,5.0,5.0]
	CMPINLINE XMVECTOR					GetCentrePoint(void) const		{ return m_centre_point; }

	// Recalculates the state of the tile following a change to its position/size etc.  Includes recalc of the world matrix and bounding volume
	void								RecalculateTileData(void);

	
	// Recalculates the bounding volume for this tile based on the element size in world space
	void								RecalculateBoundingVolume(void);
	CMPINLINE BoundingObject *			GetBoundingObject(void) const				{ return m_boundingbox; }

	// Handle the import of additional collision data from the models that comprise this tile
	// Import any collision data from our tile models and store as collision-terrain objects
	void								UpdateCollisionDataFromModels();
	
	// Handle the import of additional collision data from the models that comprise this tile
	// Remove any collision-terrain objects that were added to the tile based on its model data
	void								RemoveAllCollisionDataFromModels();

	// Handle the import of additional collision data from the models that comprise this tile
	// Import collision data from the single specified model, with no location or rotation offset required
	void								AddCollisionDataFromModel(const ModelInstance & model);

	// Handle the import of additional collision data from the models that comprise this tile
	// Import collision data from the single specified model, with the given element location
	// and rotation offsets applied during calculation of the collision volumes
	void								AddCollisionDataFromModel(const ModelInstance & model, const UINTVECTOR3 & element_offset, Rotation90Degree rotation_offset);


	// Get the approximate radius of a bounding sphere that encompasses this tile
	CMPINLINE float						GetBoundingSphereRadius(void) const			{ return m_bounding_radius; }

	// Retrieve or recalculate the position and transform matrix for this tile relative to its parent ship object
	CMPINLINE const XMVECTOR			GetRelativePosition(void) const				{ return m_relativeposition; }
	CMPINLINE const XMMATRIX			GetRelativePositionMatrix(void) const		{ return m_relativepositionmatrix; }
	CMPINLINE const XMMATRIX 			GetWorldMatrix(void) const					{ return m_worldmatrix; }
	void								RecalculateWorldMatrix(void);

	// Return pointer to the parent object that owns this tile
	CMPINLINE iSpaceObjectEnvironment *			GetParent(void) const				{ return m_parent; }

	// Override method; sets the parent pointer manually.  Performs no other recalculation.  Only to be used by internal
	// methods that can perform the calculation directly.  Use iSpaceObjectEnvironment::AddTile() to correctly add to a parent environment
	CMPINLINE void								OverrideParentEnvironmentReference(iSpaceObjectEnvironment *env) { m_parent = env; }

	// The set of connections that are present from this tile
	TileConnections								Connections;

	// The set of connections that are possible from this tile
	TileConnections								PossibleConnections;

	// Portals owned by this tile
	CMPINLINE std::vector<ViewPortal> &				GetPortals(void)				{ return m_portals; }
	CMPINLINE std::vector<ViewPortal>::size_type	GetPortalCount(void) const		{ return m_portalcount; }
	void											RecalculatePortalData(void);
	void											AddPortal(const ViewPortal & portal);
	void											AddPortal(ViewPortal && portal);

	// Events generated when the tile is added/removed from an environment
	void										BeforeAddedToEnvironment(iSpaceObjectEnvironment *environment);
	void										AfterAddedToEnvironment(iSpaceObjectEnvironment *environment);
	void										BeforeRemovedToEnvironment(iSpaceObjectEnvironment *environment);
	void										AfterRemovedFromEnvironment(iSpaceObjectEnvironment *environment);

	// Compiles the tile based on its definition
	Result								CompileTile(void);	

	// Validates the tile against its hard-stop requirements and returns the result
	Result								ValidateHardStopRequirements(void);

	// Compiles and validates the tile based on its definition, class & associated hard-stop criteria
	Result								CompileAndValidateTile(void);

	// Generates the geometry for this tile.  Subset of the "CompileTile()" functionality which can
	// be called separately if required
	Result								GenerateGeometry(void);

	// Static method to look up a tile definition and create a new tile based upon it
	static ComplexShipTile *			Create(std::string code);

	// Static method to create a new instance of the specified class of tile.  Creates the object only, no initialisation
	static ComplexShipTile *			New(D::TileClass cls);

	// Static base class methods to generate and read XML data for the base class portion of any tile
	static TiXmlElement *				GenerateBaseClassXML(ComplexShipTile *tile);
	static void							ReadBaseClassXML(TiXmlElement *node, ComplexShipTile *tile);

	// Static base class method to copy data for the base class portion of any tile
	static void							CopyBaseClassData(ComplexShipTile *source, ComplexShipTile *target);

	// Virtual method to read class-specific XML data for the tile
	virtual void						ReadClassSpecificXMLData(TiXmlElement *node)		= 0;

	// Virtual method for implementation by subclasses, that will generate the XML necessary to represent the tile in full
	virtual TiXmlElement *				GenerateXML(void) = 0;

	// Set the single model data for this tile.  Should not be used directly by callers; model should be instantiated via either
	// SetSingleModel() or SetMultipleModels()
	void								SetModel(Model *m);

	// Set the flag indicating whether this tile uses a compound or single model set.  Should not be used directly by callers; model 
	// should be instantiated via either SetSingleModel() or SetMultipleModels()
	CMPINLINE void						SetHasCompoundModel(bool compound) { m_multiplemodels = compound; }

	// Update instance rendering flags based on the current tile configuration
	void								DetermineInstanceRenderingFlags(void);

	// Returns a value indicating whether or not this is a primary tile.  Based on the underlying tile class
	bool								IsPrimaryTile(void);

	// Returns a value indicating whether this tile spans multiple elements
	CMPINLINE bool						SpansMultipleElements(void) const						{ return m_multielement; }

	// Flag which indicates whether the connections from this tile have been 'fixed' (true) or are allowed to update based on surroundings
	CMPINLINE bool						ConnectionsAreFixed(void) const							{ return m_connections_fixed; }
	CMPINLINE void						FixConnections(bool fix_connections)					{ m_connections_fixed = fix_connections; }

	// Static method to determine whether a given tileclass is 'infrastructural'
	static bool							IsInfrastructureTile(D::TileClass tileclass);

	// The current power level being supplied to this tile
	CMPINLINE Power::Type				GetPowerLevel(void) const								{ return m_powerlevel; }
	void								SetPowerLevel(Power::Type power);

	// Returns the % power satisfaction for this tile, in the range [0.0 1.0].  A tile with no power
	// requirement will always return a power satisfaction of 1.0
	CMPINLINE float						GetPowerSatisfaction(void) const 
	{
		if (m_powerlevel >= m_powerrequirement) return 1.0f;			// PowerLevel will always be >= 0, so this will always return if we have no power req
		return ((float)m_powerlevel / (float)m_powerrequirement);		// Meaning that we don't need to guard against power_req == 0 and Div/0 here
	}

	// Power requirement in order for the tile to be functional
	CMPINLINE Power::Type				GetPowerRequirement(void) const							{ return m_powerrequirement; }
	CMPINLINE void						SetPowerRequirement(Power::Type power)					{ m_powerrequirement = power; }

	// Indicates whether the tile is currently powered
	CMPINLINE bool						IsPowered(void) const									{ return (m_powerlevel >= m_powerrequirement); }

	// Initialise and clear connection data for the tile
	void								InitialiseConnectionState();

	// Default property set applied to all elements of the tile; element-specific changes are then made afterwards
	bitstring							DefaultProperties;

	// Effects that can be activated on this object
	FadeEffect							Fade;					// Allows the object to be faded in and out
	HighlightEffect						Highlight;				

	// Reference to the hardpoints owned by this tile
	CMPINLINE void									AddHardpointReference(const std::string & hp_ref) { m_hardpoint_refs.push_back(hp_ref); }
	CMPINLINE void									ClearAllHardpointReferences(void) { m_hardpoint_refs.clear(); }
	CMPINLINE const std::vector<std::string> &		GetHardpointReferences(void) { return m_hardpoint_refs; }

	// Determines the code that should be assigned to a hardpoint owned by this tile
	std::string							DetermineTileHardpointCode(Hardpoint *hardpoint) const;
	std::string							DetermineTileHardpointCode(const std::string & hardpoint_code) const;

	// Return a debug string representation of the tile
	CMPINLINE std::string				DebugString(void)  const		{ return concat("Tile (ID=")(m_id)(", Type=")(m_code)(")").str(); }

	// Event triggered upon destruction of the entity
	void								DestroyObject(void);

	// Destroy all terrain objects owned by this tile
	void								DestroyAllOwnedTerrain(void);

	// Shutdown method - not required for this class
	CMPINLINE void						Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	// Default constructor/copy constructor/destructor
	ComplexShipTile(void);
	ComplexShipTile(const ComplexShipTile &C);
	~ComplexShipTile(void);


protected:

	// Unique ID for this tile
	Game::ID_TYPE				m_id;

	// String code and name of the tile
	std::string					m_code;
	std::string					m_name;

	// Pointer back to the tile definition
	const ComplexShipTileDefinition * m_definition;

	// Tile class (for efficiency; this can also be retrieved from the class object by Tile>TileDefinition>TileClass
	D::TileClass				m_classtype;

	// Location and size in element space
	INTVECTOR3					m_elementlocation;
	AXMVECTOR					m_elementposition;
	INTVECTOR3					m_elementsize;
	unsigned int				m_elementcount;
	AXMVECTOR					m_worldsize;
	bool						m_multielement;

	// Position and transform matrix relative to the parent complex ship object, plus the child world matrix
	AXMVECTOR					m_relativeposition;
	AXMMATRIX					m_relativepositionmatrix;
	AXMMATRIX					m_worldmatrix;

	// Pointers to the various parents of this tile
	iSpaceObjectEnvironment *	m_parent;				// The environment that contains this tile

	// The geometry associated with this ship tile
	bool						m_multiplemodels;
	ModelInstance				m_model;
	CompoundElementModel		m_models;

	// Flag indicating whether the tile has been rendered this frame
	FrameFlag					m_rendered;

	// Bounding box encompassing this tile; used for more efficient visibility & collision testing
	BoundingObject *			m_boundingbox;

	// Approximate radius of a bounding sphere that encompasses this tile
	float						m_bounding_radius;

	// Centre point of the tile
	AXMVECTOR					m_centre_point;

	// Flag determining whether this is a standard tile, or just an instance within some parent entity
	bool						m_standardtile;

	// Rotation of this tile; contents are already rotated, this is mainly for geometry rendering & the SD
	Rotation90Degree			m_rotation;

	// Mass of the tile
	float						m_mass;

	// 'Hardness' of the tile, used during collision & penetration tests.  Used to approximate e.g. force per cross-sectional area, 
	// or the density of external armour which can deflect a significant impact
	float						m_hardness;

	// Flag which indicates whether the connections from this tile have been 'fixed' (true) or are allowed to update based on surroundings
	bool						m_connections_fixed;
	
	// Vector of unique terrain IDs, corresponding to the terrain objects within our parent environment that are 'owned' by this
	// tile.  We maintain this link so that terrain objects can be efficiently removed with the tile if required
	std::vector<Game::ID_TYPE>	m_terrain_ids;

	// Vector of hardpoint references, corresponding to the hardpoints in our environment that this tile owns
	std::vector<std::string>	m_hardpoint_refs;

	// Collection of portals from this tile
	std::vector<ViewPortal>				m_portals;
	std::vector<ViewPortal>::size_type	m_portalcount;

	// Power requirement for the tile to be functional
	Power::Type					m_powerrequirement;

	// Current power level being supplied to the tile
	Power::Type					m_powerlevel;

	// Simulation state of this tile.  Light implementation for tiles, since the ship & elements handle most of the logic.  This state 
	// just determines the extent of activity within SimulateTile()
	iObject::ObjectSimulationState	m_simulationstate;

	// Flag that determines whether this tile needs simulation; if set, the SimulateTile() method will be called each cycle 
	bool						m_requiressimulation;

	// We simulate the tile on defined (ms) intervals.  interval==0 means it will be simulated every frame
	unsigned int				m_simulationinterval;
	unsigned int				m_lastsimulation;

	// Damage modifiers, incorporated when damage is applied to underlying tiles
	DamageSet					m_damagemodifiers;

	// Aggregate health of the tile, calculated from underlying elements
	float						m_aggregatehealth;

	// Aggregate and detailed data on the constructed status of the tile
	ProductionCost *			m_constructedstate;
	ProductionCost****			m_elementconstructedstate;					// 3D array of constructed state values
	INTVECTOR3					m_constructedstate_previousallocatedsize;	// The last allocation that was made, for purposes of re/de-allocating

	// Applies the effects of this tile to a specific underlying element
	void						ApplyTileToElement(ComplexShipElement *el);

	// Processes a debug tile command from the console.  Protected since this is always called from a subclass
	void								ProcessDebugTileCommand(GameConsoleCommand & command);

public:
	// Debug variables to log instance creation
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		static long inst_con;
		static long inst_des;
	#endif

};

#endif