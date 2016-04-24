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
#include "EnvironmentTree.h"
#include "CSLifeSupportTile.h"

#include "iSpaceObjectEnvironment.h"


// Initialise static working vector for environment object search; holds nodes being considered in the search
std::vector<EnvironmentTree*> iSpaceObjectEnvironment::m_search_nodes;

// Default constructor
iSpaceObjectEnvironment::iSpaceObjectEnvironment(void)
{
	// Set the flag that indicates this object is itself an environment that contains objects
	m_isenvironment = true;

	// This class does implement a post-simulation update method
	m_canperformpostsimulationupdate = true;

	// Initialise fields to defaults
	m_elements = NULL;
	SetElementSize(NULL_INTVECTOR3);
	m_updatesuspended = false;
	m_containssimulationhubs = false;
	m_navnetwork = NULL;
	SpatialPartitioningTree = NULL;
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
	Ship::InitialiseCopiedObject(source);
	iContainsComplexShipTiles::InitialiseCopiedObject(source);

	/* Now perform all iSpaceObjectEnvironment-related initialisation here */

	// Initialise the environment element space with new data, since currently the pointer is copied from the source ship
	// Important: Set to NULL first, otherwise deallocation method will deallocate the original element space before replacing it
	SetElements(NULL);
	InitialiseElements(m_elementsize, source->GetElements(), source->GetElementSize());

	// Remove the nav network pointer in this environment, since we want to generate a new one for the environment when first required
	this->RemoveNavNetworkLink();

	// Perform an initial derivation of the world/zero point matrices, as a starting point
	RefreshPositionImmediate();
}


// Standard object simulation method, used to simulate the contents of this object environment
void iSpaceObjectEnvironment::SimulateObject(void)
{
	// Simulate all ship tiles within the environment that require simulation
	ComplexShipTile *tile;
	iContainsComplexShipTiles::ComplexShipTileCollection::iterator t_it_end = m_tiles[0].end();
	for (iContainsComplexShipTiles::ComplexShipTileCollection::iterator t_it = m_tiles[0].begin(); t_it != t_it_end; ++t_it)
	{
		// Test whether the tile actually requires simulation (e.g. basic tile types do not have any simulation logic, and 
		// most tiles are simulated at intervals so will not require any simulation time on most cycles)
		tile = (*t_it).value;
		if (tile && tile->RequiresSimulation())
		{
			// Call the tile simulation method
			tile->SimulateTile();
		}
	}

	// Update properties of the environment if required
	if (m_gravityupdaterequired)		PerformGravityUpdate();
	if (m_oxygenupdaterequired)			PerformOxygenUpdate();
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

// Set the simulation state of all environment contents
void iSpaceObjectEnvironment::SetSimulationStateOfEnvironmentContents(iObject::ObjectSimulationState state)
{
	// Iterate through all objects in the environment and change their state
	std::vector<ObjectReference<iEnvironmentObject>>::iterator it_end = Objects.end();
	for (std::vector<ObjectReference<iEnvironmentObject>>::iterator it = Objects.begin(); it != it_end; ++it)
	{
		// Set the simulation state of this object
		if ((*it)()) (*it)()->SetSimulationState(state);
	}

	// Also set the simulation state of all tiles in the environment
	iContainsComplexShipTiles::ComplexShipTileCollection::iterator it2_end = m_tiles[0].end();
	for (iContainsComplexShipTiles::ComplexShipTileCollection::iterator it2 = m_tiles[0].begin(); it2 != it2_end; ++it2)
	{
		// Set the simulation state of this tile
		if ((*it2).value) (*it2).value->SetSimulationState(state);
	}
}

// Performs an update of environment gravity levels, based on each life support system in the ship
void iSpaceObjectEnvironment::PerformGravityUpdate(void)
{
	CSLifeSupportTile *tile;
	INTVECTOR3 elmin, elmax;
	int maxrange;
	float gravity;

	// Reset the update flag now we are performing an update
	m_gravityupdaterequired = false;

	// First, reset the gravity strength at every element to zero
	ComplexShipElement *el = m_elements;
	for (int i = 0; i < m_elementcount; ++i, ++el)
	{
		el->ChangeGravityStrength(0.0f);
	}

	// Now process each life support system in turn
	std::vector<AComplexShipTile_P>::iterator it_end = GetTilesOfType(D::TileClass::LifeSupport).end();
	for (std::vector<AComplexShipTile_P>::iterator it = GetTilesOfType(D::TileClass::LifeSupport).begin(); it != it_end; ++it)
	{
		// Make sure this is a valid life support system
		tile = (CSLifeSupportTile*)(*it).value;
		if (!tile) continue;

		// Determine the maximum effective tile range of this system.  E.g. Range=3 --> MaxRange=ceil(3*1.41...) = ceil(4.24) = 5
		maxrange = (int)ceilf(tile->GetGravityRange() * ROOT2);

		// Now determine the range of elements that need to be considered for the system
		elmin = INTVECTOR3(max(0, tile->GetElementLocationX() - maxrange), max(0, tile->GetElementLocationY() - maxrange),
			max(0, tile->GetElementLocationZ() - maxrange));
		elmax = INTVECTOR3(min(m_elementsize.x, tile->GetElementLocationX() + maxrange),
			min(m_elementsize.y, tile->GetElementLocationY() + maxrange),
			min(m_elementsize.z, tile->GetElementLocationZ() + maxrange));

		// Consider each relevant element in turn
		for (int x = elmin.x; x < elmax.x; ++x)
		{
			for (int y = elmin.y; y < elmax.y; ++y)
			{
				for (int z = elmin.z; z < elmax.z; ++z)
				{
					// Get the effective gravity strength at this location
					gravity = tile->GetGravityStrength(x, y, z);

					// Apply this to the target element, if it is higher than the current gravity value
					el = GetElement(x, y, z); if (!el) continue;
					if (gravity > el->GetGravityStrength())
					{
						el->ChangeGravityStrength(gravity);
					}
				}
			}
		}	// x/y/z
	}		// For each life support system
}

// Performs an update of environment oxygen levels, based on each life support system in the ship
void iSpaceObjectEnvironment::PerformOxygenUpdate(void)
{
	// Reset the update flag now we are performing an update
	m_oxygenupdaterequired = false;

	// TODO: Do this
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
		// Add to the objects collection
		Objects.push_back(ObjectReference<iEnvironmentObject>(obj));

		// Add to the spatial partitioning tree
		if (SpatialPartitioningTree) SpatialPartitioningTree->AddItem<iEnvironmentObject*>(obj);
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

		// Remove from the spatial parititiong tree
		if (SpatialPartitioningTree) SpatialPartitioningTree->RemoveItem<iEnvironmentObject*>((*it)());

		// Remove from the collection
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

	// Also add a reference in the spatial partitioning tree
	if (SpatialPartitioningTree) SpatialPartitioningTree->AddItem<StaticTerrain*>(obj);
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

	// Remove from the spatial partitioning tree
	if (SpatialPartitioningTree) SpatialPartitioningTree->RemoveItem<StaticTerrain*>(obj);

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

	// Perform a full rebuild of the spatial partitioning tree; likely more efficient than removing one-by-one
	BuildSpatialPartitioningTree();
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

// Copies all tiles from another object and adds the copies to this object
Result iSpaceObjectEnvironment::CopyTileDataFromObject(iContainsComplexShipTiles *src)
{
	// Parameter check
	if (!src) return ErrorCodes::CannotCopyTileDataFromNullSource;

	// Suspend updates while making these bulk changes, and we will then resume updates at the end
	SuspendEnvironmentUpdates();

	// Remove any tile data that currently exists for the environment; do not use the RemoveTile() methods since the
	// existing tiles belong to the source environment, which we just copied from
	RemoveAllShipTiles();

	// Remove all terrain objects which were introduced as part of a tile; create a new vector, populate with any 
	// terrain objects that are still valid, and then swap the vectors at the end.  More efficient than removing
	// items from the main vector since the majority will likely be removed here.
	TerrainCollection terrain;
	TerrainCollection::size_type n = TerrainObjects.size();
	for (TerrainCollection::size_type i = 0; i < n; ++i)
	{
		if (TerrainObjects[i]->GetParentTileID() == 0) terrain.push_back(TerrainObjects[i]);
	}
	TerrainObjects = terrain;

	//  Iterate through each tile in the source in turn
	iContainsComplexShipTiles::ConstTileIterator it_end = src->GetTiles().end();
	for (iContainsComplexShipTiles::ConstTileIterator it = src->GetTiles().begin(); it != it_end; ++it)
	{
		// Get a reference to this tile and make a copy via virtual subclass copy method
		ComplexShipTile *tile = (*it).value->Copy();

		// Add this cloned tile to the new ship
		if (tile) AddTile(tile);
	}

	// Rebuild the spatial partitioning tree to account for all the changes above
	BuildSpatialPartitioningTree();

	// Resume updates, which will also trigger an update of the whole environment
	ResumeEnvironmentUpdates();

	// Return success
	return ErrorCodes::NoError;
}

// Add a tile to the environment
void iSpaceObjectEnvironment::AddTile(ComplexShipTile *tile)
{
	// Parameter check
	if (!tile) return;

	// Raise the pre-addition event
	BeforeTileAdded(tile);

	// Add to the tile collection
	AddShipTile(tile);
	
	// Add any terrain objects that come with this tile
	AddTerrainObjectsFromTile(tile);

	// Update the environment
	UpdateEnvironment();

	// Raise the post-addition event
	TileAdded(tile);
}

// Remove a tile from the environment
void iSpaceObjectEnvironment::RemoveTile(ComplexShipTile *tile)
{
	// Parameter check
	if (!tile) return;

	// Raise the pre-removal event
	BeforeTileRemoved(tile);

	// Add to the tile collection
	RemoveShipTile(tile);

	// Add any terrain objects that come with this tile
	RemoveTerrainObjectsFromTile(tile);

	// Update the environment
	UpdateEnvironment();

	// Raise the post-addition event
	TileRemoved(tile);
}


// Adds all terrain objects associated with a tile to the environment
void iSpaceObjectEnvironment::AddTerrainObjectsFromTile(ComplexShipTile *tile)
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

// Adds all terrain objects associated with a tile to the environment
void iSpaceObjectEnvironment::RemoveTerrainObjectsFromTile(ComplexShipTile *tile)
{
	// Parameter check
	if (!tile) return;

	// Remove any terrain objects in the environment that were owned by this tile
	Game::ID_TYPE id = tile->GetID();
	TerrainCollection::iterator it = std::partition(TerrainObjects.begin(), TerrainObjects.end(),
		[&id](const StaticTerrain *element) { return (element->GetParentTileID() != id); });
	delete_erase<TerrainCollection, StaticTerrain*>(TerrainObjects, it, TerrainObjects.end());

	// Rebuild the spatial partitioning tree following this bulk change
	BuildSpatialPartitioningTree();
}


// Event triggered after addition of a tile to this environment.  Virtual, can be inherited by subclasses
void iSpaceObjectEnvironment::BeforeTileAdded(ComplexShipTile *tile)
{
	// Raise the pre-addition event in the tile object
	if (tile) tile->BeforeAddedToEnvironment(this);
}
// Event triggered after addition of a tile to this environment.  Virtual, can be inherited by subclasses
void iSpaceObjectEnvironment::TileAdded(ComplexShipTile *tile)
{
	// Raise the post-addition event in the tile object
	if (tile) tile->AfterAddedToEnvironment(this);
}

// Event triggered after removal of a tile from this environment.  Virtual, can be inherited by subclasses
void iSpaceObjectEnvironment::BeforeTileRemoved(ComplexShipTile *tile)
{
	// Raise the pre-removal event in the tile object
	if (tile) tile->BeforeRemovedToEnvironment(this);
}
// Event triggered after removal of a tile from this environment.  Virtual, can be inherited by subclasses
void iSpaceObjectEnvironment::TileRemoved(ComplexShipTile *tile)
{
	// Raise the post-removal event in the tile object
	if (tile) tile->AfterRemovedFromEnvironment(this);
}

// Updates the environment following a change to its structure, for example when adding/removing a tile
void iSpaceObjectEnvironment::UpdateEnvironment(void)
{
	// Only perform the update if updates are not suspended
	if (m_updatesuspended) return;

	// Reset all element properties that will be set during the update steps below
	for (int i = 0; i < m_elementcount; ++i)
	{
		m_elements[i].SetTile(NULL);
	}

	// Process each tile in turn
	iContainsComplexShipTiles::ComplexShipTileCollection::iterator it_end = m_tiles[0].end();
	for (iContainsComplexShipTiles::ComplexShipTileCollection::iterator it = m_tiles[0].begin(); it != it_end; ++it)
	{
		// Apply the effects of the tile to its underlying elements
		(*it).value->ApplyTile();
	}

	// Update the environment navigation network given that connectivity may have changed
	UpdateNavigationNetwork();
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
	return InitialiseElements(extent, true);
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
	ShutdownAllTileData(true);

	// Detach and deallocate the navigation network assigned to this ship
	ShutdownNavNetwork();

	// Deallocate all element storage assigned to the ship itself
	DeallocateElementSpace();
}


// Allocates a new element space of the specified size.  Contents are copied from the existing 
// space (as far as possible, and if relevant).  If any element space does already exist 
// it will be deallocated first
Result iSpaceObjectEnvironment::InitialiseElements(INTVECTOR3 size)
{
	// Pass to the overloaded method, with preserve_contents = true as the default behaviour
	return InitialiseElements(size, true);
}

// Allocates a new element space of the specified size.  Contents are initialised to default starting values or
// copied from the existing space (as far as possible, and if relevant).  If any element space does already exist 
// it will be deallocated first
Result iSpaceObjectEnvironment::InitialiseElements(INTVECTOR3 size, bool preserve_contents)
{
	// If we want to preserve the current contents, and if we do currently have an element
	// space, pass a pointer to it as the copy source when allocating the new space
	if (preserve_contents && !m_elementsize.IsZeroVector() && m_elements)
	{
		return InitialiseElements(size, m_elements, m_elementsize);
	}
	else
	{
		return InitialiseElements(size, NULL, NULL_INTVECTOR3);
	}
}

// Allocates a new element space of the specified size, copying data from the existing element space as 
// far as possible based on dimensions
Result iSpaceObjectEnvironment::InitialiseElements(INTVECTOR3 size, const ComplexShipElement * const source, INTVECTOR3 source_size)
{
	// We only need to allocate new space if the desired size is positive & non-zero in all dimension
	ComplexShipElement *el = NULL;
	int ecount = (size.x * size.y * size.z);
	bool allocate_new = (size.x >= 0 && size.y >= 0 && size.z >= 0 && ecount <= Game::C_CS_ELEMENT_SIZE_LIMIT);
	if (allocate_new)
	{
		// Allocate a new element space of the specified size
		el = new (nothrow)ComplexShipElement[ecount];
		if (!el)
		{
			DeallocateElementSpace();
			return ErrorCodes::CouldNotAllocateEnvironmentElementSpace;
		}

		// Test whether we want to copy data from another source
		if (source)
		{
			// Values precalculated for use during the copy operation
			int scount = (source_size.x * source_size.y * source_size.z);
			INTVECTOR3 ubound = (size - INTVECTOR3(1, 1, 1));
			int size_xy = (size.x * size.y);

			// Loop through all elements in the source collection
			for (int i = 0; i < scount; ++i)
			{
				// See whether this element will exist in our new element space
				const ComplexShipElement & src = source[i];
				const INTVECTOR3 & sloc = src.GetLocation();
				if (IntVector3Between(sloc, NULL_INTVECTOR3, ubound))
				{
					// Copy the source element into the new space
					el[ELEMENT_INDEX_EX(sloc.x, sloc.y, sloc.z, size, size_xy)] = src;
				}
			}
		}
	}

	// Deallocate any element space that already exists before assigning the new one
	DeallocateElementSpace();

	// Now assign the new element space, if we allocated one
	if (allocate_new)
	{
		// Assign the new element space
		m_elements = el;
		SetElementSize(size);

		// Update the internal links and pointers between adjacent elements
		UpdateElementSpaceStructure();

		// Rebuild the spatial partitioning tree based on this new element structure
		BuildSpatialPartitioningTree();
	}

	// Return success
	return ErrorCodes::NoError;
}


// Sets the size of the element space within this environment.  Protected.  Only called by
// object methods which are handling the effects of the element space change
void iSpaceObjectEnvironment::SetElementSize(const INTVECTOR3 & size)
{
	// Store the new element size
	m_elementsize = size;

	// Cache intermediate values based on this element size for efficiency at runtime
	m_elementcount = (size.x * size.y * size.z);
	m_xy_size = (m_elementsize.x * m_elementsize.y);
	m_yz_size = (m_elementsize.y * m_elementsize.z);
}


// Rebuilds the spatial partitioning tree, populating with all existing objects if relevant
void iSpaceObjectEnvironment::BuildSpatialPartitioningTree(void)
{
	// If we already have a spatial partitioning tree, deallocate it before rebuilding
	if (SpatialPartitioningTree)
	{
		SpatialPartitioningTree->Shutdown();
		SpatialPartitioningTree = NULL;
	}

	// Create a new root node
	SpatialPartitioningTree = new EnvironmentTree(this);

	// Add all terrain objects to the tree
	std::vector<StaticTerrain*>::iterator it_end = TerrainObjects.end();
	for (std::vector<StaticTerrain*>::iterator it = TerrainObjects.begin(); it != it_end; ++it)
	{
		SpatialPartitioningTree->AddItem<StaticTerrain*>((*it));
	}

	// Also add all objects to the tree
	std::vector<ObjectReference<iEnvironmentObject>>::iterator it2_end = Objects.end();
	for (std::vector<ObjectReference<iEnvironmentObject>>::iterator it2 = Objects.begin(); it2 != it2_end; ++it2)
	{
		SpatialPartitioningTree->AddItem<iEnvironmentObject*>((*it2)());
	}
}

// Converts an element index into its x/y/z location
INTVECTOR3 iSpaceObjectEnvironment::ElementIndexToLocation(int index) const
{
	/* z = Math.round(i / (WIDTH * HEIGHT));
	   y = Math.round((i - z * WIDTH * HEIGHT) / WIDTH);
	   x = i - WIDTH * (y + HEIGHT * z); */
	INTVECTOR3 v;
	v.z = (index / m_xy_size);
	v.y = ((index - (v.z * m_xy_size)) / m_elementsize.x);
	v.x = (index - (m_elementsize.x * (v.y + (m_elementsize.y * v.z))));
	return v;
}

// Converts an element index into its x/y/z location
void iSpaceObjectEnvironment::ElementIndexToLocation(int index, INTVECTOR3 & outVector) const
{
	/* z = Math.round(i / (WIDTH * HEIGHT));
	   y = Math.round((i - z * WIDTH * HEIGHT) / WIDTH);
	   x = i - WIDTH * (y + HEIGHT * z); */
	outVector.z = (index / m_xy_size);
	outVector.y = ((index - (outVector.z * m_xy_size)) / m_elementsize.x);
	outVector.x = (index - (m_elementsize.x * (outVector.y + (m_elementsize.y * outVector.z))));
}



// Updates the IDs, locations, and internal links between adjacent elements
void iSpaceObjectEnvironment::UpdateElementSpaceStructure(void)
{
	INTVECTOR3 loc, adj;

	// Process each element in turn
	for (int i = 0; i < m_elementcount; ++i)
	{
		// Set the element ID and location
		loc = ElementIndexToLocation(i);
		ComplexShipElement & el = m_elements[i];
		el.SetID(i);
		el.SetLocation(loc);

		// TODO: Temporary: Backward-validate to make sure the generated location is correct
#		if _DEBUG
		if (ELEMENT_INDEX(loc.x, loc.y, loc.z) != i)
		{
			throw "ERROR: Element index method generated incorrect result";
		}
#		endif

		// Set links to adjacent elements
		for (int d = 0; d < (int)Direction::_Count; ++d)
		{
			adj = (loc + DirectionUnitOffset((Direction)d));
			if (GetElement(adj) != NULL)
			{
				el.LinkNeighbour((Direction)d, ELEMENT_INDEX(adj.x, adj.y, adj.z));
			}
			else
			{
				el.LinkNeighbour((Direction)d, ComplexShipElement::NO_ELEMENT);
			}
		}
	}
}

// Deallocates the object element space
void iSpaceObjectEnvironment::DeallocateElementSpace(void)
{
	// Deallocate the element spce if it exists
	if (m_elements) SafeDeleteArray(m_elements);

	// Reset the current element size to show there are no elements present
	SetElementSize(NULL_INTVECTOR3);
}


// Rotates the element space by the specified angle
Result iSpaceObjectEnvironment::RotateElementSpace(Rotation90Degree rotation)
{
	// Parameter checks
	if (!m_elements || m_elementsize.IsZeroVector()) return ErrorCodes::InvalidParametersToRotateElementSpace;
	if (rotation == Rotation90Degree::Rotate0) return ErrorCodes::NoError;

	// Allocate a new element space (of the same size)
	ComplexShipElement *elements = new (nothrow)ComplexShipElement[m_elementcount];
	if (!elements) return ErrorCodes::CannotAllocateElementSpaceForRotation;

	// Determine new element space dimensions
	INTVECTOR3 newsize = ((rotation == Rotation90Degree::Rotate90 || rotation == Rotation90Degree::Rotate270) ?
		INTVECTOR3(m_elementsize.y, m_elementsize.x, m_elementsize.z) : m_elementsize);
	int newsize_xy = (newsize.x * newsize.y);

	// Reassign each element in turn
	int index;
	for (int i = 0; i < m_elementcount; ++i)
	{
		ComplexShipElement & el = m_elements[i];
		const INTVECTOR3 & loc = el.GetLocation();

		// Determine the target element that will receive data from 'el'
		index = -1;
		switch (rotation)
		{
		case Rotation90Degree::Rotate90:
			index = ELEMENT_INDEX_EX(loc.y, (m_elementsize.x - 1) - loc.x, loc.z, newsize, newsize_xy);							break;
		case Rotation90Degree::Rotate180:
			index = ELEMENT_INDEX_EX((m_elementsize.x - 1) - loc.x, (m_elementsize.y - 1) - loc.y, loc.z, newsize, newsize_xy);	break;
		case Rotation90Degree::Rotate270:
			index = ELEMENT_INDEX_EX((m_elementsize.y - 1) - loc.y, loc.x, loc.z, newsize, newsize_xy);							break;
		}

		// Copy the element contents 
		if (index != -1)
		{
			elements[index] = el;
			elements[index].RotateElement(rotation);
		}
	}

	// Update the element size to reflect the change in dimensions, if required
	SetElementSize(newsize);

	// Update links between elements in the space
	UpdateElementSpaceStructure();

	// Return success
	return ErrorCodes::NoError;
}

// Internal method; get all objects within a given distance of the specified position, within the 
// specified EnvironmentTree node
void iSpaceObjectEnvironment::_GetAllObjectsWithinDistance(EnvironmentTree *tree_node, const FXMVECTOR position, float distance,
	std::vector<iEnvironmentObject*> *outObjects, std::vector<StaticTerrain*> *outTerrain)
{
	// Parameter check
	if (!tree_node) return;

	// Determine the range that needs to be covered
	XMVECTOR vdist = XMVectorReplicate(distance + Game::C_ENVIRONMENT_OBJECT_SEARCH_DISTANCE_MARGIN);		// Add threshold 
	INTVECTOR3 emin = GetClampedElementContainingPosition(XMVectorSubtract(position, vdist));
	INTVECTOR3 emax = GetClampedElementContainingPosition(XMVectorAdd(position, vdist));
	XMVECTOR distsq = XMVectorReplicate(distance * distance);
	
	// Test whether the current node contains the desired search range; while it does not, keep
	// backing up towards the root
	EnvironmentTree *node = tree_node;
	while (!(node->ContainsElement(emin) && node->ContainsElement(emax)) && node->GetParent())
	{
		node = node->GetParent();
	}

	// The current node should now contain the full search area (or it is the root, if the search 
	// area extends beyond the bounds of the tree itself).  We therefore want to process all nodes 
	// and record any objects that meet the distance criteria

	// Use a non-recursive vector search for efficiency
	m_search_nodes.clear();
	m_search_nodes.push_back(node);

	// Continue until all nodes have been processed
	EnvironmentTree *child;
	while (!m_search_nodes.empty())
	{
		// Retrieve the next node and pop it from the search vector
		node = m_search_nodes.back();
		m_search_nodes.pop_back();
		if (!node) continue;

		// If the node is a branch, add any of its children to the search vector that are
		// within the search area
		if (node->IsBranchNode())
		{
			int n = node->GetChildCount();
			for (int i = 0; i < n; ++i)
			{
				// Only add the node if it contains part of our search area
				child = node->GetActiveChildNode(i);
				if (child->CoversElementRange(emin, emax)) m_search_nodes.push_back(child);
			}
		}
		else
		{
			// This is a leaf node; process all items and return any that are within the search distance
			if (outObjects)
			{
				iEnvironmentObject *obj;
				std::vector<iEnvironmentObject*>::const_iterator it_end = node->GetNodeObjects().end();
				for (std::vector<iEnvironmentObject*>::const_iterator it = node->GetNodeObjects().begin(); it != it_end; ++it)
				{
					obj = (*it);
					if (obj && XMVector2LessOrEqual(XMVector3LengthSq(XMVectorSubtract(obj->GetEnvironmentPosition(), position)), distsq))
						outObjects->push_back(obj);
				}
			}

			// Also process any terrain objects
			if (outTerrain)
			{
				StaticTerrain *obj;
				std::vector<StaticTerrain*>::const_iterator it_end = node->GetNodeTerrain().end();
				for (std::vector<StaticTerrain*>::const_iterator it = node->GetNodeTerrain().begin(); it != it_end; ++it)
				{
					obj = (*it);
					if (obj && XMVector2LessOrEqual(XMVector3LengthSq(XMVectorSubtract(obj->GetEnvironmentPosition(), position)), distsq))
						outTerrain->push_back(obj);
				}
			}
		}
	}
}


// Find all objects within a given distance of the specified object.  Object & Terrain output
// vectors will be populated if valid pointers are supplied
void iSpaceObjectEnvironment::GetAllObjectsWithinDistance(iEnvironmentObject *focal_object, float distance,
	std::vector<iEnvironmentObject*> *outObjects, std::vector<StaticTerrain*> *outTerrain)
{
	if (focal_object) _GetAllObjectsWithinDistance(focal_object->GetEnvironmentTreeNode(),
		focal_object->GetEnvironmentPosition(), distance, outObjects, outTerrain);
}

// Find all objects within a given distance of the specified object.  Object & Terrain output
// vectors will be populated if valid pointers are supplied
void iSpaceObjectEnvironment::GetAllObjectsWithinDistance(StaticTerrain *focal_object, float distance,
	std::vector<iEnvironmentObject*> *outObjects, std::vector<StaticTerrain*> *outTerrain)
{
	if (focal_object) _GetAllObjectsWithinDistance(focal_object->GetEnvironmentTreeNode(),
		focal_object->GetEnvironmentPosition(), distance, outObjects, outTerrain);
}

// Find all objects within a given distance of the specified location.  Object & Terrain output
// vectors will be populated if valid pointers are supplied.  Less efficient than the method
// which supplies a focal object, since the relevant node has to be determined based on the position
void iSpaceObjectEnvironment::GetAllObjectsWithinDistance(EnvironmentTree *spatial_tree, const FXMVECTOR position, float distance,
	std::vector<iEnvironmentObject*> *outObjects, std::vector<StaticTerrain*> *outTerrain)
{
	if (spatial_tree)
	{
		// Determine the relevant tree node and pass to the primary search function
		_GetAllObjectsWithinDistance(spatial_tree->GetNodeContainingPoint(position), position, distance, outObjects, outTerrain);
	}
}

// Default destructor
iSpaceObjectEnvironment::~iSpaceObjectEnvironment(void)
{

}
