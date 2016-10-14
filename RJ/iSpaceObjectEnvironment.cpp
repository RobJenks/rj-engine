#include "ErrorCodes.h"
#include "Utility.h"
#include "FastMath.h"
#include "GameVarsExtern.h"
#include "ComplexShipElement.h"
#include "iEnvironmentObject.h"
#include "ComplexShipTileDefinition.h"
#include "ComplexShipTile.h"
#include "DynamicTileSet.h"
#include "StaticTerrain.h"
#include "Ship.h"
#include "iContainsComplexShipTiles.h"
#include "NavNetwork.h"
#include "EnvironmentTree.h"
#include "CSLifeSupportTile.h"
#include "Ray.h"
#include "AABB.h"
#include "OrientedBoundingBox.h"
#include "ElementIntersection.h"
#include "EnvironmentCollision.h"
#include"SimulatedEnvironmentCollision.h"

#include "iSpaceObjectEnvironment.h"


// Initialise static working vector for environment object search; holds nodes being considered in the search
std::vector<EnvironmentTree*> iSpaceObjectEnvironment::m_search_nodes;
SimulatedEnvironmentCollision iSpaceObjectEnvironment::EnvironmentCollisionSimulationResults;;

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
	m_deckcount = 0;
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

	// Process any active collision events
	if (HaveActiveEnvironmentCollisionEvents())
	{
		ProcessAllEnvironmentCollisions();
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
		// Determine the transformation matrix for this rotation
		XMMATRIX transform = GetRotationMatrix(sourcetile->GetRotation());

		// Transform the tile-local object position by this matrix
		XMVECTOR localpos = XMVector3TransformCoord(obj->GetPosition(), transform);

		// Now adjust the terrain orientation and position, transforming it into environment space
		obj->SetOrientation(XMQuaternionMultiply(obj->GetOrientation(), GetRotationQuaternion(sourcetile->GetRotation())));
		obj->SetPosition(XMVectorAdd(localpos, sourcetile->GetRelativePosition()));
	}
	else
	{
		// The tile is not rotated at all, so we just need to determine the correct terrain position
		obj->SetPosition(XMVectorAdd(obj->GetPosition(), sourcetile->GetRelativePosition()));
	}

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
		if (tile) AddTile(&tile);
	}

	// Rebuild the spatial partitioning tree to account for all the changes above
	BuildSpatialPartitioningTree();

	// Resume updates, which will also trigger an update of the whole environment
	ResumeEnvironmentUpdates();

	// Return success
	return ErrorCodes::NoError;
}

// Add a tile to the environment
void iSpaceObjectEnvironment::AddTile(ComplexShipTile **ppTile)
{
	// Parameter check
	if (!ppTile || !(*ppTile)) return;
	ComplexShipTile *tile_obj = (*ppTile);
	if (!tile_obj) return;

	// Raise the pre-addition event
	BeforeTileAdded(tile_obj);

	// Add to the tile collection
	AddShipTile(tile_obj);
	
	// Add any terrain objects that come with this tile
	AddTerrainObjectsFromTile(tile_obj);

	// Raise the post-addition event
	TileAdded(tile_obj);

	// Set the tile simulation state to match that of the parent ship
	tile_obj->SetSimulationState(m_simulationstate);

	// Test whether this tile, or its neighbours, need to adapt to adjust to 
	// their surrounding neighbourhood of tiles
	UpdateTileConnectionState(ppTile);

	// Update the environment
	UpdateEnvironment();
}

// Remove a tile from the environment
void iSpaceObjectEnvironment::RemoveTile(ComplexShipTile *tile)
{
	// Parameter check
	if (!tile) return;

	// Raise the pre-removal event
	BeforeTileRemoved(tile);

	// Remove from the tile collection
	RemoveShipTile(tile);

	// Remove any terrain objects that came with this tile
	RemoveTerrainObjectsFromTile(tile);

	// Raise the post-removal event
	TileRemoved(tile);

	// Update the environment
	UpdateEnvironment();
}


// Adds all terrain objects associated with a tile to the environment
void iSpaceObjectEnvironment::AddTerrainObjectsFromTile(ComplexShipTile *tile)
{
	// Parameter check
	if (!tile) return;
	const ComplexShipTileDefinition *def = tile->GetTileDefinition();

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
	
	// Keep a temporary record of all decks that contain tiles
	unsigned int nZ = max((unsigned int)m_elementsize.z, 1);
	std::vector<bool> decks(nZ, false);

	// Process each tile in turn
	ComplexShipTile *tile;
	iContainsComplexShipTiles::ComplexShipTileCollection::iterator it_end = m_tiles[0].end();
	for (iContainsComplexShipTiles::ComplexShipTileCollection::iterator it = m_tiles[0].begin(); it != it_end; ++it)
	{
		// Apply the effects of the tile to its underlying elements
		tile = (*it).value;
		tile->ApplyTile();

		// Mark all decks covered by this tile as active
		int min_deck = tile->GetElementLocationZ();
		int max_deck = min_deck + tile->GetElementSizeZ() - 1;
		for (int i = min_deck; i <= max_deck; ++i) decks[i] = true;
	}

	// Store the number of decks and indices into the relevant z-values
	m_deckcount = 0; m_deck_indices.clear();
	for (int i = 0; i < (int)nZ; ++i)
	{
		if (decks[i])
		{
			++m_deckcount;
			m_deck_indices.push_back(i);
		}
	}
	assert(m_deckcount == (int)m_deck_indices.size());

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

// Updates a tile following a change to its connection state, i.e. where it now connects to new or fewer
// neighbouring tiles.  Accepts the address of a tile pointer and will adjust that tile pointer if
// the tile is updated as part of the analysis.  TODO: May wish to replace with more general methods in future
Result iSpaceObjectEnvironment::UpdateTileBasedOnConnectionData(ComplexShipTile **ppOutTile)
{
	Result result;

	// Parameter check
	if (!ppOutTile || !(*ppOutTile)) return ErrorCodes::NoError;
	
	// Test whether this tile is part of a dynamic tile set
	const ComplexShipTileDefinition *definition = (*ppOutTile)->GetTileDefinition();
	if (definition && definition->BelongsToDynamicTileSet())
	{
		// Attempt to get a reference to this dynamic tileset
		DynamicTileSet *tileset = D::DynamicTileSets.Get(definition->GetDynamicTileSet());
		if (tileset)
		{
			// Store for later (since "tile" may not exist later if it is replaced)
			Rotation90Degree rot = (*ppOutTile)->GetRotation();

			// Verify whether the current definition is still applicable
			DynamicTileSet::DynamicTileSetResult new_def = tileset->GetMostAppropriateTileDefinition(*ppOutTile);
			if (new_def.TileDefinition != definition)
			{
				// The current tile definition is no longer valid.  We therefore want to generate a 
				// replacement tile using this new definition and copy the relevant data across to it
				ComplexShipTile *new_tile = new_def.TileDefinition->CreateTile();
				if (new_tile)
				{
					ComplexShipTile::CopyBaseClassData((*ppOutTile), new_tile);
					result = new_tile->CompileAndValidateTile();
					if (result != ErrorCodes::NoError)
					{
						// Discard the new tile since it could not be validated
						SafeDelete(new_tile);
					}
					else
					{
						// We are good to make the change; first, replace the tile with the new one
						ReplaceTile((*ppOutTile), new_tile);

						// Now deallocate the old tile
						SafeDelete(*ppOutTile);

						// Finally, adjust the pointer in this method to reference the new tile
						(*ppOutTile) = new_tile;
					}
				}
			}

			// We also need to make sure the tile is oriented as per the DTS entry
			if (rot != new_def.Rotation)
			{
				(*ppOutTile)->SetRotation(new_def.Rotation);
			}
		}
	}

	// We now know that this tile has the correct tile definition associated to it, so can
	// proceed with adjusting any geometry etc. based on that definition

	// Regenerate the tile geometry if required based on the new connections etc. into this tile
	result = (*ppOutTile)->GenerateGeometry();
	if (result != ErrorCodes::NoError) return result;

	// Return success; the contents of ppOutTile will be the new or existing tile, depending on whether a change was made
	return ErrorCodes::NoError;
}

// Replaces one tile in the environment with another.  The old tile is not 
// shut down or deallocated by this operation
void iSpaceObjectEnvironment::ReplaceTile(ComplexShipTile *old_tile, ComplexShipTile *new_tile)
{
	// Parameter check; old tile cannot be null
	if (!old_tile) return;

	// Suspend updates while we make changes
	SuspendTileRecalculation();

	// Remove the old tile from the environment
	RemoveTile(old_tile);

	// We now want to add the new tile, assuming it exists
	if (new_tile)
	{
		ComplexShipTile **ppNewTile = &new_tile;		// To prevent the contents of the new_tile pointer being modified
		AddTile(ppNewTile);
	}

	// Resume updates, which will recalculate all tile-dependent data
	ReactivateTileRecalculation();
}

// Returns the element index corresponding to the supplied deck.  Default 0 if invalid parameter
int iSpaceObjectEnvironment::GetDeckIndex(int deck) const
{
	if (deck < 0 || deck >= m_deckcount)		return 0;
	else										return m_deck_indices[deck];
}

// Determines the element intersected by a world-space ray.  Returns a flag indicating whether any
// intersection does take place
bool iSpaceObjectEnvironment::DetermineElementIntersectedByRay(const Ray & ray, INTVECTOR3 & outElement)
{
	// Test whether we actually have an intersection
	if (Game::PhysicsEngine.DetermineRayVsOBBIntersection(ray, CollisionOBB.Data()))
	{
		// We do have an intersection.  The intersection time has been calculated by the physics engine
		// so simply pass it to the oveloaded function
		return DetermineElementIntersectedByRayAtTime(ray, Game::PhysicsEngine.RayIntersectionResult.tmin, outElement);
	}
	else
	{
		// There is no intersection at all, so return false immediately
		return false;
	}
}

// Determines the element intersected by a world-space ray at the specified time t along the ray.  
// Returns a flag indicating whether any intersection does take place
bool iSpaceObjectEnvironment::DetermineElementIntersectedByRayAtTime(const Ray & ray, float t, INTVECTOR3 & outElement)
{
	// Determine the world-space position corresponding to this time t along the ray (at Pos = Origin + (t * Direction))
	XMVECTOR pos = XMVectorMultiplyAdd(XMVectorReplicate(t), XMVector3NormalizeEst(ray.Direction), ray.Origin);

	// Transform this position into local ship space, and make sure it is within the ship bounds
	// We artificially extend the ship bounds slightly (by [1,1,1], and here only) to allow for imprecision in the intersection tests
	XMVECTOR localpos = XMVector3TransformCoord(pos, m_inversezeropointworldmatrix);

	// TODO: this check removed for now, since imprecision in the ray/OBB test was frequently leading to an actual collision point 
	// outside of the expected element bounds.  We now clamp to the element bounds in the final statement below
	/*if ((XMVector3GreaterOrEqual(localpos, ONE_VECTOR_N) && XMVector3LessOrEqual(localpos, XMVectorAdd(m_size, ONE_VECTOR_P))) == false)
		return false;*/

	// We know this is a valid element within the ship, so determine and return the element location
	// Clamp the position value within bounds that will translate directly to an element index ([0,0,0] to (size-[eps,eps,eps]))
	// since we already know the collision is approximately within the ship bounds
	static const AXMVECTOR clamp_neg = XMVectorSetW(NULL_VECTOR, FLT_MAX_NEG);			// Since XMVectorClamp() will assert Min<=Max, and the W component may not be
	outElement = Game::PhysicalPositionToElementLocation(XMVectorClamp(localpos, clamp_neg, XMVectorAdd(m_size, Game::C_EPSILON_NEG_V)));
	return true;
}

// Determines the element intersected by a world-space ray on the specified level.  Returns a flag indicating whether any
// intersection does take place
bool iSpaceObjectEnvironment::DetermineElementIntersectedByRay(const Ray & ray, int level, INTVECTOR3 & outElement)
{
	// Generate an AABB that represents one level of the ship
	static const float level_height = Game::ElementLocationToPhysicalPosition(1);
	XMVECTOR size = XMVectorSetY(m_size, level_height);
	AABB slice = AABB(NULL_VECTOR, size);

	// Transform the ray into the local space of this AABB
	Ray local_ray = Ray(
		XMVector3TransformCoord(ray.Origin, m_inversezeropointworldmatrix),
		XMVector3TransformCoord(ray.Direction, m_inverseorientationmatrix));

	// Translate the local ray up or down by -/+(level*level_height) to account for 
	// the fact that the target level is now centred at 0,0,0
	local_ray.SetOrigin(XMVectorSubtract(local_ray.Origin, XMVectorSetY(NULL_VECTOR, ((float)level * level_height))));

	// Perform a ray/AABB intersection test to see if this floor is hit by the ray
	if (Game::PhysicsEngine.DetermineRayVsAABBIntersection(local_ray, slice))
	{
		// Get the AABB-local position of this intersection
		XMVECTOR local_pos = XMVectorMultiplyAdd(XMVector3NormalizeEst(local_ray.Direction), 
			XMVectorReplicate(Game::PhysicsEngine.RayIntersectionResult.tmin), local_ray.Origin);

		// We know this is a valid element within the slice, so determine and return the element location
		// Clamp the position value within bounds that will translate directly to an element index ([0,0,0] to (size-[eps,eps,eps]))
		// since we already know the collision is approximately within the ship bounds
		static const AXMVECTOR clamp_neg = XMVectorSetW(NULL_VECTOR, FLT_MAX_NEG);			// Since XMVectorClamp() will assert Min<=Max, and the W component may not be
		outElement = Game::PhysicalPositionToElementLocation(XMVectorClamp(local_pos, clamp_neg, XMVectorAdd(size, Game::C_EPSILON_NEG_V)));
		outElement.z = level;
		return true;
	}
	else
	{
		// There is no intersection at all, so return false immediately
		return false;
	}
}

// Determines the sequence of elements intersected by a world-space ray.  Returns a flag indicating 
// whether any intersection does take place
bool iSpaceObjectEnvironment::DetermineElementPathIntersectedByRay(const Ray & ray, float ray_radius, ElementIntersectionData & outElements)
{
	// Locate the entry point of the ray in this environment (and early-exit if there is no intersection)
	INTVECTOR3 start_el;
	if (DetermineElementIntersectedByRay(ray, start_el) == false) return false;

	// Maintain a (bitstring) vector of all elements that have been tested for intersection, to avoid infinite loops
	std::vector<bool> checked = std::vector<bool>(m_elementcount, false);

	// Maintain a vector of elements to test intersections from; starting element is the first value
	std::vector<int> test;
	test.push_back(ELEMENT_INDEX(start_el.x, start_el.y, start_el.z));

	// Get a reference to the environment OBB data, forcing a recalculation at the same time if required
	const OrientedBoundingBox::CoreOBBData & obb = CollisionOBB.Data();

	// Create an AABB to represent element (0,0,0), with bounds expanded to account for the 'width' of the ray
	XMVECTOR extent = XMVectorAdd(Game::C_CS_ELEMENT_EXTENT_V, XMVectorReplicate(ray_radius));
	AABB bounds = AABB(XMVectorNegate(extent), extent);

	// Transform the ray into local environment space, and then translate it into element (0,0,0) space
	Ray localray = Ray(ray); 
	localray.TransformIntoCoordinateSystem(obb.Centre, obb.Axis);
	//XMVECTOR el0adj = XMVectorSubtract(obb.ExtentV, Game::C_CS_ELEMENT_MIDPOINT_NEG_V);	// Since El0 = -(obb_extent - el_midpoint)
	XMVECTOR ray_in_el0 = XMVectorAdd(localray.Origin, /*el0adj*/ obb.ExtentV);
	localray.SetOrigin(ray_in_el0);

	// Iterate until we have no more elements to test
	DBG_COLLISION_OUTPUT("> Beginning environment collision test\n");
	int id; INTVECTOR3 location;
	int current = -1;
	while (++current < (int)test.size())
	{
		// Retrieve the next element to be tested, and only proceed if we have not already processed it
		id = test[current];	
		DBG_COLLISION_OUTPUT(concat("    Testing element ")(id).str().c_str());
		if (checked[id] == true)
		{
			DBG_COLLISION_OUTPUT("...ALREADY TESTED\n");
			continue;
		}

		// Record the fact that we are now processing this element
		checked[id] = true; 
		const ComplexShipElement & el = m_elements[id];

		// Translate the ray into the local space of this element, based on an offset from (0,0,0)
		localray.SetOrigin(XMVectorSubtract(ray_in_el0, Game::ElementLocationToPhysicalPosition(el.GetLocation())));
		DBG_COLLISION_OUTPUT(concat(" ")(el.GetLocation().ToString()).str().c_str());

		// Test for a collision between this ray and the element AABB; if none, skip processing this element immediately
		if (Game::PhysicsEngine.DetermineRayVsAABBIntersection(localray, bounds) == false)
		{
			DBG_COLLISION_OUTPUT(": No collision\n");
			continue;
		}

		// We have an intersection.  First, add this to the list of elements that were intersected
		outElements.push_back(ElementIntersection(id, Game::PhysicsEngine.RayIntersectionResult.tmin, Game::PhysicsEngine.RayIntersectionResult.tmax));
		DBG_COLLISION_OUTPUT(concat(": COLLIDES (t=")(Game::PhysicsEngine.RayIntersectionResult.tmin)(", ")(Game::PhysicsEngine.RayIntersectionResult.tmax)("), adding { ").str().c_str());

		// Now consider all neighbours of this element for intersection, if they have not already been tested
		const int(&adj)[Direction::_Count] = el.AdjacentElements();
		for (int i = 0; i < Direction::_Count; ++i)
		{
			if (adj[i] != -1 && checked[adj[i]] == false)
			{
				DBG_COLLISION_OUTPUT(concat(adj[i])(" ").str().c_str());
				test.push_back(adj[i]);
			}
		}
		DBG_COLLISION_OUTPUT("}\n");
	}
 
	// Testing complete; return true since we know we had an intersection of some kind
	return true;
}


// Registers a new collision with this environment, calculates the effect and begins to apply the effects
// Returns a flag indicating whether the event was registered (there are several validations that may prevent this)
bool iSpaceObjectEnvironment::RegisterEnvironmentImpact(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact)
{
	// Parameter check; quit immediately if object is invalid, or if it is valid but we are already processing the collision
	if (!object || EnvironmentIsCollidingWithObject(object)) return false;

	// Calculate the effect of the intersection with this environment (if any)
	EnvironmentCollision collision;
	bool intersects = CalculateCollisionThroughEnvironment(object, impact, collision);
	if (intersects == false) return false;

	// Add this collision event to the collection and return success
	m_collision_events.push_back(collision);
	return true;
}

// Checks whether collision of the specified object with this environment is already being simulated
bool iSpaceObjectEnvironment::EnvironmentIsCollidingWithObject(iActiveObject *object)
{
	if (HaveActiveEnvironmentCollisionEvents() && object)
	{
		std::vector<EnvironmentCollision>::const_iterator it_end = m_collision_events.end();
		for (std::vector<EnvironmentCollision>::const_iterator it = m_collision_events.begin(); it != it_end; ++it)
		{
			if ((*it).Collider() == object) return true;
		}
	}

	return false;
}

// Determines the effect of a collision with trajectory through the environment
// Returns a flag indicating whether a collision has occured, and data on all the collision events via "outResults"
bool iSpaceObjectEnvironment::CalculateCollisionThroughEnvironment(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact, EnvironmentCollision & outResult)
{
	// Parameter check
	if (!object) return false;

	// Get a reference to this environment and the colliding object (and validate at the same time)
	const GamePhysicsEngine::ImpactData::ObjectImpactData & impact_env = impact.GetObjectData(m_id);
	const GamePhysicsEngine::ImpactData::ObjectImpactData & impact_coll = impact.GetObjectData(object->GetID());
	if (impact_env.ID == 0U || impact_coll.ID == 0U) return false;		// If the collision was not between these objects
	
	// Initialise the output data object for this collision
	outResult.Collider = object;
	outResult.CollisionStartTime = Game::ClockTime;
	outResult.ClosingVelocity = XMVectorGetX(impact.TotalImpactVelocity);

	// Determine the trajectory and properties of the colliding object
	float proj_radius = object->GetCollisionSphereRadius();									// TODO: *** Need to get actual colliding cross-section ***
	Ray proj_trajectory = Ray(object->GetPosition(), impact_coll.PreImpactVelocity);	// Pre-impact velocity, since the collision handling will have adjusted this by now

	// Calcualate the path of elements intersected by this ray
	ElementIntersectionData elements;
	bool intersects = DetermineElementPathIntersectedByRay(proj_trajectory, proj_radius, elements);

	// Quit here if there is no intersection
	if (intersects == false)
	{
		outResult.IsActive = false;
		return false;
	}

	// Add an event for the processing of each intersected element
	ElementIntersectionData::const_iterator it_end = elements.end();
	for (ElementIntersectionData::const_iterator it = elements.begin(); it != it_end; ++it)
	{
		// Add an event for this collision.  This will automatically be sorted into the correct sequence
		// based upon start time.  This will almost always be ~equal to a push_back since 
		// intersections have generally been determined in temporal order
		const ElementIntersection & item = (*it);
		outResult.AddElementIntersection(item.ID, item.StartTime, (item.EndTime - item.StartTime));

		// Also push a copy of the intersection data into the result object for reference later, in case it is needed
		outResult.IntersectionData.push_back(*it);
	}

	// Safety check: there should always be >0 events since a collision has occured, but make sure of that here
	if (outResult.Events.empty()) { outResult.IsActive = false; return false; }

	// Return true to indicate that a collision occured
	outResult.IsActive = true;
	return true;
}

// Processes all active environment collisions at the current point in time.  Called as part of object simulation
void iSpaceObjectEnvironment::ProcessAllEnvironmentCollisions(void)
{
	// Iterate through all active collisions
	std::vector<EnvironmentCollision>::iterator c_it_end = m_collision_events.end();
	for (std::vector<EnvironmentCollision>::iterator c_it = m_collision_events.begin(); c_it != c_it_end; /* No increment */)
	{
		// Process each collision event in turn
		ProcessEnvironmentCollision(*c_it);

		// If the collision event is no longer active/valid, we can remove it from the vector here
		if ((*c_it).IsActive == false)		{ c_it = m_collision_events.erase(c_it); }
		else								{ ++c_it; }
	}
}

// Processes an environment collision at the current point in time.  Determines and applies all effects since the last frame
void iSpaceObjectEnvironment::ProcessEnvironmentCollision(EnvironmentCollision & collision)
{
	// Parameter check; make sure the collision is still active
	if (!collision.IsActive) return;

	// Loop through collision events while the object is still valid
	std::vector<EnvironmentCollision>::size_type event_count = collision.GetEventCount();
	while (collision.IsActive)
	{
		// Make sure the collider still has momentum; if not, the collision event is over
		if (collision.ClosingVelocity <= 0.0f) { collision.IsActive = false; return; }

		// Get the next event in the sequence
		std::vector<EnvironmentCollision>::size_type index = collision.GetNextEvent();
		if (index >= event_count) { collision.IsActive = false; return; }

		// Test whether the event is now ready to execute
		const EnvironmentCollision::EventDetails & ev = collision.Events[index];
		if (Game::ClockTime >= (collision.CollisionStartTime + ev.EventTime))
		{
			// Process the collision with this element. TODO: in future, account for event type and take different actions
			ExecuteElementCollision(ev, collision);

			// Move on to the next event (or inactivate the object if this was the final event in the sequence)
			collision.CurrentEventCompleted();
		}
		else
		{
			// We are not ready to run the next event yet, so quit here
			return;
		}
	}
}


// Executes the collision of an object with the specified object, as part of an environment collision event
void iSpaceObjectEnvironment::ExecuteElementCollision(const EnvironmentCollision::EventDetails ev, EnvironmentCollision & collision)
{
	// Get a reference to the element 
	if (ElementIndexIsValid(ev.EntityID) == false) return;
	const ComplexShipElement & el = GetElementDirect(ev.EntityID);
	const INTVECTOR3 & el_loc = el.GetLocation();

	// Get a reference to the colliding object
	iActiveObject *object = collision.Collider();
	if (!object) return;

	// Determine the current momentum of the object, which will be its normal impact resistance * the current closing velocity (not momentum, 
	// since already incl in impact resistance).  This gives us (obj_vel * obj_mass) + (env_vel * [1.0f]) - we don't want to account
	// for the environment momentum/mass here since we are dealing with the impact on only a very small part of the environment, that does
	// not share the same momentum of the entire structure
	float obj_force = object->GetImpactResistance() * collision.ClosingVelocity;		
	if (obj_force < 1.0f) return;

	// Get the aggregate stopping power of the element based on its properties, parent tile, and the actual contents of the element
	ComplexShipTile *tile = el.GetTile();
	float tile_strength = 0.0f, objects_strength = 0.0f, terrain_strength = 0.0f;
	std::vector<iEnvironmentObject*> objects;
	std::vector<StaticTerrain*> terrain;

	// First, the tile currently in this element (if any)
	if (tile)
	{
		tile_strength = tile->GetImpactResistance(el);
	}
	
	// Next, any objects or terrain in the element
	if (this->SpatialPartitioningTree)
	{
		EnvironmentTree *node = this->SpatialPartitioningTree->GetNodeContainingElement(el.GetLocation());
		if (node)
		{
			// Get all the objects & terrain in this tree node
			node->GetAllItems(objects, terrain);
			
			// Add the contribution from all objects in the element
			std::vector<iEnvironmentObject*>::const_iterator it_end = objects.end();
			for (std::vector<iEnvironmentObject*>::const_iterator it = objects.begin(); it != it_end; ++it)
			{
				if (*it && (*it)->GetElementLocation() == el_loc) objects_strength += (*it)->GetImpactResistance();
			}

			// Also add the contribution from all terrain in the element
			std::vector<StaticTerrain*>::const_iterator it2_end = terrain.end();
			for (std::vector<StaticTerrain*>::const_iterator it2 = terrain.begin(); it2 != it2_end; ++it2)
			{
				if (*it2 && (*it2)->GetElementLocation() == el_loc) terrain_strength += (*it2)->GetImpactResistance();
			}
		}
	}

	// Sum the total impact resistance and compare to the incoming object force
	float total_strength = (tile_strength + objects_strength + terrain_strength + 1.0f);	// Add 1.0f to ensure this is always >0.0
	float damage_pc = (obj_force / total_strength);

	// Reduce the object closing velocity based on this impact.  obj_force is currently (velocity * obj_impact_resistance),
	// so as a quick solution we can divide the remaining force through by obj_impact_resistance to get back to velocity
	// If this now takes the object velocity <= 0 it will trigger destruction of the collider in the next collision evaluation
	float obj_remaining_force = (obj_force - total_strength);
	collision.ClosingVelocity -= obj_remaining_force;

	// Assess the damage from this impact and apply to the element, potentially destroying it if the damage is sufficiently high
	float damage_abs = (el.GetHealth() * damage_pc);
	if (EnvironmentCollisionsAreBeingSimulated())
	{
		// Test whether we are simulating an environment collision, in which case we do not apply any real damage
		SimulateElementDamage(el.GetID(), damage_abs);
	}
	else
	{
		// Apply damage to the element, as normal
		TriggerElementDamage(el.GetID(), damage_abs);
	}
}

// Triggers damage to an element (and potentially its contents).  Element may be destroyed if sufficiently damaged
void iSpaceObjectEnvironment::TriggerElementDamage(int element_id, float damage)
{
	// Get a reference to the element
	if (ElementIndexIsValid(element_id) == false) return;
	ComplexShipElement & el = GetElementDirect(element_id);

	// Check this normalised [0.0 1.0] damage against the element health
	if (damage > el.GetHealth())
	{
		// If the damage is sufficiently high, trigger immediate destruction of the element and quit immediately
		TriggerElementDestruction(element_id);
		return;
	}

	// Otherwise we want to apply damage to the element
	el.SetHealth(el.GetHealth() - damage);

	// Notify any tile in this location that the element has been damaged
	if (el.GetTile()) el.GetTile()->ElementHealthChanged();

	// We may also apply damage to the contents of the element if the damage state is significant enough
	/*if (el.GetHealth() < Game::C_ELEMENT_DAMAGE_CONTENTS_THRESHOLD)
	{
	}*/
}

// SIMULATES damage to an element (and potentially its contents).  Element may be destroyed (in the simulation) if sufficiently damaged
void iSpaceObjectEnvironment::SimulateElementDamage(int element_id, float damage) const
{
	// Get a reference to the element
	if (ElementIndexIsValid(element_id) == false) return;
	const ComplexShipElement & el = GetConstElementDirect(element_id);

	// Check this normalised [0.0 1.0] damage against the element health
	if (damage > el.GetHealth())
	{
		// If the damage is sufficiently high, trigger immediate destruction of the element and quit immediately
		iSpaceObjectEnvironment::EnvironmentCollisionSimulationResults.push_back(
			SimulatedEnvironmentCollisionEvent(SimulatedEnvironmentCollisionEventType::ElementDestroyed, element_id, 0.0f));
		return;
	}

	// Otherwise we want to apply damage to the element
	iSpaceObjectEnvironment::EnvironmentCollisionSimulationResults.push_back(
		SimulatedEnvironmentCollisionEvent(SimulatedEnvironmentCollisionEventType::ElementDamaged, element_id, damage));
}

// Triggers immediate destruction of an element
void iSpaceObjectEnvironment::TriggerElementDestruction(int element_id)
{
	// Get a reference to the element
	if (ElementIndexIsValid(element_id) == false) return;
	ComplexShipElement & el = GetElementDirect(element_id);
	const INTVECTOR3 & el_loc = el.GetLocation();

	// Update the element state
	el.SetHealth(0.0f);

	// Notify any tile in this location that the element has been damaged
	if (el.GetTile()) el.GetTile()->ElementHealthChanged();

	// All objects and terrain in the element are in trouble
	if (this->SpatialPartitioningTree)
	{
		EnvironmentTree *node = this->SpatialPartitioningTree->GetNodeContainingElement(el.GetLocation());
		if (node)
		{
			// Get all the objects & terrain in this tree node
			std::vector<iEnvironmentObject*> objects; 
			std::vector<StaticTerrain*> terrain;
			node->GetAllItems(objects, terrain);

			// Add the contribution from all objects in the element
			std::vector<iEnvironmentObject*>::size_type n_obj = objects.size();
			for (std::vector<iEnvironmentObject*>::size_type i_obj = 0U; i_obj < n_obj; ++i_obj)
			{
				if (objects[i_obj] && objects[i_obj]->GetElementLocation() == el_loc) objects[i_obj]->DestroyObject();
			}

			// Also add the contribution from all terrain in the element
			std::vector<StaticTerrain*>::size_type n_ter = terrain.size();
			for (std::vector<StaticTerrain*>::size_type i_ter = 0U; i_ter < n_ter; ++i_ter)
			{
				if (terrain[i_ter] && terrain[i_ter]->GetElementLocation() == el_loc) terrain[i_ter]->DestroyObject();
			}
		}
	}
}

// Enable the ability to simulate environment collisions
void iSpaceObjectEnvironment::EnableEnvironmentCollisionSimulationMode(const iSpaceObjectEnvironment *env)
{
	// Reset all simulation data first
	iSpaceObjectEnvironment::DisableEnvironmentCollisionSimulationMode();

	// Now initialise for the specified environment, if applicable
	if (env)
	{
		iSpaceObjectEnvironment::EnvironmentCollisionSimulationResults.EnvironmentID = env->GetID();
	}
}

// Disable the ability to simulate environment collisions
void iSpaceObjectEnvironment::DisableEnvironmentCollisionSimulationMode(void)
{
	// Clear and reset the simulation data
	iSpaceObjectEnvironment::EnvironmentCollisionSimulationResults.Reset();
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

	// Create a new root node and initialise as the root node for this environment
	SpatialPartitioningTree = EnvironmentTree::_MemoryPool->RequestItem(); // new EnvironmentTree(this);
	SpatialPartitioningTree->Initialise(this);

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

// Determines the set of connections from other tiles that surround this element
void iSpaceObjectEnvironment::GetNeighbouringTiles(ComplexShipTile *tile, bool(&outConnects)[4], std::vector<TileAdjacency> & outNeighbours)
{
	// Initialise all connection results to false by default
	for (int i = 0; i < 4; ++i) outConnects[i] = false;

	// Parameter check
	if (!tile) return;
	const INTVECTOR3 & location = tile->GetElementLocation();
	const INTVECTOR3 & size = tile->GetElementSize();

	// Check each direction in turn
	for (int i = 0; i < 4; ++i)
	{
		Direction dir = (Direction)i;

		// Determine the range of elements on this 'face' of the tile.  This will always
		// be the element size in two dimensions and 1 in the third
		bool x_dim = (dir == Direction::Up || dir == Direction::Down);
		INTVECTOR3 el_incr = INTVECTOR3(
			(x_dim ? size.x : 1), (x_dim ? 1 : size.y), size.z);

		// Loop across the range of elements in this direction
		INTVECTOR3 el, incr; ComplexShipElement *element;
		ComplexShipTile *adj_tile; int adj_index;
		for (int x = 0; x < el_incr.x; ++x)
		{
			for (int y = 0; y < el_incr.y; ++y)
			{
				for (int z = 0; z < el_incr.z; ++z)
				{
					// Get the element in question
					incr = INTVECTOR3(x, y, z);
					el = (location + incr);
					element = GetElement(el);
					if (!element) continue;

					// Try to retrieve the neighbouring element; if none, we are at the edge and can skip this case
					adj_index = element->GetNeighbour(dir);
					if (adj_index < 0 || adj_index >= m_elementcount) continue;
					const ComplexShipElement & adj_el = m_elements[adj_index];

					// Test whether this element contains a tile; if so, we want to record that fact
					adj_tile = adj_el.GetTile();
					if (adj_tile)
					{
						outConnects[i] = true;
						outNeighbours.push_back(TileAdjacency(incr, dir, adj_el.GetLocation(), adj_tile));
					}
				}
			}
		}
	}
}

// Updates the connection state of the specified tile based on its neighbours.  Ensures a bi-directional
// connection is setup and that the adjacent tile is also updated
void iSpaceObjectEnvironment::UpdateTileConnectionState(ComplexShipTile **ppTile)
{
	// Parameter check
	if (!ppTile) return;
	ComplexShipTile *tile = (*ppTile);
	if (!tile) return;

	// We will not be able to make any updates if the connections from this tile are
	// fixed; in this case, simply quit immediately
	if (tile->ConnectionsAreFixed()) return;

	// We will reset all connections before re-calculating them below.  Save a copy of the current
	// connection state so we can test whether it changed at the end
	TileConnections old_state = TileConnections(tile->Connections);
	tile->Connections.ResetConnectionState();

	// Find all neighbours of this tile
	bool neighbours[4]; 
	std::vector<TileAdjacency> adjacent;
	GetNeighbouringTiles(tile, neighbours, adjacent);

	// Check each adjacent tile in turn
	std::vector<TileAdjacency>::iterator it_end = adjacent.end();
	for (std::vector<TileAdjacency>::iterator it = adjacent.begin(); it != it_end; ++it)
	{
		// Get a reference to the adjacency data
		TileAdjacency & adj = (*it);
		
		// Get the ship-relative element under the tile which is being considered here
		INTVECTOR3 actual_el_loc = (tile->GetElementLocation() + adj.Location);
		ComplexShipElement *actual_el = GetElement(actual_el_loc);
		if (!actual_el) continue;

		// Determine the element within the target tile which would be making this connection
		int adj_el_index = actual_el->GetNeighbour(adj.AdjDirection);
		ComplexShipElement *adj_el = GetElement(adj_el_index);
		if (!adj_el) continue;

		// Attempt to locate the neighbouring tile that spans this adjacent element
		// We can proceed no further if the tile does not exist, or if it is fixed and cannot be modified
		ComplexShipTile *adjtile = FindTileAtLocation(adj_el->GetLocation());
		if (!adjtile || adjtile->ConnectionsAreFixed()) continue;

		// The element within the target tile will be the absolute element location minus the target tile location
		INTVECTOR3 adj_loc = (adj_el->GetLocation() - adjtile->GetElementLocation());

		// Test all possible connection types
		DirectionBS dirBS = DirectionToBS(adj.AdjDirection);
		DirectionBS invDirBS = GetOppositeDirectionBS(dirBS);
		bool target_updated = false;
		for (int i = 0; i < (int)TileConnections::TileConnectionType::_COUNT; ++i)
		{
			TileConnections::TileConnectionType type = (TileConnections::TileConnectionType)i;

			// Test whether it is possible to make a connection between the two tiles at this point
			if (tile->PossibleConnections.ConnectionExists(type, adj.Location, dirBS) &&			// Tile > Adj
				adjtile->PossibleConnections.ConnectionExists(type, adj_loc, invDirBS))			// Adj > Tile
			{
				// It is; make the connection from this tile, since we previously reset all connections
				tile->Connections.AddConnection(type, adj.Location, dirBS);

				// Make the reciprocal connection IF it does not already exist
				if (adjtile->Connections.ConnectionExists(type, adj_loc, invDirBS) == false)
				{
					adjtile->Connections.AddConnection(type, adj_loc, invDirBS);
					target_updated = true;
				}
			}
		}

		// Trigger an update of the adjacent tile if any connections were updated
		if (target_updated) UpdateTileBasedOnConnectionData(&(adjtile));
	}

	// We have processed all adjacent tiles; test whether any connections were changed
	// in this tile as part of the update
	bool tile_updated = (!tile->Connections.Equals(old_state));
	if (tile_updated)
	{
		// The subject tile was modified, so finally trigger an update of that tile 
		UpdateTileBasedOnConnectionData(ppTile);
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


// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void iSpaceObjectEnvironment::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(GetElement, command.ParameterAsInt(2), command.ParameterAsInt(3), command.ParameterAsInt(4))
	REGISTER_DEBUG_ACCESSOR_FN(GetElementByIndex, command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(GetElementSize)
	REGISTER_DEBUG_ACCESSOR_FN(GetElementCount)
	REGISTER_DEBUG_ACCESSOR_FN(GetElementIndex, INTVECTOR3(command.ParameterAsInt(2), command.ParameterAsInt(3), command.ParameterAsInt(4)))
	REGISTER_DEBUG_ACCESSOR_FN(GetZeroPointTranslation)
	REGISTER_DEBUG_ACCESSOR_FN(GetZeroPointWorldMatrix)
	REGISTER_DEBUG_ACCESSOR_FN(GetInverseZeroPointWorldMatrix)
	REGISTER_DEBUG_ACCESSOR_FN(IsEnvironmentUpdateSuspended)
	REGISTER_DEBUG_ACCESSOR_FN(ContainsSimulationHubs)
	REGISTER_DEBUG_ACCESSOR_FN(GetNavNetwork)
	REGISTER_DEBUG_ACCESSOR_FN(IsValidElementID, command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(GetDeckIndex, command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(ElementIndexToLocation, command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(GetElementContainingPositionUnbounded, XMVectorSet(command.ParameterAsFloat(2), command.ParameterAsFloat(3), command.ParameterAsFloat(4), 0.0f))
	REGISTER_DEBUG_ACCESSOR_FN(GetClampedElementContainingPosition, XMVectorSet(command.ParameterAsFloat(2), command.ParameterAsFloat(3), command.ParameterAsFloat(4), 0.0f))


	// Mutator methods
	REGISTER_DEBUG_FN(BuildSpatialPartitioningTree)
	REGISTER_DEBUG_FN(SimulateObject)
	REGISTER_DEBUG_FN(PerformPostSimulationUpdate)
	REGISTER_DEBUG_FN(SetSimulationStateOfEnvironmentContents, (iObject::ObjectSimulationState)command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(UpdateGravity)
	REGISTER_DEBUG_FN(UpdateOxygenLevels)
	REGISTER_DEBUG_FN(RefreshPositionImmediate)
	REGISTER_DEBUG_FN(UpdateEnvironment)
	REGISTER_DEBUG_FN(SuspendEnvironmentUpdates)
	REGISTER_DEBUG_FN(ResumeEnvironmentUpdates)
	REGISTER_DEBUG_FN(NotifyIsContainerOfSimulationHubs, command.ParameterAsBool(2))
	REGISTER_DEBUG_FN(ClearAllTerrainObjects)
	REGISTER_DEBUG_FN(ShutdownNavNetwork)
	REGISTER_DEBUG_FN(UpdateNavigationNetwork)
	REGISTER_DEBUG_FN(UpdateElementSpaceStructure)


	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		iContainsComplexShipTiles::ProcessDebugCommand(command);
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		Ship::ProcessDebugCommand(command);
}

