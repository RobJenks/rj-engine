#pragma once

#ifndef __iSpaceObjectEnvironmentH__
#define __iSpaceObjectEnvironmentH__

#include "Utility.h"
#include "DX11_Core.h"
#include "Ship.h"
#include "ComplexShipElement.h"
#include "TileAdjacency.h"
#include "EnvironmentCollision.h"
#include "SimulatedEnvironmentCollision.h"
#include "EnvironmentOBBRegion.h"
#include "BasicColourDefinition.h"
#include "EnvironmentOxygenMap.h"
#include "EnvironmentPowerMap.h"
#include "EnvironmentHullBreaches.h"
class iEnvironmentObject;
class StaticTerrain;
class NavNetwork;
class EnvironmentTree;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class iSpaceObjectEnvironment : public ALIGN16<iSpaceObjectEnvironment>, public Ship, public iContainsComplexShipTiles
{


public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(iSpaceObjectEnvironment)

	// Debug flag indicating whether we should output information on element collision tests
//#	define DEBUG_OUTPUT_ENVIRONMENT_COLLISION_TESTING
#	define DEBUG_OUTPUT_ENVIRONMENT_COLLISION_RESULT

	// Environment terrain collection
	typedef std::vector<StaticTerrain*, AlignedAllocator<StaticTerrain*, 16U>> TerrainCollection;

	// Structure holding information on a particular deck of the environment
	struct DeckInfo
	{
		int DeckNumber;			// The deck number (which may not be the same a Z-value, if there are 
								// empty levels between decks.  Corresponds to the index in m_deck_data
		int ElementZIndex;		// Z-index for all elements on this deck

		int ElementStart;		// The first and last elements in the range that cover this deck.  Elements
		int ElementEnd;			// are indexed in order of x>y>z, so all elements on the same deck will be contiguous

		DeckInfo(void) : DeckNumber(0), ElementZIndex(0), ElementStart(0), ElementEnd(0) { }
		DeckInfo(int deck, int zindex, int el_start, int el_end) : DeckNumber(deck), ElementZIndex(zindex), ElementStart(el_start), ElementEnd(el_end) { }
	};
	static const DeckInfo NULL_DECK;

	// Default constructor
	iSpaceObjectEnvironment(void);

	// Default destructor
	virtual ~iSpaceObjectEnvironment(void) = 0;

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void							InitialiseCopiedObject(iSpaceObjectEnvironment *source);


	CMPINLINE ComplexShipElement*	GetElements(void)												{ return m_elements; }
	CMPINLINE void					SetElements(ComplexShipElement *elements)						{ m_elements = elements; }

	CMPINLINE ComplexShipElement *	GetElement(const INTVECTOR3 & loc)								{ return GetElement(loc.x, loc.y, loc.z); }
	CMPINLINE ComplexShipElement *	GetElement(int x, int y, int z);
	CMPINLINE ComplexShipElement *	GetElement(int index)											{ return ((index >= 0 && index < m_elementcount) ? &m_elements[index] : NULL); }
	CMPINLINE ComplexShipElement *	GetElementByIndex(int index)									{ return GetElement(index); }

	CMPINLINE void					SetElement(const INTVECTOR3 & loc, ComplexShipElement * e)		{ SetElement(loc.x, loc.y, loc.z, e); }
	CMPINLINE void					SetElement(int x, int y, int z, ComplexShipElement * e);

	CMPINLINE ComplexShipElement &	GetElementDirect(const INTVECTOR3 & loc)						{ return m_elements[ELEMENT_INDEX(loc.x, loc.y, loc.z)]; }
	CMPINLINE ComplexShipElement &	GetElementDirect(int x, int y, int z)							{ return m_elements[ELEMENT_INDEX(x, y, z)]; }
	CMPINLINE ComplexShipElement &	GetElementDirect(int index)										{ return m_elements[index]; }

	CMPINLINE const ComplexShipElement &	GetElementDirect(const INTVECTOR3 & loc) const			{ return m_elements[ELEMENT_INDEX(loc.x, loc.y, loc.z)]; }
	CMPINLINE const ComplexShipElement &	GetElementDirect(int x, int y, int z) const 			{ return m_elements[ELEMENT_INDEX(x, y, z)]; }
	CMPINLINE const ComplexShipElement &	GetElementDirect(int index) const 						{ return m_elements[index]; }

	CMPINLINE void					SetElementDirect(const INTVECTOR3 & loc, ComplexShipElement *e)	{ m_elements[ELEMENT_INDEX(loc.x, loc.y, loc.z)] = (*e); }
	CMPINLINE void					SetElementDirect(int x, int y, int z, ComplexShipElement *e)	{ m_elements[ELEMENT_INDEX(x, y, z)] = (*e); }

	CMPINLINE const ComplexShipElement & GetConstElementDirect(int index) const						{ return m_elements[index]; }

	// Methods to retrieve and manipulate the size of the environment
	CMPINLINE INTVECTOR3 			GetElementSize(void) const										{ return m_elementsize; }
	CMPINLINE INTVECTOR3 *			GetElementSizePointer(void)										{ return &m_elementsize; }

	// Returns the total number of elements in this environment
	CMPINLINE int					GetElementCount(void) const										{ return m_elementcount; }

	// Returns the element index corresponding to the specified element location
	CMPINLINE int					GetElementIndex(const INTVECTOR3 & location)					{ return ELEMENT_INDEX(location.x, location.y, location.z); }

	// Validates whether the given element index is valid in this environment
	CMPINLINE bool					ElementIndexIsValid(int index) const							{ return (index >= 0 && index < m_elementcount); }

	// Allocates a new element space of the specified size.  Contents are copied from the existing 
	// space (as far as possible, and if relevant).  If any element space does already exist 
	// it will be deallocated first
	Result							InitialiseElements(INTVECTOR3 size);

	// Allocates a new element space of the specified size.  Contents are initialised to default starting values or
	// copied from the existing space (as far as possible, and if relevant).  If any element space does already exist 
	// it will be deallocated first
	Result							InitialiseElements(INTVECTOR3 size, bool preserve_contents);

	// Allocates a new element space of the specified size, copying data from the existing element space as 
	// far as possible based on dimensions
	Result							InitialiseElements(INTVECTOR3 size, const ComplexShipElement * const source, INTVECTOR3 source_size);

	// Vector of active objects within the ship; each CS-Element also holds a pointer to its local objects for runtime efficiency
	std::vector<ObjectReference<iEnvironmentObject>>						Objects;
	CMPINLINE std::vector<ObjectReference<iEnvironmentObject>>::size_type	GetEnvironmentObjectCount(void) const { return Objects.size(); }

	// Vector of terrain objects held within this ship; each CS-Element also holds a pointer to the terrain for runtime efficiency
	TerrainCollection														TerrainObjects;

	// Spatial partitioning tree for all objects in the environment
	EnvironmentTree * 														SpatialPartitioningTree;

	// Rebuilds the spatial partitioning tree, populating with all existing objects if relevant
	void							BuildSpatialPartitioningTree(void);

	// Standard object simulation method, used to simulate the contents of this object environment
	void							SimulateObject(void);

	// Perform the post-simulation update.  Pure virtual inherited from iObject base class
	void							PerformPostSimulationUpdate(void);

	// Set the simulation state of all environment contents
	void							SetSimulationStateOfEnvironmentContents(iObject::ObjectSimulationState state);

	// Methods to update life support-related properties of the ship; we set flags that force an update next cycle
	// Note that this method is available both for properties update in response to certain events (e.g. gravity) and 
	// those which are updated on a periodic basis; in the case of the latter, these methods force an update 
	// of the property ahead of its next scheduled update and reset the time to next update
	CMPINLINE void					UpdateGravity(void)			{ m_gravityupdaterequired = true; }
	CMPINLINE void					UpdatePower(void)			{ m_powerupdaterequired = true; }
	CMPINLINE void					UpdateOxygen(void)			{ m_oxygenupdaterequired = true; }

	// Returns the oxygen level for a specific element
	CMPINLINE Oxygen::Type			GetOxygenLevel(int element_id) const					{ return m_oxygenmap.GetOxygenLevel(element_id); }
	CMPINLINE Oxygen::Type			GetOxygenLevel(const INTVECTOR3 & location) const		{ return m_oxygenmap.GetOxygenLevel(location); }

	// Set or retrieve the zero-element translation for this environment
	CMPINLINE XMVECTOR				GetZeroPointTranslation(void) const						{ return m_zeropointtranslation; }
	CMPINLINE XMFLOAT3				GetZeroPointTranslationF(void) const					{ return m_zeropointtranslationf; }
	CMPINLINE void					SetZeroPointTranslation(const FXMVECTOR offset)			
	{ 
		m_zeropointtranslation = offset; 
		XMStoreFloat3(&m_zeropointtranslationf, m_zeropointtranslation);
	}

	// Registers a new collision with this environment, calculates the effect and begins to apply the effects
	// Returns a flag indicating whether the event was registered (there are several validations that may prevent this)
	bool							RegisterEnvironmentImpact(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact);

	// Checks whether collision of the specified object with this environment is already being simulated
	bool							EnvironmentIsCollidingWithObject(iActiveObject *object);

	// Derive the world matrix that translates to the (0,0) point of this environment.  Based upon the object world matrix and 
	// object size.  Called once we know that the environment needs to be rendered this frame; subsequent rendering methods
	// can then simply call GetZeroPointWorldMatrix() to retrieve the transformation
	void							DeriveZeroPointWorldMatrix(void)
	{
		// Determine the adjusted world matrix that incorporates the zero-element offset
		XMMATRIX zerotrans = XMMatrixTranslationFromVector(m_zeropointtranslation);
		m_zeropointworldmatrix = XMMatrixMultiply(zerotrans, m_worldmatrix);

		// Also store the inverse zero point matrix, for transforming objects from world space into this environment
		m_inversezeropointworldmatrix = XMMatrixInverse(NULL, m_zeropointworldmatrix);
	}

	// Retrieve the zero-point world matrix; should be preceded by a call to DeriveZeroPointWorldMatrix() to calculate the matrix
	CMPINLINE const XMMATRIX		GetZeroPointWorldMatrix(void)  const		{ return m_zeropointworldmatrix; }
	CMPINLINE const XMMATRIX		GetInverseZeroPointWorldMatrix(void) const 	{ return m_inversezeropointworldmatrix; }

	// Method to force an immediate recalculation of player position/orientation, for circumstances where we cannot wait until the
	// end of the frame (e.g. for use in further calculations within the same frame that require the updated data)
	CMPINLINE void					RefreshPositionImmediate(void)
	{
		// Call the base Ship class method, to recalculate Ship-related position data
		iObject::RefreshPositionImmediate();

		// Environments should also recalculate their zero-point world transforms
		DeriveZeroPointWorldMatrix();
	}

	// Updates the environment following a change to its structure, for example when adding/removing a tile
	void							UpdateEnvironment(void);

	// Flag that indicates whether environment updates are currently suspended, e.g. when adding a large set
	// of tiles in one go where we don't want to run UpdateEnvironment() after each addition
	CMPINLINE bool					IsEnvironmentUpdateSuspended(void) const		{ return m_updatesuspended; }
	CMPINLINE void					SuspendEnvironmentUpdates(void)					{ m_updatesuspended = true; }
	CMPINLINE void					ResumeEnvironmentUpdates(void)
	{
		m_updatesuspended = false;
		UpdateEnvironment();
	}

	// Identify the elements that make up this environment's outer hull
	void							BuildOuterHullModel(void);

	// Identify any hull breaches.  Dependent on outer hull model
	void							IdentifyHullBreaches(void);

	// Build detail caches on the state of environment elements
	void							BuildEnvironmentDetailCaches(void);

	// Generates a bounding box hierarchy to represent the environment, accounting for any elements that may 
	// have been destroyed
	void							BuildBoundingBoxHierarchy(void);

	// Build all environment maps (power, data, oxygen, munitions, ...)
	Result							BuildAllEnvironmentMaps(void);

	// Build a specific environment map
	void							BuildEnvironmentOxygenMap(void);
	void							BuildEnvironmentPowerMap(void);

	// Verifies all environment maps (power, data, oxygen, munitions, ...) and adjusts them as required to fit 
	// with the new environment structure
	Result							RevalidateEnvironmentMaps(void);

	// Adds a new object to this environment
	void							ObjectEnteringEnvironment(iEnvironmentObject *obj);

	// Removes an object from this environment
	void							ObjectLeavingEnvironment(iEnvironmentObject *obj);

	// Indicates whether this environment contains any interior simulation hubs
	CMPINLINE bool					ContainsSimulationHubs(void) const								{ return m_containssimulationhubs; }

	// Notifies the environment of whether it contains at least one interior simulation hub
	CMPINLINE void					NotifyIsContainerOfSimulationHubs(bool is_container)			{ m_containssimulationhubs = is_container; }

	// Virtual method to set the base properties of the environment, which must be implemented by 
	// each environment-type object
	virtual void					SetBaseEnvironmentProperties(void) = 0;

	// Methods to add, find or remove terrain objects in the environment
	void							AddTerrainObject(StaticTerrain *obj);
	void							RemoveTerrainObject(StaticTerrain *obj);
	void							ClearAllTerrainObjects(void);

	// Returns an iterator to the specified terrain object, or TerrainObjects.end() if not found
	TerrainCollection::const_iterator FindTerrainObject(StaticTerrain *obj)
	{ 
		return FindInVector<TerrainCollection, StaticTerrain*>(TerrainObjects, obj);
	}

	// Returns an iterator to the specified terrain object, or TerrainObjects.end() if not found
	TerrainCollection::const_iterator FindTerrainObject(Game::ID_TYPE id)
	{
		return (std::find_if(TerrainObjects.begin(), TerrainObjects.end(),
			[&id](const StaticTerrain *element) { return (element && element->GetID() == id); }));
	}

	// Destroy the specified terrain object(s)
	CMPINLINE void					DestroyTerrain(Game::ID_TYPE id)						{ SetTerrainDestructionState(id, true); }
	CMPINLINE void					DestroyTerrain(const std::vector<Game::ID_TYPE> & ids)	{ SetTerrainDestructionState(ids, true); }

	// Repair the specified terrain object(s)
	CMPINLINE void					RepairTerrain(Game::ID_TYPE id)							{ SetTerrainDestructionState(id, false); }
	CMPINLINE void					RepairTerrain(const std::vector<Game::ID_TYPE> & ids)	{ SetTerrainDestructionState(ids, false); }

	// Add a tile to the environment
	void							AddTile(ComplexShipTile **ppTile);

	// Events that are generated pre- and post-tile addition.  Exposed for use by subclasses as required
	virtual void					BeforeTileAdded(ComplexShipTile *tile);
	virtual void					TileAdded(ComplexShipTile *tile);

	// Remove a tile from the environment
	void							RemoveTile(ComplexShipTile *tile);

	// Events that are generated pre- and post-tile removal.  Exposed for use by subclasses as required
	virtual void					BeforeTileRemoved(ComplexShipTile *tile);
	virtual void					TileRemoved(ComplexShipTile *tile);

	// Adds all terrain objects associated with a tile to the environment
	void							AddTerrainObjectsFromTile(ComplexShipTile *tile);

	// Specialised method to add a new terrain object that is part of a tile.  Object will be transformed from tile-relative to
	// environment-relative position & orientation and then added to the environment as normal
	void							AddTerrainObjectFromTile(StaticTerrain *obj, ComplexShipTile *sourcetile);

	// Remove all terrain objects that were associated with a tile from this environment
	// Adds all terrain objects associated with a tile to the environment
	void							RemoveTerrainObjectsFromTile(ComplexShipTile *tile);

	// Copies all tiles from another object and adds the copies to this object
	Result							CopyTileDataFromObject(iContainsComplexShipTiles *src);

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
	// Further derived classes (e.g. ships) can implement this method and then call iSpaceObjectEnvironment::SimulationStateChanged() to maintain the chain
	void							SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate);

	// When the layout (e.g. active/walkable state, connectivity) of elements is changed
	virtual void					ElementLayoutChanged(void);

	// Determines the element intersected by a world-space ray.  Returns a flag indicating whether any
	// intersection does take place
	bool							DetermineElementIntersectedByRay(const Ray & ray, INTVECTOR3 & outElement);

	// Determines the element intersected by a world-space ray at the specified time t along the ray.  
	// Returns a flag indicating whether any intersection does take place
	bool							DetermineElementIntersectedByRayAtTime(const Ray & ray, float t, INTVECTOR3 & outElement);

	// Determines the element intersected by a world-space ray on the specified level.  Returns a flag indicating whether any
	// intersection does take place
	bool							DetermineElementIntersectedByRay(const Ray & ray, int level, INTVECTOR3 & outElement);

	// Determines the sequence of elements intersected by a world-space ray.  Returns a flag indicating 
	// whether any intersection does take place
	bool							DetermineElementPathIntersectedByRay(const Ray & ray, float ray_radius, ElementIntersectionData & outElements);

	// Collection of any hull breaches in this environment
	EnvironmentHullBreaches			HullBreaches;

	// Get a reference to the navigation network assigned to this ship
	CMPINLINE NavNetwork *			GetNavNetwork(void)				{ return m_navnetwork; }

	// Delete or simply remove the nav network. Removing the link will leave the network itself intact, just unliked - for when we 
	// copy objects and want to retain original ship's network
	void							ShutdownNavNetwork(void);
	CMPINLINE void					RemoveNavNetworkLink(void)		{ m_navnetwork = NULL; }

	// Updates the ship navigation network based on the set of elements and their properties
	void							UpdateNavigationNetwork(void);

	// Updates the IDs, locations, and internal links between adjacent elements
	void							UpdateElementSpaceStructure(void);

	// Rotates the element space by the specified angle
	Result							RotateElementSpace(Rotation90Degree rotation);

	// Indicates whether the specified element ID is valid for this environment
	bool							IsValidElementID(int id) const { return (id >= 0 && id < m_elementcount); }

	// Copies the terrain objects from a source ship and regenerates them with pointers within this ship
	void							CopyTerrainDataFromObject(iSpaceObjectEnvironment *source);

	// Ensures that the ship element space is sufficiently large to incorporate the location specified, by reallocating 
	// if necessary.  Returns a bool indicating whether reallocation was necessary
	Result							EnsureShipElementSpaceIncorporatesLocation(INTVECTOR3 location);

	// Returns the deck data for the deck with specified index.  Returns a reference to a "null deck" if invalid parameter
	const iSpaceObjectEnvironment::DeckInfo &
									GetDeckInformation(int deck) const;

	// Returns the element index corresponding to the supplied deck.  Default 0 if invalid parameter
	int								GetDeckIndex(int deck) const;

	// Determines the set of connections from other tiles that surround this element
	void							GetNeighbouringTiles(ComplexShipTile *tile, bool(&outConnects)[4], std::vector<TileAdjacency> & outNeighbours);

	// Updates the connection state of the specified tile based on its neighbours.  Ensures a bi-directional
	// connection is setup and that the adjacent tile is also updated
	void							UpdateTileConnectionState(ComplexShipTile **ppTile);

	// Primary method called when the object takes damage at a specified (object-local) position.  
	// Calculates modified damage value (based on e.g. damage resistances) and applies to the 
	// object hitpoints.  Damage is applied in the order in which is was added to the damage 
	// set.  Returns true if the object was destroyed by any of the damage in this damage set
	// Overrides the base iTakesDamage method to calculate per-element damage
	virtual bool					ApplyDamage(const DamageSet & damage, const GamePhysicsEngine::OBBIntersectionData & impact);

	// Shutdown method to deallocate the contents of the environment
	CMPINLINE void					Shutdown(void)						{ Shutdown(true); }
	void							Shutdown(bool unlink_tiles);

	// Converts an element index into its x/y/z location
	INTVECTOR3						ElementIndexToLocation(int index) const;
	void							ElementIndexToLocation(int index, INTVECTOR3 & outVector) const;

	// Converts an element location into its element index
	CMPINLINE int					ElementLocationToIndex(const INTVECTOR3 & location) const		{ return ELEMENT_INDEX(location.x, location.y, location.z); }
	CMPINLINE int					ElementLocationToIndex(int x, int y, int z) const				{ return ELEMENT_INDEX(x, y, z); }
	
	// Clamps an element location to the bounds of this environment
	CMPINLINE INTVECTOR3			ClampElementLocationToEnvironment(const INTVECTOR3 & loc) const
	{
		return IntVector3Clamp(loc, NULL_INTVECTOR3, m_elementsize);
	}

	// Returns the element location containing the specified position.  Unbounded, so can return an element
	// location outside the bounds of this environment
	CMPINLINE INTVECTOR3			GetElementContainingPositionUnbounded(const FXMVECTOR position) const
	{
		return Game::PhysicalPositionToElementLocation(position);
	}

	// Returns the element containing the specified position, clamped to the environment bounds
	CMPINLINE INTVECTOR3			GetClampedElementContainingPosition(const FXMVECTOR position) const
	{
		return ClampElementLocationToEnvironment(Game::PhysicalPositionToElementLocation(position));
	}

	// Returns the location of the element at the specified point within this OBB node
	INTVECTOR3						DetermineElementAtOBBLocation(const OrientedBoundingBox & obb, const FXMVECTOR obb_local_pos);

	// Returns the number of elements with the specified property
	CMPINLINE int					ElementsWithProperty(ComplexShipElement::PROPERTY prop) { return m_element_property_count[(int)prop]; }

	// Determines and applies the effect of a collision with trajectory through the environment
	// Returns a flag indicating whether a collision has occured, and data on all the collision events via "outResults"
	bool							CalculateCollisionThroughEnvironment(	iActiveObject *object, const GamePhysicsEngine::ImpactData & impact, 
																			bool external_collider, EnvironmentCollision & outResult);

	// Processes all active environment collisions at the current point in time.  Called as part of object simulation
	void							ProcessAllEnvironmentCollisions(void);

	// Processes an environment collision at the current point in time.  Determines and applies all effects since the last frame
	void							ProcessEnvironmentCollision(EnvironmentCollision & collision);

	// Triggers damage to an element (and potentially its contents).  Element may be destroyed if sufficiently damaged
	// Returns a value indicating the effect of the collision on this element
	EnvironmentCollision::ElementCollisionResult
									TriggerElementDamage(int element_id, float damage);

	// SIMULATES damage to an element (and potentially its contents).  Element may be destroyed (in the simulation) if sufficiently damaged
	void							SimulateElementDamage(int element_id, float damage) const;

	// Triggers immediate destruction of an element
	void							TriggerElementDestruction(int element_id);

	// Enable or disable the ability to simulate environment collisions
	static void						EnableEnvironmentCollisionSimulationMode(const iSpaceObjectEnvironment *env);
	static void						DisableEnvironmentCollisionSimulationMode(void);

	// Static data used when simulating environment collisions (rather than actually applying them)
	static SimulatedEnvironmentCollision		EnvironmentCollisionSimulationResults;

	// Renders a 3D overlay showing the health of each element in the environment
	void							DebugRenderElementHealth(void);
	void							DebugRenderElementHealth(int z_index);
	void							DebugRenderElementHealth(int start, int end);
	void							DebugRenderOxygenLevels(void);
	void							DebugRenderOxygenLevels(int z_index);
	void							DebugRenderOxygenLevels(int start, int end);
	void							DebugRenderPowerLevels(void);
	void							DebugRenderPowerLevels(int z_index);
	void							DebugRenderPowerLevels(int start, int end);

	// Renders a 3D overlay showing the properties of each element in the environment.  If the reference 'outLegend' is provided
	// it will be populated with a mapping from overlay colours to the corresponding property state definitions.  The render process
	// will also make use of any existing mappings if they are provided within the legend object reference
	void							DebugRenderElementState(void);
	void							DebugRenderElementState(std::unordered_map<bitstring, BasicColourDefinition> & outLegend);
	void							DebugRenderElementState(int z_index);
	void							DebugRenderElementState(int z_index, std::unordered_map<bitstring, BasicColourDefinition> & outLegend);
	void							DebugRenderElementState(int start, int end);
	void							DebugRenderElementState(int start, int end, std::unordered_map<bitstring, BasicColourDefinition> & outLegend);

	// Find all objects within a given distance of the specified object.  Object & Terrain output
	// vectors will be populated if valid pointers are supplied
	void							GetAllObjectsWithinDistance(iEnvironmentObject *focal_object, float distance,
																std::vector<iEnvironmentObject*> *outObjects, std::vector<StaticTerrain*> *outTerrain);
	void							GetAllObjectsWithinDistance(StaticTerrain *focal_object, float distance,
																std::vector<iEnvironmentObject*> *outObjects, std::vector<StaticTerrain*> *outTerrain);

	// Find all objects within a given distance of the specified location.  Object & Terrain output
	// vectors will be populated if valid pointers are supplied.  Less efficient than the method
	// which supplies a focal object, since the relevant node has to be determined based on the position
	void							GetAllObjectsWithinDistance(EnvironmentTree *spatial_tree, const FXMVECTOR position, float distance,
																std::vector<iEnvironmentObject*> *outObjects, std::vector<StaticTerrain*> *outTerrain);

	// Returns debug string information on the environment
	CMPINLINE std::string			DebugEnvironmentString(void) const
	{
		return concat("Size=")(m_elementsize.ToString())(", Elements=")(m_elementcount)(", Tiles=")(m_tilecount)(", Objects=")(Objects.size())(", Terrain=")(TerrainObjects.size()).str();
	}

	// Custom debug string function
	std::string						DebugString(void) const;

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void							ProcessDebugCommand(GameConsoleCommand & command);


protected:

	// The individual elements that make up this object
	ComplexShipElement *			m_elements;					// E[x*y*z]

	// Size of this object, in elements
	INTVECTOR3						m_elementsize;

	// Precalculated values for efficiency
	int								m_elementcount;				// Total element count
	int								m_xy_size;					// Precalculated (element_size.x * element_size.y)
	int								m_yz_size;					// Precalculated (element_size.y * element_size.z)

	// Flag that indicates whether environment updates are currently suspended, e.g. when adding a large set
	// of tiles in one go where we don't want to run UpdateEnvironment() after each addition
	bool							m_updatesuspended;

	// Flag indicating whether this environment contains at least one interior simulation hub
	bool							m_containssimulationhubs;

	// Translation from environment centre to its (0,0,0) point
	AXMVECTOR						m_zeropointtranslation;
	XMFLOAT3						m_zeropointtranslationf;

	// Adjusted world matrix, which transforms to/from the element (0,0,0) point rather than the environment centre point
	AXMMATRIX						m_zeropointworldmatrix;
	AXMMATRIX						m_inversezeropointworldmatrix;

	// The navigation network that actors will use to move around this environment
	NavNetwork *					m_navnetwork;
	
	// Flags used to indicate whether certain ship properties need to be recalculated
	bool							m_gravityupdaterequired;
	bool							m_powerupdaterequired;
	bool							m_oxygenupdaterequired;
	
	// Oxygen map and recalculation parameters
	EnvironmentOxygenMap			m_oxygenmap;					// Map holding oxygen levels for the environment
	unsigned int					m_nextoxygenupdate;				// Clock ms time that the oxygen map should next be updated
	float							m_lastoxygenupdatetime;			// Timestamp (secs) of the last oxygen update; used to perform time-dependent map updates

	// Power map for the environment
	EnvironmentPowerMap				m_powermap;

	// Methods determining when environment updates are required
	CMPINLINE bool					GravityUpdateRequired() const		{ return m_gravityupdaterequired; }
	CMPINLINE bool					PowerUpdateRequired() const			{ return m_powerupdaterequired; }
	CMPINLINE bool					OxygenUpdateRequired() const		{ return (m_oxygenupdaterequired || m_nextoxygenupdate <= Game::ClockMs); }

	// Determines the time that we should next update the environment oxygen map
	CMPINLINE unsigned int			DetermineNextOxygenUpdateTime() const { return (Game::ClockMs + Oxygen::GetOxygenUpdateInterval(m_simulationstate)); }

	// Private methods used to update key ship properties
	void							PerformGravityUpdate(void);
	void							PerformPowerUpdate(void);
	void							PerformOxygenUpdate(void);

	// The set of all active collision events
	std::vector<EnvironmentCollision>	m_collision_events;

	// Indicates whether there are any collision events currently taking place in the environment
	CMPINLINE bool					HaveActiveEnvironmentCollisionEvents(void) const		{ return !m_collision_events.empty(); }

	// Updates a tile following a change to its connection state, i.e. where it now connects to new or fewer
	// neighbouring tiles.  Accepts the address of a tile pointer and will adjust that tile pointer if
	// the tile is updated as part of the analysis.  TODO: May wish to replace with more general methods in future
	Result							UpdateTileBasedOnConnectionData(ComplexShipTile **ppOutTile);

	// Replaces one tile in the environment with another.  The old tile is not 
	// shut down or deallocated by this operation
	void							ReplaceTile(ComplexShipTile *old_tile, ComplexShipTile *new_tile);

	// Sets the size of the element space within this environment.  Protected.  Only called by
	// object methods which are handling the effects of the element space change
	void							SetElementSize(const INTVECTOR3 & size);

	// Executes the collision of an object with the specified object, as part of an envrionment collision event
	void							ExecuteElementCollision(const EnvironmentCollision::EventDetails & ev, EnvironmentCollision & collision);

	// Returns a flag indicating whether environment collisions are currently being simulated for this environment
	CMPINLINE bool					EnvironmentCollisionsAreBeingSimulated(void) const		{ return iSpaceObjectEnvironment::EnvironmentCollisionSimulationResults.EnvironmentID == m_id; }

	// Applies a damage component to the specified element.  Returns true if the damage was sufficient
	// to destroy the element
	EnvironmentCollision::ElementCollisionResult	
									ApplyDamageComponentToElement(ComplexShipElement & el, Damage damage);

	// Set the destruction state of the specified terrain object(s)
	void							SetTerrainDestructionState(Game::ID_TYPE id, bool is_destroyed);
	void							SetTerrainDestructionState(const std::vector<Game::ID_TYPE> & ids, bool is_destroyed);

	// Deallocates the object element space
	void							DeallocateElementSpace(void);

	// Determines the contiguous range of elements between the specified two elements
	INTVECTOR2						GetElementRange(const INTVECTOR3 & el1, const INTVECTOR3 & el2);

	// Determines the contiguous range of elements on the specified z-level of the environment
	INTVECTOR2						GetElementRange(int zlevel);

	// We store the number of decks in this environment, and a pointer to relevant data for each
	int								m_deckcount;
	std::vector<DeckInfo>			m_deck_data;

	// We cache the number of environment elements which have each element property
	int								m_element_property_count[ComplexShipElement::PROPERTY::PROPERTY_MAX];

	// Internal method; get all objects within a given distaance of the specified position, within the 
	// specified EnvironmentTree node
	void							_GetAllObjectsWithinDistance(EnvironmentTree *tree_node, const FXMVECTOR position, float distance,
									 							 std::vector<iEnvironmentObject*> *outObjects, std::vector<StaticTerrain*> *outTerrain);

	// Internal recursive method for building the environment OBB hierarchy.  Returns the number of child
	// regions created below this node, in the range [0 - 8]
	EnvironmentOBBRegion::RegionState	DetermineOBBRegionHierarchy(EnvironmentOBBRegion & region) const;

	// Internal method to subdivide a region into child nodes.  Return the number of subnodes created.  Output 
	// parameter returns the new subnode data
	void								SubdivideOBBRegion(EnvironmentOBBRegion & region) const;

	// Builds the compound environment OBB based on calculated region data
	void								BuildOBBFromRegionData(const EnvironmentOBBRegion & region);

	// Recursively builds each node of the OBB that matches the supplied hierarchical region structure
	void								BuildOBBNodeFromRegionData(OrientedBoundingBox & obb, const EnvironmentOBBRegion & region);

	// Static working vector for environment object search; holds nodes being considered in the search
	static std::vector<EnvironmentTree*>		m_search_nodes;
	
};


CMPINLINE ComplexShipElement *iSpaceObjectEnvironment::GetElement(int x, int y, int z)
{
	// Make sure the coordinates provided are valid
	if (!m_elements || x < 0 || y < 0 || z < 0 || x >= m_elementsize.x || y >= m_elementsize.y || z >= m_elementsize.z) return NULL;

	// Return the element at this location
	return &(m_elements[ELEMENT_INDEX(x, y, z)]);
}

CMPINLINE void iSpaceObjectEnvironment::SetElement(int x, int y, int z, ComplexShipElement *e)
{
	// Make sure the coordinates provided are valid
	if (!m_elements || x < 0 || y < 0 || z < 0 || x >= m_elementsize.x || y >= m_elementsize.y || z >= m_elementsize.z) return;

	// Set the element to a copy of this object
	m_elements[ELEMENT_INDEX(x, y, z)] = (*e);
}

// Outputs debug information on environment collision testing
#if defined(_DEBUG) && defined(DEBUG_OUTPUT_ENVIRONMENT_COLLISION_TESTING)
#	define DBG_COLLISION_TEST(text)  OutputDebugString(text)
#else
#	define DBG_COLLISION_TEST(text)
#endif

// Outputs debug information on the results of an environment collision
#if defined(_DEBUG) && defined(DEBUG_OUTPUT_ENVIRONMENT_COLLISION_RESULT)
#	define DBG_COLLISION_RESULT(text)  OutputDebugString(text)
#else
#	define DBG_COLLISION_RESULT(text)
#endif

#endif