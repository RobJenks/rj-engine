#pragma once

#ifndef __iSpaceObjectEnvironmentH__
#define __iSpaceObjectEnvironmentH__

#include "Utility.h"
#include "DX11_Core.h"
#include "Ship.h"
#include "ComplexShipElement.h"
#include "TileAdjacency.h"
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

	// Formula to translate x/y/z coordinates into an index in the element collection
#	define ELEMENT_INDEX(_x, _y, _z) (_x + (_y * m_elementsize.x) + (_z * m_xy_size))

	// Special overloaded formula to translate x/y/z coordinates into an index in the element collection, which also
	// accepts the element space size and precalculated y/z size product.  Allows use on element spaces
	// other than our own, e.g. when allocating a new space with different dimensions
#	define ELEMENT_INDEX_EX(_x, _y, _z, _size, _size_xy) (_x + (_y * _size.x) + (_z * _size_xy))

	// Environment terrain collection
	typedef std::vector<StaticTerrain*, AlignedAllocator<StaticTerrain*, 16U>> TerrainCollection;

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

	CMPINLINE void					SetElement(const INTVECTOR3 & loc, ComplexShipElement * e)		{ SetElement(loc.x, loc.y, loc.z, e); }
	CMPINLINE void					SetElement(int x, int y, int z, ComplexShipElement * e);

	CMPINLINE ComplexShipElement &	GetElementDirect(const INTVECTOR3 & loc)						{ return m_elements[ELEMENT_INDEX(loc.x, loc.y, loc.z)]; }
	CMPINLINE ComplexShipElement &	GetElementDirect(int x, int y, int z)							{ return m_elements[ELEMENT_INDEX(x, y, z)]; }
	CMPINLINE ComplexShipElement &	GetElementDirect(int index)										{ return m_elements[index]; }

	CMPINLINE void					SetElementDirect(const INTVECTOR3 & loc, ComplexShipElement *e)	{ m_elements[ELEMENT_INDEX(loc.x, loc.y, loc.z)] = (*e); }
	CMPINLINE void					SetElementDirect(int x, int y, int z, ComplexShipElement *e)	{ m_elements[ELEMENT_INDEX(x, y, z)] = (*e); }

	// Methods to retrieve and manipulate the size of the environment
	CMPINLINE INTVECTOR3 			GetElementSize(void) const										{ return m_elementsize; }
	CMPINLINE INTVECTOR3 *			GetElementSizePointer(void)										{ return &m_elementsize; }

	// Returns the total number of elements in this environment
	CMPINLINE int					GetElementCount(void) const										{ return m_elementcount; }

	// Returns the element index corresponding to the specified element location
	CMPINLINE int					GetElementIndex(const INTVECTOR3 & location)					{ return ELEMENT_INDEX(location.x, location.y, location.z); }

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
	CMPINLINE void					UpdateGravity(void)			{ m_gravityupdaterequired = true; }
	CMPINLINE void					UpdateOxygenLevels(void)	{ m_oxygenupdaterequired = true; }

	// Set or retrieve the zero-element translation for this environment
	CMPINLINE XMVECTOR				GetZeroPointTranslation(void) const						{ return m_zeropointtranslation; }
	CMPINLINE XMFLOAT3				GetZeroPointTranslationF(void) const					{ return m_zeropointtranslationf; }
	CMPINLINE void					SetZeroPointTranslation(const FXMVECTOR offset)			
	{ 
		m_zeropointtranslation = offset; 
		XMStoreFloat3(&m_zeropointtranslationf, m_zeropointtranslation);
	}

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

	// Adds a new object to this environment
	void							ObjectEnteringEnvironment(iEnvironmentObject *obj);

	// Removes an object from this environment
	void							ObjectLeavingEnvironment(iEnvironmentObject *obj);

	// Indicates whether this environment contains any interior simulation hubs
	CMPINLINE bool					ContainsSimulationHubs(void) const								{ return m_containssimulationhubs; }

	// Notifies the environment of whether it contains at least one interior simulation hub
	CMPINLINE void					NotifyIsContainerOfSimulationHubs(bool is_container)			{ m_containssimulationhubs = is_container; }

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

	// Returns the element index corresponding to the supplied deck.  Default 0 if invalid parameter
	int								GetDeckIndex(int deck) const;

	// Determines the set of connections from other tiles that surround this element
	void							GetNeighbouringTiles(ComplexShipTile *tile, bool(&outConnects)[4], std::vector<TileAdjacency> & outNeighbours);

	// Updates the connection state of the specified tile based on its neighbours.  Ensures a bi-directional
	// connection is setup and that the adjacent tile is also updated
	void							UpdateTileConnectionState(ComplexShipTile **ppTile);

	// Shutdown method to deallocate the contents of the environment
	CMPINLINE void					Shutdown(void)						{ Shutdown(true); }
	void							Shutdown(bool unlink_tiles);

	// Converts an element index into its x/y/z location
	INTVECTOR3						ElementIndexToLocation(int index) const;
	void							ElementIndexToLocation(int index, INTVECTOR3 & outVector) const;
	
	// Returns the element containing the specified position, clamped to the environment bounds
	INTVECTOR3						GetClampedElementContainingPosition(const FXMVECTOR position)
	{
		return IntVector3Clamp(Game::PhysicalPositionToElementLocation(position), NULL_INTVECTOR3, m_elementsize);
	}

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



protected:

	// The individual elements that make up this object
	ComplexShipElement *			m_elements;					// E[x*y*z]

	// Size of this object, in elements
	INTVECTOR3						m_elementsize;

	// Precalculated values for efficiency
	int								m_xy_size;				// Product of element size in the x & y dimensions for lookup efficiency
	int								m_yz_size;				// Product of element size in the y & z dimensions for lookup efficiency
	int								m_elementcount;			// Total element count
	
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
	bool							m_oxygenupdaterequired;

	// Private methods used to update key ship properties
	void							PerformGravityUpdate(void);
	void							PerformOxygenUpdate(void);

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

	// Deallocates the object element space
	void							DeallocateElementSpace(void);

	// We store the number of decks in this environment, and a pointer to the element Z value for each
	int								m_deckcount;
	std::vector<int>				m_deck_indices;

	// Internal method; get all objects within a given distaance of the specified position, within the 
	// specified EnvironmentTree node
	void							_GetAllObjectsWithinDistance(EnvironmentTree *tree_node, const FXMVECTOR position, float distance,
									 							 std::vector<iEnvironmentObject*> *outObjects, std::vector<StaticTerrain*> *outTerrain);

	// Static working vector for environment object search; holds nodes being considered in the search
	static std::vector<EnvironmentTree*>	m_search_nodes;
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




#endif