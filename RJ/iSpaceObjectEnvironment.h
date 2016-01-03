#pragma once

#ifndef __iSpaceObjectEnvironmentH__
#define __iSpaceObjectEnvironmentH__

#include "Utility.h"
#include "DX11_Core.h"
#include "Ship.h"
#include "ComplexShipElement.h"
class iEnvironmentObject;
class StaticTerrain;
class NavNetwork;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class iSpaceObjectEnvironment : public ALIGN16<iSpaceObjectEnvironment>, public Ship, public iContainsComplexShipTiles
{


public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(iSpaceObjectEnvironment)

	// Default constructor
	iSpaceObjectEnvironment(void);


	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void							InitialiseCopiedObject(iSpaceObjectEnvironment *source);


	CMPINLINE ComplexShipElement*** GetElements(void) { return m_elements; }
	CMPINLINE void					SetElements(ComplexShipElement ***elements) { m_elements = elements; }

	CMPINLINE ComplexShipElement *	GetElement(const INTVECTOR3 & loc) { return GetElement(loc.x, loc.y, loc.z); }
	CMPINLINE ComplexShipElement *	GetElement(int x, int y, int z);
	CMPINLINE void					SetElement(int x, int y, int z, const ComplexShipElement *e);

	CMPINLINE ComplexShipElement *	GetElementDirect(int x, int y, int z) { return &(m_elements[x][y][z]); }
	CMPINLINE void					SetElementDirect(int x, int y, int z, const ComplexShipElement *e) { m_elements[x][y][z] = *e; }

	CMPINLINE ComplexShipElement****	GetElementsPointer(void) { return &m_elements; }

	// Methods to retrieve and manipulate the size of the environment
	CMPINLINE INTVECTOR3 			GetElementSize(void) { return m_elementsize; }
	CMPINLINE int					GetElementSizeX(void) { return m_elementsize.x; }
	CMPINLINE int					GetElementSizeY(void) { return m_elementsize.y; }
	CMPINLINE int					GetElementSizeZ(void) { return m_elementsize.z; }
	CMPINLINE INTVECTOR3 *			GetElementSizePointer(void) { return &m_elementsize; }
	CMPINLINE void					SetElementSize(const INTVECTOR3 & size) { m_elementsize = size; }
	CMPINLINE void					SetElementSize(int x, int y, int z) { m_elementsize = INTVECTOR3(x, y, z); }
	CMPINLINE void					SetElementSizeX(int x) { m_elementsize.x = x; }
	CMPINLINE void					SetElementSizeY(int y) { m_elementsize.y = y; }
	CMPINLINE void					SetElementSizeZ(int z) { m_elementsize.z = z; }

	// Vector of active objects within the ship; each CS-Element also holds a pointer to its local objects for runtime efficiency
	std::vector<ObjectReference<iEnvironmentObject>>	Objects;

	// Vector of terrain objects held within this ship; each CS-Element also holds a pointer to the terrain for runtime efficiency
	std::vector<StaticTerrain*>							TerrainObjects;

	// Standard object simulation method, used to simulate the contents of this object environment
	void							SimulateObject(void);

	// Perform the post-simulation update.  Pure virtual inherited from iObject base class
	void							PerformPostSimulationUpdate(void);

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
	CMPINLINE const XMMATRIX		GetZeroPointWorldMatrix(void)			{ return m_zeropointworldmatrix; }
	CMPINLINE const XMMATRIX		GetInverseZeroPointWorldMatrix(void)	{ return m_inversezeropointworldmatrix; }

	// Method to force an immediate recalculation of player position/orientation, for circumstances where we cannot wait until the
	// end of the frame (e.g. for use in further calculations within the same frame that require the updated data)
	CMPINLINE void					RefreshPositionImmediate(void)
	{
		// Call the base Ship class method, to recalculate Ship-related position data
		iObject::RefreshPositionImmediate();

		// Environments should also recalculate their zero-point world transforms
		DeriveZeroPointWorldMatrix();
	}

	// Adds a new object to this environment
	void							ObjectEnteringEnvironment(iEnvironmentObject *obj);

	// Removes an object from this environment
	void							ObjectLeavingEnvironment(iEnvironmentObject *obj);

	// Indicates whether this environment contains any interior simulation hubs
	CMPINLINE bool					ContainsSimulationHubs(void) const								{ return m_containssimulationhubs; }

	// Notifies the environment of whether it contains at least one interior simulation hub
	CMPINLINE void					NotifyIsContainerOfSimulationHubs(bool is_container)			{ m_containssimulationhubs = is_container; }

	// Methods triggered when tiles are added or removed from the environment
	void							ShipTileAdded(ComplexShipTile *tile);		// When a tile is added.  Virtual inherited from interface.
	void							ShipTileRemoved(ComplexShipTile *tile);		// When a tile is removed.  Virtual inherited from interface.

	// Methods to add, find or remove terrain objects in the environment
	void							AddTerrainObject(StaticTerrain *obj);
	CMPINLINE void					RemoveTerrainObject(StaticTerrain *obj)	{ RemoveTerrainObject(obj, -1); }
	void							RemoveTerrainObject(StaticTerrain *obj, std::vector<StaticTerrain*>::size_type terrainindex);
	CMPINLINE int					FindTerrainObject(StaticTerrain *obj)	{ return FindInVector<StaticTerrain*>(TerrainObjects, obj); }
	void							ClearAllTerrainObjects(void);
	void							ClearAllTerrainObjects(bool unlink);

	// Specialised method to add a new terrain object that is part of a tile.  Object will be transformed from tile-relative to
	// environment-relative position & orientation and then added to the environment as normal
	void							AddTerrainObjectFromTile(StaticTerrain *obj, ComplexShipTile *sourcetile);

	// Event to handle the movement of objects within this element-containing object.  Calculates the new element location based
	// upon the current object position.  
	void							ObjectMoved(iEnvironmentObject *object, const INTVECTOR3 & old_min_el, const INTVECTOR3 & old_max_el);

	// Event to handle the movement of objects within this element-containing object.  Accepts both old and new element locations
	// as parameters, so more efficient than other overloaded function.
	void							ObjectMoved(iEnvironmentObject *object, const INTVECTOR3 & old_min_el, const INTVECTOR3 & old_max_el, 
																			const INTVECTOR3 & new_min_el, const INTVECTOR3 & new_max_el);

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
	// Further derived classes (e.g. ships) can implement this method and then call iSpaceObjectEnvironment::SimulationStateChanged() to maintain the chain
	void							SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate);

	// When the layout (e.g. active/walkable state, connectivity) of elements is changed
	virtual void					ElementLayoutChanged(void);

	// Get a reference to the navigation network assigned to this ship
	CMPINLINE NavNetwork *			GetNavNetwork(void)				{ return m_navnetwork; }

	// Delete or simply remove the nav network. Removing the link will leave the network itself intact, just unliked - for when we 
	// copy objects and want to retain original ship's network
	void							ShutdownNavNetwork(void);
	CMPINLINE void					RemoveNavNetworkLink(void)		{ m_navnetwork = NULL; }

	// Updates the ship navigation network based on the set of elements and their properties
	void							UpdateNavigationNetwork(void);

	// Initialise the element storage based on this object's element size
	Result							InitialiseAllElements(void);

	// Reallocates the memory allocated for element storage
	Result							ReallocateElementSpace(INTVECTOR3 size);

	// Generates an element space for this object based on that of the specified object
	Result							CopyElementSpaceFromObject(iSpaceObjectEnvironment *src);

	// Copies the terrain objects from a source ship and regenerates them with pointers within this ship
	void							CopyTerrainDataFromObject(iSpaceObjectEnvironment *source);

	// Ensures that the ship element space is sufficiently large to incorporate the location specified, by reallocating 
	// if necessary.  Returns a bool indicating whether reallocation was necessary
	Result							EnsureShipElementSpaceIncorporatesLocation(INTVECTOR3 location);

	// Shutdown method to deallocate the contents of the environment
	CMPINLINE void					Shutdown(void)						{ Shutdown(true); }
	void							Shutdown(bool unlink_tiles);

protected:

	// The individual elements that make up this object
	ComplexShipElement ***			m_elements;					// E[x][y][z]

	// Size of this object, in elements
	INTVECTOR3						m_elementsize;

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

};


CMPINLINE ComplexShipElement *iSpaceObjectEnvironment::GetElement(int x, int y, int z)
{
	// Make sure the coordinates provided are valid
	if (!m_elements || x < 0 || y < 0 || z < 0 || x >= m_elementsize.x || y >= m_elementsize.y || z >= m_elementsize.z) return NULL;

	// Return the element at this location
	return &(m_elements[x][y][z]);
}

CMPINLINE void iSpaceObjectEnvironment::SetElement(int x, int y, int z, const ComplexShipElement *e)
{
	// Make sure the coordinates provided are valid
	if (x < 0 || y < 0 || z < 0 || x >= m_elementsize.x || y >= m_elementsize.y || z >= m_elementsize.z) return;

	// Set the element to a copy of this object
	m_elements[x][y][z] = (*e);
}



#endif