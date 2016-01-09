#include "ErrorCodes.h"
#include "Utility.h"
#include "FastMath.h"
#include "GameVarsExtern.h"
#include "ComplexShipElement.h"
#include "iEnvironmentObject.h"
#include "ComplexShipTileDefinition.h"
#include "ComplexShipTile.h"
#include "StaticTerrain.h"
#include "Ship.h"
#include "iContainsComplexShipTiles.h"
#include "NavNetwork.h"

#include "iSpaceObjectEnvironment.h"


// Default constructor
iSpaceObjectEnvironment::iSpaceObjectEnvironment(void)
{
	// Set the flag that indicates this object is itself an environment that contains objects
	m_isenvironment = true;

	// This class does implement a post-simulation update method
	m_canperformpostsimulationupdate = true;

	// Initialise fields to defaults
	m_elements = NULL;
	m_elementsize = NULL_INTVECTOR3;
	m_containssimulationhubs = false;
	m_navnetwork = NULL;
	m_zeropointtranslation = NULL_VECTOR;
	m_zeropointtranslationf = NULL_FLOAT3;
	m_zeropointworldmatrix = m_inversezeropointworldmatrix = ID_MATRIX;
}


// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void iSpaceObjectEnvironment::InitialiseCopiedObject(iSpaceObjectEnvironment *source)
{
	// Pass control to all base classes
	Ship::InitialiseCopiedObject((Ship*)source);
	iContainsComplexShipTiles::InitialiseCopiedObject((iContainsComplexShipTiles*)source);

	/* Now perform all iSpaceObjectEnvironment-related initialisation here */

	// Initialise the environment element space with new data, since currently the pointer is copied from the source ship
	// Important: Set to NULL first, otherwise copy method will deallocate the original element space before replacing it
	this->SetElements(NULL);
	ComplexShipElement::CopyElementSpace(source, this);

	// Remove the nav network pointer in this environment, since we want to generate a new one for the environment when first required
	this->RemoveNavNetworkLink();

	// Perform an initial derivation of the world/zero point matrices, as a starting point
	RefreshPositionImmediate();
}


// Standard object simulation method, used to simulate the contents of this object environment
void iSpaceObjectEnvironment::SimulateObject(void)
{
	// Simulate all ship tiles within the environment that require simulation
	iContainsComplexShipTiles::ComplexShipTileCollection::iterator t_it_end = m_tiles[0].end();
	for (iContainsComplexShipTiles::ComplexShipTileCollection::iterator t_it = m_tiles[0].begin(); t_it != t_it_end; ++t_it)
	{
		// Test whether the tile actually requires simulation (e.g. basic tile types do not have any simulation logic, and 
		// most tiles are simulated at intervals so will not require any simulation time on most cycles)
		if ((*t_it) && (*t_it)->RequiresSimulation())
		{
			// Call the tile simulation method
			(*t_it)->SimulateTile();
		}
	}
}

// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
void iSpaceObjectEnvironment::SimulationStateChanged(iObject::ObjectSimulationState prevstate, iObject::ObjectSimulationState newstate)
{
	// Call the superclass event before proceeding
	Ship::SimulationStateChanged(prevstate, newstate);

	// If we were not being simulated, and we now are, then we may need to take some iSpaceObjectEnvironment-specific actions here
	// TODO: this will not always be true in future when we have more granular simulation states 
	if (prevstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// Update the nav network for actors to use in traversing the environment
		UpdateNavigationNetwork();
	}

	// Conversely, if we are no longer going to be simulated, we can remove the nav network etc. 
	if (newstate == iObject::ObjectSimulationState::NoSimulation)
	{
		ShutdownNavNetwork();
	}

}


// Method triggered when the layout (e.g. active/walkable state, connectivity) of elements is changed
void iSpaceObjectEnvironment::ElementLayoutChanged(void)
{
	// We want to update the ship navigation network if the element layout has changed.  
	UpdateNavigationNetwork();
}


// Perform the post-simulation update.  Pure virtual inherited from iObject base class
void iSpaceObjectEnvironment::PerformPostSimulationUpdate(void)
{
	// TOOD: Need to test for SpatialDataChanged() if the "IsPostSimulationUpdateRequired()" method starts considering
	// other fields besides the spatial data flag

	// Call the base class method
	iObject::PerformPostSimulationUpdate();

	// Simulation activities to be performed when the environment is in FULL simulation mode, and our position/orientation has changed
	// TOOD: Need to test for SpatialDataChanged() if the "IsPostSimulationUpdateRequired()" method starts considering
	// other fields besides the spatial data flag
	if (m_simulationstate == iObject::ObjectSimulationState::FullSimulation)
	{
		// Derive the zero-point world transformations for this environment, which will be used in many areas such as 
		// rendering, collision detection and object placement
		DeriveZeroPointWorldMatrix();

		// Update the position of all objects within the environment, IF we are in a high enough simulation state (otherwise, object
		// position remains undefined until we measure it, at which point world position will be accurately determined for the object)
		// We also only perform this update if our own position or orientation has changed.  Environment objects will update their own
		// world position if their environment pos/orient changes.  The update here is only required if our environment pos/orient changes,
		// since in this case we need to update their world position based on OUR change in pos/orient
		std::vector<ObjectReference<iEnvironmentObject>>::iterator o_it_end = Objects.end();
		for (std::vector<ObjectReference<iEnvironmentObject>>::iterator o_it = Objects.begin(); o_it != o_it_end; ++o_it)
		{
			// Have the object update its own world position & orientation
			if ((*o_it)()) (*o_it)()->RecalculateEnvironmentPositionAndOrientationData();
		}
	}
}

// Adds a new object to this environment
void iSpaceObjectEnvironment::ObjectEnteringEnvironment(iEnvironmentObject *obj)
{
	// Make sure the object is valid
	if (!obj) return;

	// Add to the main collection of objects in this environment, assuming it does not already exist
	if (Objects.end() == std::find_if(Objects.begin(), Objects.end(),
		[&obj](const ObjectReference<iEnvironmentObject> & element) { return (element() == obj); }))
	{
		Objects.push_back(ObjectReference<iEnvironmentObject>(obj));
	}
}

// Removes an object from this environment
void iSpaceObjectEnvironment::ObjectLeavingEnvironment(iEnvironmentObject *obj)
{
	// Make sure the object is valid
	if (!obj) return;

	// Remove from the environment object collection
	while (true)
	{
		std::vector<ObjectReference<iEnvironmentObject>>::iterator it = std::find_if(Objects.begin(), Objects.end(),
			[&obj](const ObjectReference<iEnvironmentObject> & element) { return (element() == obj); });
		if (it == Objects.end()) break;

		Objects.erase(it);
	}
}

// Adds a terrain object to the environment
void iSpaceObjectEnvironment::AddTerrainObject(StaticTerrain *obj)
{
	// Make sure the terrain object is valid
	if (!obj) return;

	// Make sure the terrain object does not already exist in this environment
	if (std::find(TerrainObjects.begin(), TerrainObjects.end(), obj) != TerrainObjects.end()) return;

	// Add to the terrain collection, and set the reverse terrain pointer to this environment
	TerrainObjects.push_back(obj);
	obj->SetParentEnvironment(this);
}

// Removes a terrain object from the environment.  Optionally takes a second parameter indicating the index of this 
// object in the terrain collection; if set, and if the index is correct, it will be used rather than performing
// a search of the collection for the object.  Deallocates the terrain object.
void iSpaceObjectEnvironment::RemoveTerrainObject(StaticTerrain *obj)
{
	// Make sure the terrain object is valid
	if (!obj) return;

	// Remove the reverse terrain pointer back to its environment
	obj->SetParentEnvironment(NULL);

	// Remove from the terrain object collection
	std::vector<StaticTerrain*>::iterator it = std::find(TerrainObjects.begin(), TerrainObjects.end(), obj);
	if (it != TerrainObjects.end()) TerrainObjects.erase(it);

	// Finally, deallocate the terrain object
	SafeDelete(obj);
}

// Removes all terrain objects from an environment
void iSpaceObjectEnvironment::ClearAllTerrainObjects(void)
{
	// Deallocate all terrain objects and then clear the collection
	std::vector<StaticTerrain*>::iterator it_end = TerrainObjects.end();
	for (std::vector<StaticTerrain*>::iterator it = TerrainObjects.begin(); it != it_end; ++it)
	{
		if (*it) (delete (*it));
	}
	TerrainObjects.clear();
}

// Specialised method to add a new terrain object that is part of a tile.  Object will be transformed from tile-relative to
// environment-relative position & orientation and then added to the environment as normal
void iSpaceObjectEnvironment::AddTerrainObjectFromTile(StaticTerrain *obj, ComplexShipTile *sourcetile)
{
	// Parameter check
	if (!obj) return;
	if (!sourcetile) { AddTerrainObject(obj); return; }

	// Suspend updates while fields are updated
	obj->PostponeUpdates();

	// If the tile has a non-ID orientation, apply that orientation change to the object
	if (sourcetile->GetRotation() != Rotation90Degree::Rotate0)
	{
		// First adjust the terrain relative position, by rotating it about the local tile centre by the tile orientation
		XMVECTOR ctrans, localpos;XMMATRIX ct, invct, transform;
		
		// We need to translate to/from the tile centre before rotating, since (0,0,0) represents the top-left corner
		ctrans = XMVectorScale(Game::ElementLocationToPhysicalPosition(sourcetile->GetElementSize()), 0.5f);
		ct = XMMatrixTranslationFromVector(XMVectorScale(ctrans, -0.5f));
		invct = XMMatrixTranslationFromVector(XMVectorScale(ctrans, 0.5f));
		transform = XMMatrixMultiply(XMMatrixMultiply(
			ct,	
			GetRotationMatrix(sourcetile->GetRotation())),
			invct);

		// Transform the object position by this matrix
		localpos = XMVector3TransformCoord(obj->GetPosition(), transform);
		obj->SetPosition(localpos);

		// Now adjust the terrain orientation itself to account for the tile orientation
		obj->SetOrientation(XMQuaternionMultiply(obj->GetOrientation(), GetRotationQuaternion(sourcetile->GetRotation())));
	}

	// Transform the object position from tile- to environment-space
	obj->SetPosition(XMVectorAdd(Game::ElementLocationToPhysicalPosition(sourcetile->GetElementLocation()), obj->GetPosition()));

	// Resume updates following these changes
	obj->ResumeUpdates();

	// Finally call the main AddTerrainObject method, now that the terrain object has been transformed to environment-space
	AddTerrainObject(obj);
}

// Method to handle the addition of a ship tile to this environment
void iSpaceObjectEnvironment::ShipTileAdded(ComplexShipTile *tile)
{
	// Parameter check
	if (!tile) return;
	ComplexShipTileDefinition *def = tile->GetTileDefinition();

	// If the tile definition specifies any terrain objects we want to add them to the environment now, and also
	// store references to the terrain instance IDs within the tile instance to maintain the link 
	if (def)
	{
		std::vector<StaticTerrain*>::const_iterator it_end = def->TerrainObjects.end();
		for (std::vector<StaticTerrain*>::const_iterator it = def->TerrainObjects.begin(); it != it_end; ++it)
		{
			// Create a copy of the terrain object, if it is valid
			if (!(*it)) continue;
			StaticTerrain *terrain = (*it)->Copy();
			if (!terrain) continue;

			// Add this terrain object to the environment; method will determine the correct ship-relative position & orientation
			// based on the tile-relative position & orientation currently stored in the terrain object
			AddTerrainObjectFromTile(terrain, tile);

			// Add a reference from the terrain object to its parent tile object
			terrain->SetParentTileID(tile->GetID());

			// Add a reference from the tile to the terrain being added.  The tile cleared and initialised its storage
			// for terrain links at the point we began the tile assignment, so we can simply add the link here
			tile->AddTerrainObjectLink(terrain->GetID());
		}
	}
}

// Method to handle the removal of a ship tile from this environment
void iSpaceObjectEnvironment::ShipTileRemoved(ComplexShipTile *tile)
{
	// Parameter check
	if (!tile) return;
	
	// Remove any terrain objects in the environment that were owned by this tile
	Game::ID_TYPE id = tile->GetID();
	std::vector<StaticTerrain*>::iterator it = std::partition(TerrainObjects.begin(), TerrainObjects.end(),
		[&id](const StaticTerrain *element) { return (element->GetParentTileID() != id); });
	delete_erase<StaticTerrain*>(TerrainObjects, it, TerrainObjects.end());
}

// Updates the ship navigation network based on the set of elements and their properties
void iSpaceObjectEnvironment::UpdateNavigationNetwork(void)
{
	// Make sure the network exists.  If it doesn't, create the network object first
	if (!m_navnetwork) m_navnetwork = new NavNetwork();

	// Initialise the nav network with data from this complex ship
	m_navnetwork->InitialiseNavNetwork(this);

	// TODO: Find any actors currently following a path provided by the previous network, and have them recalculate their paths
}

void iSpaceObjectEnvironment::ShutdownNavNetwork(void)
{
	if (m_navnetwork)
	{
		m_navnetwork->Shutdown();
		SafeDelete(m_navnetwork);
	}
}

// Allocates and initialises storage for elements based on the element size properties.  
Result iSpaceObjectEnvironment::InitialiseAllElements(void)
{
	// Generate the new element space for this ship section
	return ComplexShipElement::CreateElementSpace(this, &m_elements, m_elementsize);
}

// Reallocates the memory allocated for element storage
Result iSpaceObjectEnvironment::ReallocateElementSpace(INTVECTOR3 size)
{
	Result result;
	ComplexShipElement ***el = NULL;

	// Validate parameters
	if (size.x < 0 || size.y < 0 || size.z < 0) return ErrorCodes::CannotReallocateElementSpaceWithInvalidParams;

	// Useful efficiency measure; if the size has not changed then simply return success now
	if (size == m_elementsize) return ErrorCodes::NoError;

	// If there is no element space to begin with, we instead branch to the method that will create one from scratch
	if (!m_elements) return InitialiseAllElements();

	// Otherwise, we first need to allocate memory for the new element space
	result = ComplexShipElement::AllocateElementStorage(&el, size);
	if (result != ErrorCodes::NoError) return result;

	// Now initialise this element space using our current element space as a copy source
	result = ComplexShipElement::InitialiseElementStorage(el, size, this, this);
	if (result != ErrorCodes::NoError)
	{
		if (el) ComplexShipElement::DeallocateElementStorage(&el, size);
		return result;
	}

	// We have successfully created the new element space, so delete the existing space and replace it with the new one
	ComplexShipElement::DeallocateElementStorage(&m_elements, m_elementsize);
	m_elements = el;
	m_elementsize = size;

	// Return success
	return ErrorCodes::NoError;
}

// Generates an element space for this object based on that of the specified object
Result iSpaceObjectEnvironment::CopyElementSpaceFromObject(iSpaceObjectEnvironment *src)
{
	// Directly call the copy method, which will perform null/error handling itself and return the result
	return (ComplexShipElement::CopyElementSpace(src, this));
}

// Ensures that the ship element space is sufficiently large to incorporate the location specified, by reallocating 
// if necessary.  Returns a bool indicating whether reallocation was necessary
Result iSpaceObjectEnvironment::EnsureShipElementSpaceIncorporatesLocation(INTVECTOR3 location)
{
	// If the element space currently includes this location then there is no change necessary
	if (location.x < m_elementsize.x && location.y < m_elementsize.y && location.z < m_elementsize.z)
		return ErrorCodes::NoError;

	// Determine the maximum extent in each dimension
	INTVECTOR3 extent = INTVECTOR3(max(m_elementsize.x, location.x), max(m_elementsize.y, location.y), max(m_elementsize.z, location.z));

	// Attempt to reallocate as necessary to accomodate this space
	return ReallocateElementSpace(extent);
}

// Copies the terrain objects from a source ship and regenerates them with pointers within this ship
void iSpaceObjectEnvironment::CopyTerrainDataFromObject(iSpaceObjectEnvironment *source)
{
	// Parameter check
	if (!source) return;

	// Iterate through each terrain object in the source ship in turn
	std::vector<StaticTerrain*>::const_iterator it_end = source->TerrainObjects.end();
	for (std::vector<StaticTerrain*>::const_iterator it = source->TerrainObjects.begin(); it != it_end; ++it)
	{
		// Make a copy of the terrain object, then add it to this ship
		AddTerrainObject((*it)->Copy());
	}
}

// Shutdown method to deallocate the contents of the environment
//    unlink_tiles - will generally be set to 'true'.  May be set to 'false' in case of application shutdown where
//                   we do not need to do a controlled unlinking, and where parent objects may be deallocated in 
//                   any order before this method runs.  In that case we should not attempt to unlink from them.
void iSpaceObjectEnvironment::Shutdown(bool unlink_tiles)
{
	// Deallocate all tile data
	ShutdownAllTileData(true, unlink_tiles);

	// Detach and deallocate the navigation network assigned to this ship
	ShutdownNavNetwork();

	// Deallocate all element storage assigned to the ship itself
	if (m_elements) ComplexShipElement::DeallocateElementStorage(&m_elements, m_elementsize);
}




