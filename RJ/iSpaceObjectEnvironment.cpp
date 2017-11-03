#include <vector>
#include "ErrorCodes.h"
#include "Utility.h"
#include "FastMath.h"
#include "GameVarsExtern.h"
#include "Logging.h"
#include "Collections.h"
#include "ComplexShipElement.h"
#include "CoreEngine.h"
#include "OverlayRenderer.h"
#include "BasicColourDefinition.h"
#include "iEnvironmentObject.h"
#include "ComplexShipTileDefinition.h"
#include "ComplexShipTile.h"
#include "Hardpoint.h"
#include "DynamicTileSet.h"
#include "Terrain.h"
#include "TerrainDefinition.h"
#include "Ship.h"
#include "iContainsComplexShipTiles.h"
#include "NavNetwork.h"
#include "EnvironmentTree.h"
#include "CSLifeSupportTile.h"
#include "Ray.h"
#include "AABB.h"
#include "OrientedBoundingBox.h"
#include "EnvironmentOBBRegion.h"
#include "ElementIntersection.h"
#include "EnvironmentCollision.h"
#include "SimulatedEnvironmentCollision.h"
#include "Frustum.h"
#include "PortalRenderingSupport.h"
#include "DynamicTerrain.h"
#include "DataEnabledEnvironmentObject.h"

#include "iSpaceObjectEnvironment.h"


// Initialise static data
const iSpaceObjectEnvironment::DeckInfo iSpaceObjectEnvironment::NULL_DECK = iSpaceObjectEnvironment::DeckInfo();

// Initialise static working vector for environment object search; holds nodes being considered in the search
std::vector<EnvironmentTree*> iSpaceObjectEnvironment::m_search_nodes;
SimulatedEnvironmentCollision iSpaceObjectEnvironment::EnvironmentCollisionSimulationResults;;

// Default constructor
iSpaceObjectEnvironment::iSpaceObjectEnvironment(void)
	:
	// Environment maps
	m_oxygenmap(this), m_powermap(this)
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
	m_lastoxygenupdatetime = Game::TimeFactor - 1.0f;
	m_nextoxygenupdate = Game::ClockMs;
	m_gravityupdaterequired = true;
	m_powerupdaterequired = true;
	m_oxygenupdaterequired = true;
	m_gravity_override = -1.0f;
	m_portalrenderingsupported = false;
	m_overrideportalrenderingsupport = PortalRenderingSupport::DetermineAutomatically;
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

	// Create new environment maps that reference this new instance
	BuildAllEnvironmentMaps();

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

	// Properties which are updated based on an event flag
	if (GravityUpdateRequired())		PerformGravityUpdate();
	if (PowerUpdateRequired())			PerformPowerUpdate();

	// Properties which are updated on a periodic basis
	if (OxygenUpdateRequired())			PerformOxygenUpdate();

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

	// If we have an override in place, simply apply it now and early-exit
	if (m_gravity_override >= 0.0f)
	{
		ComplexShipElement *el = m_elements;
		for (int i = 0; i < m_elementcount; ++i, ++el)
		{
			el->ChangeGravityStrength(m_gravity_override);
		}
		return;
	}

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

// Performs an update of environment power levels, based on each power source in the ship
void iSpaceObjectEnvironment::PerformPowerUpdate(void)
{
	// Reset the update flag now we are performing an update
	m_powerupdaterequired = false;

	// Update the map.  All power updates are instant so we don't need to pass the time delta
	m_powermap.Update(Game::TimeFactor);

	// Notify environment components of their new power level
	ComplexShipTile *tile;
	ComplexShipTileCollection::iterator it_end = m_tiles[0].end();
	for (ComplexShipTileCollection::iterator it = m_tiles[0].begin(); it != it_end; ++it)
	{
		tile = (*it).value;
		const INTVECTOR3 & location = tile->GetElementLocation();
		const INTVECTOR3 & size = tile->GetElementSize();

		Power::Type max_power = 0;
		for (int x = 0; x < size.x; ++x)
			for (int y = 0; y < size.y; ++y)
				for (int z = 0; z < size.z; ++z)
					max_power = max(max_power, m_powermap.GetPowerLevel(
						ELEMENT_INDEX(location.x + x, location.y + y, location.z + z)));

		tile->SetPowerLevel(max_power);
	}
}

// Performs an update of environment oxygen levels, based on each life support system in the ship
void iSpaceObjectEnvironment::PerformOxygenUpdate(void)
{
	// Reset relevant flags now we are performing an update
	float timedelta = (Game::ClockTime - m_lastoxygenupdatetime);
	m_lastoxygenupdatetime = Game::ClockTime; 
	m_oxygenupdaterequired = false;
	m_nextoxygenupdate = DetermineNextOxygenUpdateTime();

	// Update the environment oxygen map based on the time since the last update
	m_oxygenmap.Update(timedelta);
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

	// Only add the object if it does not already exist
	if (Objects.end() != std::find_if(Objects.begin(), Objects.end(),
		[&obj](const ObjectReference<iEnvironmentObject> & element) { return (element() == obj); })) return;
	
	// Add to the objects collection
	Objects.push_back(ObjectReference<iEnvironmentObject>(obj));

	// Add to the spatial partitioning tree
	if (SpatialPartitioningTree) SpatialPartitioningTree->AddItem<iEnvironmentObject*>(obj);

	// Integrate this object into the data environment if it is data-enabled
	if (obj->IsDataEnabled()) RegisterDataEnabledObject(static_cast<DataEnabledEnvironmentObject*>(obj));
	
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

		// Remove this object from the data environment if it is data-enabled
		if (obj->IsDataEnabled()) UnregisterDataEnabledObject(static_cast<DataEnabledEnvironmentObject*>(obj));

		// Remove from the collection
		Objects.erase(it);
	}
}

// Adds a terrain object to the environment
void iSpaceObjectEnvironment::AddTerrainObject(Terrain *obj)
{
	// Make sure the terrain object is valid
	if (!obj) return;

	// Make sure the terrain object does not already exist in this environment
	if (std::find(TerrainObjects.begin(), TerrainObjects.end(), obj) != TerrainObjects.end()) return;

	// Add to the terrain collection, and set the reverse terrain pointer to this environment
	TerrainObjects.push_back(obj);
	obj->SetParentEnvironment(this);

	// Also add a reference in the spatial partitioning tree
	if (SpatialPartitioningTree) SpatialPartitioningTree->AddItem<Terrain*>(obj);

	// Integrate this object into the data environment if it is data-enabled
	if (obj->IsDataEnabled()) RegisterDataEnabledObject(static_cast<DynamicTerrain*>(obj));
}

// Removes a terrain object from the environment.  Optionally takes a second parameter indicating the index of this 
// object in the terrain collection; if set, and if the index is correct, it will be used rather than performing
// a search of the collection for the object.  Deallocates the terrain object.
void iSpaceObjectEnvironment::RemoveTerrainObject(Terrain *obj)
{
	// Make sure the terrain object is valid
	if (!obj) return;

	// Remove the reverse terrain pointer back to its environment
	obj->SetParentEnvironment(NULL);

	// Remove from the terrain object collection
	std::vector<Terrain*>::iterator it = std::find(TerrainObjects.begin(), TerrainObjects.end(), obj);
	if (it != TerrainObjects.end()) TerrainObjects.erase(it);

	// Remove from the spatial partitioning tree
	if (obj->GetEnvironmentTreeNode()) obj->GetEnvironmentTreeNode()->RemoveItem<Terrain*>(obj);

	// Remove this object from the data environment if it is data-enabled
	if (obj->IsDataEnabled()) UnregisterDataEnabledObject(static_cast<DynamicTerrain*>(obj));

	// Finally, deallocate the terrain object
	SafeDelete(obj);
}

// Removes all terrain objects from an environment
void iSpaceObjectEnvironment::ClearAllTerrainObjects(void)
{
	// Deallocate all terrain objects and then clear the collection
	std::vector<Terrain*>::iterator it_end = TerrainObjects.end();
	for (std::vector<Terrain*>::iterator it = TerrainObjects.begin(); it != it_end; ++it)
	{
		if (*it) (delete (*it));
	}
	TerrainObjects.clear();

	// Perform a full rebuild of the spatial partitioning tree; likely more efficient than removing one-by-one
	BuildSpatialPartitioningTree();
}

// Specialised method to add a new terrain object that is part of a tile.  Object will be transformed from tile-relative to
// environment-relative position & orientation and then added to the environment as normal
void iSpaceObjectEnvironment::AddTerrainObjectFromTile(Terrain *obj, ComplexShipTile *sourcetile)
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

	// Add a reference from the terrain object to its parent tile object
	obj->SetParentTileID(sourcetile->GetID());

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

	// Add any hardpoints that come with this tile
	InstantiateHardpointsFromTile(tile_obj);

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
void iSpaceObjectEnvironment::RemoveTile(ComplexShipTile *tile, bool deallocate)
{
	// Parameter check
	if (!tile) return;

	// Raise the pre-removal event
	BeforeTileRemoved(tile);

	// Remove from the tile collection
	RemoveShipTile(tile);

	// Remove any terrain objects that came with this tile
	RemoveTerrainObjectsFromTile(tile);

	// Remove any hardpoints that came with this tile
	RemoveHardpointsFromTile(tile);
	
	// Raise the post-removal event
	TileRemoved(tile);

	// Update the environment
	UpdateEnvironment();

	// Deallocate the tile if it has been requested
	if (deallocate) SafeDelete(tile);
}

// Removes all tiles from the environment
void iSpaceObjectEnvironment::RemoveAllTiles(bool deallocate)
{
	// Suspend environment updates until all changes have been made
	SuspendEnvironmentUpdates(); 
	SuspendTileRecalculation();

	// Take this longer approach rather than just while (!empty) in case there is any issue preventing a tile 
	// from being removed from the collection
	int count = GetTileCount();
	while (--count >= 0)
	{
		RemoveTile(GetTile(0), deallocate);
	}

	// If any tiles remain we have an issue
	if (HasTiles())
	{
		Game::Log << LOG_ERROR << "Failed to remove all tiles from environment \"" << m_instancecode << "\"; " << GetTileCount() << " tiles remain\n";
	}

	// Resume updates following all changes
	ReactivateTileRecalculation();
	ResumeEnvironmentUpdates();
}

// Revert a tile to its base definition.  Can modify the contents and order of the tile collection
Result iSpaceObjectEnvironment::RevertTileToBaseDefinition(int tile_index)
{
	if (tile_index < 0 || tile_index >= GetTileCount()) return ErrorCodes::CannotRevertInvalidTileToBaseDefinition;
	ComplexShipTile *current_tile = GetTile(tile_index);
	if (!current_tile) return ErrorCodes::CannotRevertInvalidTileToBaseDefinition;

	// Remove the current tile, but keep it alive for now so we can replicate it
	RemoveTile(current_tile, false);

	// Create a new tile based on the definition and replicate required parameters
	Result result = ErrorCodes::NoError;
	const ComplexShipTileDefinition *def = current_tile->GetTileDefinition();
	if (def)
	{
		ComplexShipTile *tile = def->CreateTile();
		tile->CopyBasicProperties(*current_tile);

		result = tile->CompileAndValidateTile();
		if (result == ErrorCodes::NoError)
		{
			AddTile(&tile);
		}
	}

	// Deallocate the old tile now that we have replicated it
	SafeDelete(current_tile);
	return result;
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
		std::vector<Terrain*>::const_iterator it_end = def->TerrainObjects.end();
		for (std::vector<Terrain*>::const_iterator it = def->TerrainObjects.begin(); it != it_end; ++it)
		{
			// Create a copy of the terrain object, if it is valid
			if (!(*it)) continue;
			Terrain *terrain = (*it)->Copy();
			if (!terrain) continue;

			// Add this terrain object to the environment; method will determine the correct ship-relative position & orientation
			// based on the tile-relative position & orientation currently stored in the terrain object
			AddTerrainObjectFromTile(terrain, tile);
		}
	}
}

// Instantiate any hardpoints that are associated with the given new tile
void iSpaceObjectEnvironment::InstantiateHardpointsFromTile(ComplexShipTile *tile)
{
	if (!tile) return;
	const ComplexShipTileDefinition *def = tile->GetTileDefinition();
	if (!def) return;

	std::vector<Hardpoint*>::const_iterator it_end = def->GetHardpoints().end();
	for (std::vector<Hardpoint*>::const_iterator it = def->GetHardpoints().begin(); it != it_end; ++it)
	{
		Hardpoint *src = (*it); if (!src) continue;
		Hardpoint *hp = src->Clone(); if (!hp) continue;

		// Give the hardpoint a unique code based upon the parent tile.  Make sure there is no duplicate
		hp->Code = tile->DetermineTileHardpointCode(hp);
		if (hp->Code == NullString || GetHardpoints().Get(hp->Code) != NULL)
		{
			Game::Log << LOG_WARN << concat("Attempted to add hardpoint \"")(hp->Code)("\" from tile \"")(tile->GetCode())
				("\" to environment \"")(m_instancecode)("\" but hardpoint is either null or already exists\n").str();
			hp->Delete(); continue;
		}

		// Add the hardpoint to this environment
		this->GetHardpoints().AddHardpoint(hp);

		// The tile should maintain a reference to this hardpoint
		tile->AddHardpointReference(hp->Code);
	}
}

// Remove any hardpoints that are associated with the given tile.  Any equipment mounted on the hardpoints will
// be discarded
void iSpaceObjectEnvironment::RemoveHardpointsFromTile(ComplexShipTile *tile)
{
	if (!tile) return;

	// Iterate through all hardpoints owned by this tile
	std::vector<std::string>::const_iterator it_end = tile->GetHardpointReferences().end();
	for (std::vector<std::string>::const_iterator it = tile->GetHardpointReferences().begin(); it != it_end; ++it)
	{
		
		Hardpoint *hp = GetHardpoints().Get(*it);
		if (!hp)
		{
			Game::Log << LOG_WARN << concat("Attempted to remove hardpoint \"")(*it)("\" during removal of tile \"")(tile->GetCode())
				("\" from environment \"")(m_instancecode)("\" but hardpoint could not be located\n").str();
			continue;
		}

		// Remove from the environment
		GetHardpoints().DeleteHardpoint(*it);
	}

	// Clear the set of hardpoint references within this tile (since it no longer owns any hardpoints)
	tile->ClearAllHardpointReferences();
}

// Removes all terrain objects associated with a tile from the environment
void iSpaceObjectEnvironment::RemoveTerrainObjectsFromTile(ComplexShipTile *tile)
{
	// Parameter check
	if (!tile) return;

	// Remove any terrain objects in the environment that were owned by this tile
	Game::ID_TYPE id = tile->GetID();
	Collections::DeleteErase(TerrainObjects, [id](const Terrain *terrain) { return (terrain->GetParentTileID() == id); });
	
	// Rebuild the spatial partitioning tree following this bulk change
	BuildSpatialPartitioningTree();
}

// Removes all collision terrain objects associated with geometry of a specific tile from the environment
void iSpaceObjectEnvironment::RemoveCollisionTerrainFromTileGeometry(ComplexShipTile *tile)
{
	// Parameter check
	if (!tile) return;

	// Remove any terrain objects in the environment that were owned by this tile and which were generated from its collision geometry
	Game::ID_TYPE id = tile->GetID();
	Collections::DeleteErase(TerrainObjects, [id](const Terrain *terrain) 
	{ 
		return (terrain->GetParentTileID() == id && terrain->GetSourceType() == Terrain::TerrainSourceType::SourcedFromModel); 
	});

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

	// Reset all tile assignments and all non-automatic element properties.  All non-auto properties will be
	// re-derived during the environment update.  Automatically-derived properties (e.g. PROP_DESTROYED) are 
	// excluded from the reset
	for (int i = 0; i < m_elementcount; ++i)
	{
		m_elements[i].ResetElementState();
		m_elements[i].SetTile(NULL);
	}

	// The base section-relevant element properties will be set by our parent
	SetBaseEnvironmentProperties();
	
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
	m_deckcount = 0; m_deck_data.clear();
	for (int i = 0; i < (int)nZ; ++i)
	{
		if (decks[i])
		{
			m_deck_data.push_back(DeckInfo(m_deckcount, i, ELEMENT_INDEX(0, 0, i), (ELEMENT_INDEX(0, 0, i + 1) - 1)));
			++m_deckcount;
		}
	}
	assert(m_deckcount == (int)m_deck_data.size());

	// Identify the elements that make up this environment's outer hull
	BuildOuterHullModel();

	// Identify any hull breaches.  Dependent on outer hull model
	IdentifyHullBreaches();

	// Build detail caches on the state of environment elements
	BuildEnvironmentDetailCaches();

	// Rebuild the object bounding box to acccount for any elements that may have been destroyed
	BuildBoundingBoxHierarchy();

	// Verify all environment maps (power, data, oxygen, munitions, ...) and adjust them as required to fit 
	// with the new environment structure
	RevalidateEnvironmentMaps();

	// Update the environment navigation network given that connectivity may have changed
	UpdateNavigationNetwork();
	
	// Update the view portal configuration of all tiles in the environment
	UpdateViewPortalConfiguration();

	// Determine support for portal-based rendering based on the environment or any overrides
	DeterminePortalRenderingSupport();
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

// Update the view portal configuration of all tiles in the environment
void iSpaceObjectEnvironment::UpdateViewPortalConfiguration(void)
{
	// Process all tiles
	ComplexShipTileCollection::const_iterator it_end = m_tiles[0].end();
	for (ComplexShipTileCollection::const_iterator it = m_tiles[0].begin(); it != it_end; ++it)
	{
		ComplexShipTile *tile = (*it).value;
		XMVECTOR tile_centre = XMVectorAdd(tile->GetElementPosition(), tile->GetCentrePoint());

		// Process every portal in each tile
		for (auto & portal : tile->GetPortals())
		{
			// Determine the location of this portal within the environment.  We already know that it fits
			// entirely within the tile area (based on validation in the tile definition) so we can safely
			// determine the location based solely on our position
			XMVECTOR portal_centre = XMVectorAdd(tile_centre, portal.GetCentrePoint());
			INTVECTOR3 portal_location = Game::PhysicalPositionToElementLocation(portal_centre);

			// Clamp the portal element location to ensure it lies within an element owned by the tile.  We
			// need to do this to avoid (literal) edge cases where the portal is exactly on the outer bound
			// of the tile
			portal_location = IntVector3Clamp(portal_location, tile->GetElementLocation(), (tile->GetElementLocation() + tile->GetElementSize() - ONE_INTVECTOR3));
			assert(portal_location >= NULL_INTVECTOR3 && portal_location < m_elementsize);
			
			int portal_element = ElementLocationToIndex(portal_location);
			int target_element = ElementLocationToIndex(portal_location + DirectionUnitOffset(portal.GetTargetDirection()));
			assert(IsValidElementID(portal_element) && IsValidElementID(target_element));
			assert(portal_element != target_element);

			// Set the portal location and target based on this data
			portal.SetLocation(portal_element);
			portal.SetTargetLocation(target_element);			
		}
	}
}

// Build all environment maps (power, data, oxygen, munitions, ...)
Result iSpaceObjectEnvironment::BuildAllEnvironmentMaps(void)
{
	// Oxygen
	BuildEnvironmentOxygenMap();
	BuildEnvironmentPowerMap();

	// Return success
	return ErrorCodes::NoError;
}

// Build a specific environment map
void iSpaceObjectEnvironment::BuildEnvironmentOxygenMap(void)
{
	m_oxygenmap = EnvironmentOxygenMap(this);
	UpdateOxygen();
}

// Build a specific environment map
void iSpaceObjectEnvironment::BuildEnvironmentPowerMap(void)
{
	m_powermap = EnvironmentPowerMap(this);
	UpdatePower();
}

// Verifies all environment maps (power, data, oxygen, munitions, ...) and adjusts them as required to fit 
// with the new environment structure
Result iSpaceObjectEnvironment::RevalidateEnvironmentMaps(void)
{
	// Revalidate each map in turn.  Perform a full build if any cannot be incrementally rebuilt
	if (!m_oxygenmap.RevalidateMap())		BuildEnvironmentOxygenMap();
	if (!m_powermap.RevalidateMap())		BuildEnvironmentPowerMap();

	// Return success
	return ErrorCodes::NoError;
}


// Build detail caches on the state of environment elements
void iSpaceObjectEnvironment::BuildEnvironmentDetailCaches(void)
{
	// Reset the element property cache
	for (int i = 0; i < ComplexShipElement::PROPERTY::PROPERTY_MAX; ++i)
		m_element_property_count[i] = 0;

	// Traverse the element collection and update property counts where applicable
	for (int i = 0; i < m_elementcount; ++i)
	{
		bitstring properties = m_elements[i].GetProperties();
		for (int prop = 0; prop < ComplexShipElement::PROPERTY_COUNT; ++prop)
		{
			if (CheckBit_Any(properties, ComplexShipElement::PROPERTY_VALUES[prop]))
				++(m_element_property_count[prop]);
		}
	}
}

// Identify the elements that make up this environment's outer hull
void iSpaceObjectEnvironment::BuildOuterHullModel(void)
{
	// Reset all elements before building the model
	for (int i = 0; i < m_elementcount; ++i)
	{
		m_elements[i].ClearProperty(ComplexShipElement::PROPERTY::PROP_OUTER_HULL_ELEMENT);
	}

	// Constant array of parameters for the traversal process
	enum hull_param { x_start = 0, y_start, z_start, x_end, y_end, z_end, traverse_direction };
	const INTVECTOR3 & max_el = (m_elementsize - ONE_INTVECTOR3);
	const int params[6][7] = { 
		// { X_START, Y_START, Z_START, X_END, Y_END, Z_END, TRAVERSE_DIRECTION }
		{ 0, 0, 0, 0, max_el.y, max_el.z, (int)Direction::Right },					// Left side:	[0,0,0] to [0,y,z], increment +x
		{ 0, max_el.y, 0, max_el.x, max_el.y, max_el.z, (int)Direction::ZDown },	// Upper side:	[0,y,0] to [x,y,z], increment -y
		{ max_el.x, 0, 0, max_el.x, max_el.y, max_el.z, (int)Direction::Left },		// Right side:	[x,0,0] to [x,y,z], increment -x
		{ 0, 0, 0, max_el.x, 0, max_el.z, (int)Direction::ZUp },					// Bottom side:	[0,0,0] to [x,0,z], increment +y
		{ 0, 0, max_el.z, max_el.x, max_el.y, max_el.z, (int)Direction::Down },		// Front side:	[0,0,z] to [x,y,z], increment -z
		{ 0, 0, 0, max_el.x, max_el.y, 0, (int)Direction::Up }						// Rear side:	[0,0,0] to [x,y,0], increment +z
	};

	// Traverse each outer face in turn
	int index = -1;
	for (int face = 0; face < 6; ++face)
	{
		// Iterate over the elements of this face.  Note we can always ++ since #_start is always <= #_end
		for (int x = params[face][hull_param::x_start]; x <= params[face][hull_param::x_end]; ++x)
		{
			for (int y = params[face][hull_param::y_start]; y <= params[face][hull_param::y_end]; ++y)
			{
				for (int z = params[face][hull_param::z_start]; z <= params[face][hull_param::z_end]; ++z)
				{
					// Traverse inward from this element until we find the first intact element
					index = ELEMENT_INDEX(x, y, z);
					while (ElementIndexIsValid(index))
					{
						ComplexShipElement & el = GetElementDirect(index);
						if (!el.IsDestroyed())
						{
							// This is the outermost intact element, so mark it as a hull element
							el.SetProperty(ComplexShipElement::PROPERTY::PROP_OUTER_HULL_ELEMENT);
							break;
						}
						else
						{
							// This element has been destroyed so keep moving inwards
							index = el.GetNeighbour((Direction)params[face][hull_param::traverse_direction]);
						}
					}
				}
			}
		}
	}
}

// Identify any hull breaches.  Dependent on outer hull model
void iSpaceObjectEnvironment::IdentifyHullBreaches(void)
{
	// We will completely recalculate the breach collection
	HullBreaches.Reset();

	ComplexShipTile *tile;
	for (int i = 0; i < m_elementcount; ++i)
	{
		// Process every element looking for outer hull elements
		if (!m_elements[i].IsOuterHullElement()) continue;

		// We only need to consider cases where the element contains a tile (empty elements do not transmit anything)
		tile = m_elements[i].GetTile();
		if (!tile) continue;
		const INTVECTOR3 & el_loc = m_elements[i].GetLocation();
		int tile_element_index = tile->GetLocalElementIndex(tile->GetLocalElementLocation(el_loc));
		assert(tile_element_index != -1);

		// We only care about outer hull elements adjacent to destroyed elements
		// TODO: only need to consider a subset of 90-degree directions, since these are the directions we can connect in
		for (int a = 0; a < (int)Direction::_Count; ++a)	
		{
			int adj = m_elements[i].GetNeighbour((Direction)a);
			if (adj == -1 || !m_elements[adj].IsDestroyed()) continue;

			// Adj is a destroyed element next to outer hull element i & its tile.  Check whether there is a 
			// connection of tranmitting type (currently, only considering oxygen) from one to the other
			DirectionBS dirBS = DirectionToBS((Direction)a);
			if (tile->Connections.ConnectionExists(Oxygen::OXYGEN_TILE_TRANSMISSION_PROPERTY, tile_element_index, dirBS))
			{
				// Record this as a breach in the outer hull
				HullBreaches.RecordHullBreach(i, (Direction)a, adj);
			}
		}
	}
}

// Generates a bounding box hierarchy to represent the environment, accounting for any elements that may 
// have been destroyed
void iSpaceObjectEnvironment::BuildBoundingBoxHierarchy(void)
{
	// Determine the hierarchy of element-aligned bounding boxes that will encompass
	// all non-destroyed elements in this environment
	EnvironmentOBBRegion region = EnvironmentOBBRegion(0, m_elementsize);
	EnvironmentOBBRegionBuilder::Initialise(region);
	DetermineOBBRegionHierarchy(region);

	// Now process this region hierarchy and use it to construct the compound OBB
	BuildOBBFromRegionData(region);
}

// Internal recursive method for building the environment OBB hierarchy.  Returns the number of child
// regions created below this node, in the range [0 - 8]
EnvironmentOBBRegion::RegionState iSpaceObjectEnvironment::DetermineOBBRegionHierarchy(EnvironmentOBBRegion & region) const
{
	if (region.size == ONE_INTVECTOR3)
	{
		// Trivial case; single elements are either 100% or 0% complete
		return (m_elements[region.element].IsDestroyed() ? EnvironmentOBBRegion::RegionState::Empty 
														 : EnvironmentOBBRegion::RegionState::Complete);
	}
	else
	{
		// The region contains multiple elements, so subdivide it down into (maximum 8) sub-regions
		SubdivideOBBRegion(region);

		// Run this method recursively on each subregion
		EnvironmentOBBRegion::RegionState regionstate = EnvironmentOBBRegion::RegionState::Unknown;
		for (int i = 0; i < region.childcount; ++i)
		{
			// Run recursively on this child region and record its resulting state
			EnvironmentOBBRegion::RegionState state = DetermineOBBRegionHierarchy(EnvironmentOBBRegionBuilder::Get(region.children[i]));
			regionstate = EnvironmentOBBRegion::ApplyChildRegionState(regionstate, state);

			// If the child region is empty we do not need a node to cover this area
			// Also decrement the index here since we want to re-process this index, which now contains the next
			// child moved down
			if (state == EnvironmentOBBRegion::RegionState::Empty) { region.RemoveChild(i); --i; }
		}

		// If this entire region is complete we can remove all child nodes, since we don't need
		// any more granular detail for this area
		if (regionstate == EnvironmentOBBRegion::RegionState::Complete) region.RemoveAllChildren();

		// Return the state of this region and pass control back up the hierarchy
		return regionstate;
	}
}


// Builds the compound environment OBB based on calculated region data
void iSpaceObjectEnvironment::BuildOBBFromRegionData(const EnvironmentOBBRegion & region)
{
	// Deallocate any existing OBB data
	CollisionOBB.Clear();

	// Now recursively build an OBB that matches the hierarchical region structure
	BuildOBBNodeFromRegionData(CollisionOBB, region);

	// Invalidate the OBB so that the changes are reflected next frame
	CollisionOBB.UpdateFromParent();
}


// Recursively builds each node of the OBB that matches the supplied hierarchical region structure
void iSpaceObjectEnvironment::BuildOBBNodeFromRegionData(OrientedBoundingBox & obb, const EnvironmentOBBRegion & region)
{
	// Node size is easily calculated based on element size of the region
	XMVECTOR extent = XMVectorScale(Game::ElementLocationToPhysicalPosition(region.size), 0.5f);
	obb.UpdateExtent(extent);

	// Node offset is based upon the (top-left) element location of the region
	const ComplexShipElement & el = m_elements[region.element];
	obb.UpdateOffset(XMVectorSubtract(XMVectorAdd(							// Node position ==
		Game::ElementLocationToPhysicalPosition(el.GetLocation()),		//   Position of the region top-left element
		extent),														//   PLUS half the size of the region (to move from top-left > centre position of region)
		XMVectorScale(m_size, 0.5f)));									//   MINUS half the size of the environment itself (to move from el[0,0,0] > centre positon of environment)

	// Store index of the top-left element in the OBB index
	obb.SetIndex(region.element);

	// Now allocate space for the child nodes below this one and create each in turn
	obb.AllocateChildren(region.childcount);
	for (int i = 0; i < region.childcount; ++i)
	{
		BuildOBBNodeFromRegionData(obb.Children[i], EnvironmentOBBRegionBuilder::Get(region.children[i]));
	}
}


/* Internal method to subdivide a region into child nodes.  Return the number of subnodes created.  Output 
   parameter returns the new subnode data
   
   Example for [3x2x2] region --> subdivide into [2x1x1] + [1x1x1]:

   xyz	Pos		Size	Vol	Condition for creating subregion
   ===========================================================
   ---	0,0,0	2,1,1	2	if (true)
   +--	2,0,0	1,1,1	1	if (sx != 0)
   -+-	0,1,0	2,1,1	2	if (sy != 0)
   ++-	2,1,0	1,1,1	1	if (sx != 0 && sy != 0)

   --+	0,0,1	2,1,1	2	if (sz != 0)
   +-+	2,0,1	1,1,1	1	if (sz != 0 && sx != 0)
   -++	0,2,1	2,1,1	2	if (sz != 0 && sy != 0)
   +++	2,1,1	1,1,1	1	if (sz != 0 && sx != 0 && sy != 0)
*/
void iSpaceObjectEnvironment::SubdivideOBBRegion(EnvironmentOBBRegion & region) const
{
	const INTVECTOR3 & neg_pos = m_elements[region.element].GetLocation();
	INTVECTOR3 pos_size = region.size / 2;				// Positive == 'after' the centre point
	INTVECTOR3 neg_size = region.size - pos_size;		// Negative == 'before' the centre point
	INTVECTOR3 pos_pos = (neg_pos + neg_size);		

	// Precalc some conditions for efficiency
	bool x_and_y = (pos_size.x != 0 && pos_size.y != 0);

	// Add each region if required; first, those negative to the z-centre
	region.AddChild(region.element, neg_size);																									// ---
	if (pos_size.x != 0) region.AddChild(ELEMENT_INDEX(pos_pos.x, neg_pos.y, neg_pos.z), INTVECTOR3(pos_size.x, neg_size.y, neg_size.z));		// +--
	if (pos_size.y != 0) region.AddChild(ELEMENT_INDEX(neg_pos.x, pos_pos.y, neg_pos.z), INTVECTOR3(neg_size.x, pos_size.y, neg_size.z));		// -+-
	if (x_and_y) region.AddChild(ELEMENT_INDEX(pos_pos.x, pos_pos.y, neg_pos.z), INTVECTOR3(pos_size.x, pos_size.y, neg_size.z));				// ++-

	// Now add the other half of the regions, in the positive z direction
	if (pos_size.z != 0)
	{
		region.AddChild(ELEMENT_INDEX(neg_pos.x, neg_pos.y, pos_pos.z), INTVECTOR3(neg_size.x, neg_size.y, pos_size.z));						// --+
		if (pos_size.x != 0) region.AddChild(ELEMENT_INDEX(pos_pos.x, neg_pos.y, pos_pos.z), INTVECTOR3(pos_size.x, neg_size.y, pos_size.z));	// +-+
		if (pos_size.y != 0) region.AddChild(ELEMENT_INDEX(neg_pos.x, pos_pos.y, pos_pos.z), INTVECTOR3(neg_size.x, pos_size.y, pos_size.z));	// -++
		if (x_and_y) region.AddChild(ELEMENT_INDEX(pos_pos.x, pos_pos.y, pos_pos.z), INTVECTOR3(pos_size.x, pos_size.y, pos_size.z));			// +++
	}
}

// Returns the location of the element at the specified point within this OBB node
INTVECTOR3 iSpaceObjectEnvironment::DetermineElementAtOBBLocation(const OrientedBoundingBox & obb, const FXMVECTOR obb_local_pos)
{
	// Parameter check
	if (!IsValidElementID(obb.Index)) return NULL_INTVECTOR3;

	// Determine the element location within this OBB; add OBB extent to make this bottom-corner-relative
	INTVECTOR3 loc = Game::PhysicalPositionToElementLocation(XMVectorAdd(obb_local_pos, obb.ConstData().ExtentV));

	// Position of the OBB bottom-left corner is recorded in the OBB index
	INTVECTOR3 obb_loc = GetElementDirect(obb.Index).GetLocation();
	
	// Return the combined location; OBB location + location within OBB.  Clamp to ensure it is within the environment bounds
	return ClampElementLocationToEnvironment(loc + obb_loc);
}

// Shuts down and deallocates any nav network that currently exists for the environment
void iSpaceObjectEnvironment::ShutdownNavNetwork(void)
{
	if (m_navnetwork)
	{
		m_navnetwork->Shutdown();
		SafeDelete(m_navnetwork);
	}
}

// Determine support for portal-based rendering based on the environment or any overrides
void iSpaceObjectEnvironment::DeterminePortalRenderingSupport(void)
{
	// If any override is in place then simply use it and return
	if (m_overrideportalrenderingsupport != PortalRenderingSupport::DetermineAutomatically)
	{
		m_portalrenderingsupported = (m_overrideportalrenderingsupport == PortalRenderingSupport::ForceEnabled);
		return;
	}

	// Otherwise, we define an environment as supporting portal rendering if it contains cells that 
	// expose at least one portal
	auto it_end = m_tiles[0].end();
	for (auto it = m_tiles[0].begin(); it != it_end; ++it)
	{
		if ((*it).value->GetPortalCount() != 0U)
		{
			// We have at least one portal; enable portal rendering and early-out
			m_portalrenderingsupported = true;
			return;
		}
	}

	// We do not have any portals in the environment, so disable portal rendering
	m_portalrenderingsupported = false;
}

// Applies an override to automatic determination of portal-based rendering support
void iSpaceObjectEnvironment::OverridePortalBasedRenderingSupport(bool supported)
{
	m_overrideportalrenderingsupport = (supported ? PortalRenderingSupport::ForceEnabled : PortalRenderingSupport::ForceDisabled);
	DeterminePortalRenderingSupport();
}

// Removes any override on automatic determination of portal-based rendering support
void iSpaceObjectEnvironment::RemoveOverrideOfPortalBasedRenderingSupport(void)
{
	m_overrideportalrenderingsupport = PortalRenderingSupport::DetermineAutomatically;
	DeterminePortalRenderingSupport();
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

	// Recalculate the tile properties following this update to make sure everything is up-to-date
	(*ppOutTile)->RecalculateTileData();

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
	RemoveTile(old_tile, false);

	// We now want to add the new tile, assuming it exists
	if (new_tile)
	{
		ComplexShipTile **ppNewTile = &new_tile;		// To prevent the contents of the new_tile pointer being modified
		AddTile(ppNewTile);
	}

	// Resume updates, which will recalculate all tile-dependent data
	ReactivateTileRecalculation();
}

// Returns the deck data for the deck with specified index.  Returns a reference to a "null deck" if invalid parameter
const iSpaceObjectEnvironment::DeckInfo & iSpaceObjectEnvironment::GetDeckInformation(int deck) const
{
	if (deck < 0 || deck >= m_deckcount)		return iSpaceObjectEnvironment::NULL_DECK;
	else										return m_deck_data[deck];
}

// Returns the element index corresponding to the supplied deck.  Default 0 if invalid parameter
int iSpaceObjectEnvironment::GetDeckIndex(int deck) const
{
	if (deck < 0 || deck >= m_deckcount)		return 0;
	else										return m_deck_data[deck].ElementZIndex;
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
	static const AXMVECTOR clamp_pos_delta = XMVectorReplicate(-0.001f);				// Since a value of exactly (<=) m_size rather than (<) will map to the next element, which could be invalid
	outElement = Game::PhysicalPositionToElementLocation(XMVectorClamp(localpos, clamp_neg, XMVectorAdd(m_size, clamp_pos_delta)));
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
		static const AXMVECTOR clamp_pos_delta = XMVectorReplicate(0.001f);					// Since a value of exactly (<=) m_size rather than (<) will map to the next element, which could be invalid
		outElement = Game::PhysicalPositionToElementLocation(XMVectorClamp(local_pos, clamp_neg, XMVectorAdd(size, clamp_pos_delta)));
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
	XMVECTOR ray_in_el0 = XMVectorAdd(localray.Origin, obb.ExtentV);
	localray.SetOrigin(ray_in_el0);
	XMVECTOR norm_ray_dir = XMVector3NormalizeEst(localray.Direction);

	// Define the distance (squared) thresholds for the 'degree' of collision with each element
	// Minimum distance (100% collision) is defined as proj bounding sphere sq (== ray radius sq).  Max distance 
	// is (proj_sph_sq + el_sph_sq)
	float degree_min_dist = ray_radius;
	float degree_max_dist = (degree_min_dist + GetElementBoundingSphereRadius_Unchecked(1));
	float degree_max_minus_min = (degree_max_dist - degree_min_dist); 

	// Iterate until we have no more elements to test
	DBG_COLLISION_TEST("> Beginning environment collision test\n");
	int id; INTVECTOR3 location;
	int current = -1;
	while (++current < (int)test.size())
	{
		// Retrieve the next element to be tested, and only proceed if we have not already processed it & it is valid
		id = test[current];	
		assert(id >= 0 && id < m_elementcount);
		DBG_COLLISION_TEST(concat("    Testing element ")(id).str().c_str());
		if (checked[id] == true)
		{
			DBG_COLLISION_TEST("...ALREADY TESTED\n");
			continue;
		}
		else if (!IsValidElementID(id))		// This should not happen except in very very rare edge cases that may not exist any more, but just to be compleetely safe
		{
			DBG_COLLISION_TEST("...ELEMENT ID IS NOT VALID!\n");
			continue;
		}

		// Record the fact that we are now processing this element
		checked[id] = true; 
		const ComplexShipElement & el = m_elements[id];

		// Translate the ray into the local space of this element, based on an offset from (0,0,0)
		XMVECTOR elpos = Game::ElementLocationToPhysicalPosition(el.GetLocation());
		localray.SetOrigin(XMVectorSubtract(ray_in_el0, elpos));
		DBG_COLLISION_TEST(concat(" ")(el.GetLocation().ToString()).str().c_str());

		// Test for a collision between this ray and the element AABB; if none, skip processing this element immediately
		if (Game::PhysicsEngine.DetermineRayVsAABBIntersection(localray, bounds) == false)
		{
			DBG_COLLISION_TEST(": No collision\n");
			continue;
		}

		// We have an intersection.  Determine the degree of intersection based on how close to the centre of the ray 
		// this intersection occured.  Use the dot product A.B = (Origin>ElementCentre).(Origin>ProjDirection) where |B|=1
		// to get the distance along localray.  Then find the point which this represents, and get the distance from this
		// point on the ray to the element centre itself.  This is the distance between colliding object centres
		XMVECTOR dot = XMVector3Dot(XMVectorSubtract(NULL_VECTOR, localray.Origin), norm_ray_dir);
		float dist = XMVectorGetX(XMVector3LengthEst(XMVectorSubtract(NULL_VECTOR,					// Vector from the element position...
			XMVectorMultiplyAdd(norm_ray_dir, dot, localray.Origin))));								// ...to the point on the ray at Origin+(dot*Direction)

		// Use this distsq to determine the degree of intersection, based on a min/max dist threshold and 
		// degree = Clamp(1.0 - ((d - min_d) / (max_d - min_d)), 0.0, 1.0)
		float degree = (1.0f - ((dist - degree_min_dist) / degree_max_minus_min));
		degree = clamp(degree, 0.0f, 1.0f);

		// Add this intersection to the list of elements that were intersected
		outElements.push_back(ElementIntersection(id, Game::PhysicsEngine.RayIntersectionResult.tmin, Game::PhysicsEngine.RayIntersectionResult.tmax, degree));
		DBG_COLLISION_TEST(concat(": COLLIDES (t=[")(Game::PhysicsEngine.RayIntersectionResult.tmin)(", ")(Game::PhysicsEngine.RayIntersectionResult.tmax)("], degree=")(degree)("), adding { ").str().c_str());

		// Now consider all neighbours of this element for intersection, if they have not already been tested
		const int(&adj)[Direction::_Count] = el.AdjacentElements();
		for (int i = 0; i < Direction::_Count; ++i)
		{
			if (adj[i] != -1 && checked[adj[i]] == false)
			{
				DBG_COLLISION_TEST(concat(adj[i])(" ").str().c_str());
				test.push_back(adj[i]);
			}
		}
		DBG_COLLISION_TEST("}\n");
	}
 
	// Testing complete; return true since we know we had an intersection of some kind
	return true;
}

// Determine the total strength of an element within this environment.  Incorporates inherent strength of the 
// element, any tile that is present, plus all objects and terrain within the element
float iSpaceObjectEnvironment::DetermineTotalElementImpactStrength(const INTVECTOR3 & element_location)
{
	return DetermineTotalElementImpactStrength(GetElementIndex(element_location));
}

// Determine the total strength of an element within this environment.  Incorporates inherent strength of the 
// element, any tile that is present, plus all objects and terrain within the element
float iSpaceObjectEnvironment::DetermineTotalElementImpactStrengthAtLocation(const INTVECTOR3 & element_location)
{
	return DetermineTotalElementImpactStrength(element_location);
}

// Determine the total strength of an element within this environment.  Incorporates inherent strength of the 
// element, any tile that is present, plus all objects and terrain within the element
float iSpaceObjectEnvironment::DetermineTotalElementImpactStrength(int element_id)
{
	if (!IsValidElementID(element_id)) return 0.0f;
	return DetermineTotalElementImpactStrength(GetElementDirect(element_id));
}

// Determine the total strength of an element within this environment.  Incorporates inherent strength of the 
// element, any tile that is present, plus all objects and terrain within the element
float iSpaceObjectEnvironment::DetermineTotalElementImpactStrength(const ComplexShipElement & el)
{
	if (el.IsDestroyed()) return 0.0f;
	float tile_strength = 0.0f, objects_strength = 0.0f, terrain_strength = 0.0f;
	
	ComplexShipTile *tile = el.GetTile();
	std::vector<iEnvironmentObject*> objects;
	std::vector<Terrain*> terrain;

	// First, the inherent strength of the element.  This is scaled by the current element health to simulate
	// the loss of structural integrity as the hull takes more damage
	float el_strength = el.GetImpactResistance();

	// Next, the tile currently in this element (if any).  This is also scaled by the health of the underlying element
	if (tile)
	{
		tile_strength = tile->GetImpactResistance(el);
	}

	// Next, any objects or terrain in the element
	if (this->SpatialPartitioningTree)
	{
		INTVECTOR3 el_loc = el.GetLocation();
		EnvironmentTree *node = this->SpatialPartitioningTree->GetNodeContainingElement(el_loc);
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
			std::vector<Terrain*>::const_iterator it2_end = terrain.end();
			for (std::vector<Terrain*>::const_iterator it2 = terrain.begin(); it2 != it2_end; ++it2)
			{
				if (*it2 && (*it2)->GetElementLocation() == el_loc) terrain_strength += (*it2)->GetImpactResistance();
			}
		}
	}

	// Sum and return the total impact resistance 
	return (el_strength + tile_strength + objects_strength + terrain_strength + 1.0f);	// Add 1.0f to ensure this is always >0.0
}

// Registers a new collision with this environment, calculates the effect and begins to apply the effects
// Returns a flag indicating whether the event was registered (there are several validations that may prevent this)
bool iSpaceObjectEnvironment::RegisterEnvironmentImpact(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact)
{
	// Parameter check; quit immediately if object is invalid, or if it is valid but we are already processing the collision
	if (!object || EnvironmentIsCollidingWithObject(object)) return false;

	// Calculate the effect of the intersection with this environment (if any)
	EnvironmentCollision collision;
	bool intersects = CalculateCollisionThroughEnvironment(object, impact, true, collision);
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
bool iSpaceObjectEnvironment::CalculateCollisionThroughEnvironment(	iActiveObject *object, const GamePhysicsEngine::ImpactData & impact, 
																	bool external_collider, EnvironmentCollision & outResult)
{
	// Parameter check
	if (!object) return false;
	DBG_COLLISION_RESULT(concat(m_instancecode)(": Testing for collision of \"")(object->GetInstanceCode())
		("\" with environment").str().c_str());

	// Get a reference to this environment and the colliding object (and validate at the same time)
	const GamePhysicsEngine::ImpactData::ObjectImpactData & impact_env = impact.GetObjectData(m_id);
	const GamePhysicsEngine::ImpactData::ObjectImpactData & impact_coll = impact.GetObjectData(object->GetID());
	if (impact_env.ID == 0U || impact_coll.ID == 0U)
	{
		DBG_COLLISION_RESULT(", collision data mismatch, NO COLLISION\n");
		return false;		// If the collision was not between these objects
	}
	
	// Initialise the output data object for this collision
	outResult.Collider = object;
	outResult.CollisionStartTime = Game::ClockTime; 
	outResult.ClosingVelocity = XMVectorGetX(impact.TotalImpactVelocity);
	outResult.ColliderPreImpactTrajectory = impact_coll.PreImpactVelocity;
	DBG_COLLISION_RESULT(concat(", t0=")(outResult.CollisionStartTime)(", closing_vel=")(outResult.ClosingVelocity)
		(", pre_impact_trajectory=")(Vector3ToString(outResult.ColliderPreImpactTrajectory).c_str()).str().c_str());

	// Retrieve properties of the colliding object
	float proj_radius = object->GetCollisionSphereRadius();									// TODO: *** Need to get actual colliding cross-section ***

	// Calculate the collider trajectory. If this is an external collider, dial the start position 
	// back along its trajectory vector to ensure there are no issues with high-speed objects 
	// skipping over the outer elements during inter-frame time.
	// All trajectory calculations are performed using pre-impact velocity, since the collision handling will have 
	// adjusted actual collider trajectory by now
	Ray proj_trajectory; 
	float ext_adj = 0.0f;
	if (external_collider)
	{
		// Move back |env_collisionradius*2| along the trajectory vector to be sure, and record the travel time this represents
		float pvel = XMVectorGetX(XMVector3LengthEst(impact_coll.PreImpactVelocity)) + 1.0f; // +1.0f to avoid any erroneous DIV/0 
		ext_adj = ((m_collisionsphereradius * 2.0f) / pvel);	// seconds
		DBG_COLLISION_RESULT(concat(", ext_adj=")(ext_adj).str().c_str());

		proj_trajectory = Ray(XMVectorSubtract(object->GetPosition(), XMVectorScale(impact_coll.PreImpactVelocity, ext_adj)), impact_coll.PreImpactVelocity);
	}
	else
	{
		// We can simply start from the current object position
		proj_trajectory = Ray(object->GetPosition(), impact_coll.PreImpactVelocity);
	}
	DBG_COLLISION_RESULT(concat(", proj_ray=")(proj_trajectory.str()).str().c_str());

	// Calcualate the path of elements intersected by this ray
	ElementIntersectionData elements;
	outResult.Intersects = DetermineElementPathIntersectedByRay(proj_trajectory, proj_radius, elements);
	DBG_COLLISION_RESULT(concat(", intersects=")(outResult.Intersects ? "true" : "false")(" (")(elements.size())(" elements)").str().c_str());

	// Quit here if there is no intersection
	if (outResult.Intersects == false)
	{
		outResult.EndCollision(EnvironmentCollision::EnvironmentCollisionState::Inactive_NoCollision);
		DBG_COLLISION_RESULT(", NO COLLISION\n");
		return false;
	}

	// Add an event for the processing of each intersected element
	ElementIntersectionData::iterator it_end = elements.end();
	for (ElementIntersectionData::iterator it = elements.begin(); it != it_end; ++it)
	{
		ElementIntersection & item = (*it);

		// Adjust all intersection times to negate the adjustment for external colliders applied above
		item.StartTime -= ext_adj; item.EndTime -= ext_adj;

		// Add an event for this collision.  This will automatically be sorted into the correct sequence
		// based upon start time.  This will almost always be ~equal to a push_back since 
		// intersections have generally been determined in temporal order
		outResult.AddElementIntersection(item.ID, item.StartTime, (item.EndTime - item.StartTime), item.Degree);

		// Also push a copy of the intersection data into the result object for reference later, in case it is needed
		outResult.IntersectionData.push_back(*it);
	}

	// Safety check: there should always be >0 events since a collision has occured, but make sure of that here
	if (outResult.Events.empty()) 
	{ 
		outResult.EndCollision(EnvironmentCollision::EnvironmentCollisionState::Inactive_NoCollision);
		DBG_COLLISION_RESULT(", unexpected zero event count, NO COLLISION\n");
		return false; 
	}

	// Perform post-processing on the event collection, finally return true to indicate that a collision occured
	outResult.Activate();
	outResult.Finalise();
	DBG_COLLISION_RESULT(concat("\n")(m_instancecode)(": Registered new collision with \"")(object->GetInstanceCode())("\" at ")
		(outResult.CollisionStartTime)("s, with ")(outResult.GetEventCount())(" collision events, and closing velocity of ")
		(outResult.ClosingVelocity)("m/s\n").str().c_str());
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
		if ((*c_it).IsActive() == false)		
		{ 
			DBG_COLLISION_RESULT(concat(m_instancecode)(": collision with \"")
				(c_it->Collider() ? c_it->Collider()->GetInstanceCode() : "<destroyed>")("\" has ended (")
				(EnvironmentCollision::GetStateDescription((*c_it).GetState()))(")\n").str().c_str());

			c_it = m_collision_events.erase(c_it); 
			c_it_end = m_collision_events.end(); 
		}
		else								
		{ 
			++c_it; 
		}
	}
}

// Processes an environment collision at the current point in time.  Determines and applies all effects since the last frame
void iSpaceObjectEnvironment::ProcessEnvironmentCollision(EnvironmentCollision & collision)
{
	// Parameter check not required; we don't need to check if IsActive() since that will be tested by the loop below

	// Loop through collision events while the object is still valid
	std::vector<EnvironmentCollision>::size_type event_count = collision.GetEventCount();
	while (collision.IsActive())
	{
		// Make sure the collider still has momentum; if not, the collision event is over
		if (collision.ClosingVelocity <= 0.0f) { collision.EndCollision(EnvironmentCollision::EnvironmentCollisionState::Inactive_Stopped); return; }

		// Get the next event in the sequence
		std::vector<EnvironmentCollision>::size_type index = collision.GetNextEvent();
		if (index >= event_count) { collision.EndCollision(EnvironmentCollision::EnvironmentCollisionState::Inactive_Completed); return; }

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
void iSpaceObjectEnvironment::ExecuteElementCollision(const EnvironmentCollision::EventDetails & ev, EnvironmentCollision & collision)
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

	// Make sure this element is not already destroyed.  If it is, simply allow the object to pass through
	if (el.IsDestroyed())
	{
		DBG_COLLISION_RESULT(concat(m_instancecode)(": \"")(object->GetInstanceCode())("\" intersected element ")
			(el.GetID())(" ")(el.GetLocation().ToString())(" which has already been destroyed\n").str().c_str());
		return;
	}

	// Get the aggregate stopping power of the element based on its properties, parent tile, and the actual contents 
	// of the element.   Will always be >= 1.0f.
	float total_strength = DetermineTotalElementImpactStrength(el);

	// Compare this total element strength to the incoming object force.  The percentage of force
	// transferred to this impacted hull is scaled by the degree of impact (Param1)
	float damage_pc = (obj_force / total_strength) * ev.Param1;

	// The final damage value (in the range 0.0-1.0) can be derived from this damage_pc and any relevant damage 
	// resistance properties of the elemnet
	float damage_actual = (damage_pc * 1.0f);		// TODO: Apply these resistances

	// Reduce the object closing force based on this impact.  The force is reduced by the hull strength, scaled by degree since
	// a 50% glancing hit against hull will bleed off around 50% of the object velocity compared to a 100% collision
	float obj_remaining_force = (obj_force - (total_strength * ev.Param1));

	// Log details of this collision if required
	DBG_COLLISION_RESULT(concat(m_instancecode)(": \"")(object->GetInstanceCode())("\" collided with element ")
		(el.GetID())(" ")(el.GetLocation().ToString())(" at ")(ev.EventTime)("s causing ")(damage_actual)(" damage (([ObjForce=")
		("[IR=")(object->GetImpactResistance())("]*[CVel=")(collision.ClosingVelocity)("]=")(obj_force)
		("] / [ElStrength=")(total_strength)("]) * [Degree=")(ev.Param1)("])")
		(". Closing velocity fell from ")(collision.ClosingVelocity)("m/s").str().c_str());
		
	// Determine remaining velocity by dividing the projectile force through by its impact_resistance, since we originally
	// derived obj_force = (obj_vel * impact_resist).  If this now takes the object velocity <= 0 it will trigger 
	// destruction of the collider in the next collision evaluation
	collision.ClosingVelocity = (obj_remaining_force / object->GetImpactResistance());
	DBG_COLLISION_RESULT(concat(" to ")(collision.ClosingVelocity)("m/s\n").str().c_str());

	// Assess the damage from this impact and apply to the element, potentially destroying it if the damage is sufficiently high
	if (EnvironmentCollisionsAreBeingSimulated())
	{
		// Test whether we are simulating an environment collision, in which case we do not apply any real damage
		SimulateElementDamage(el.GetID(), damage_actual);
	}
	else
	{
		// Apply damage to the element, as normal
		EnvironmentCollision::ElementCollisionResult result = TriggerElementDamage(el.GetID(), damage_actual);

		// If this was the first valid (not already destroyed) element to be impacted, test the outcome
		// to determine if the remainder of the collision will be a deflection or penetrating impact
		// We know the element is valid (not already destroyed) at this point since destroyed elements
		// are tested for and rejected earlier in the method
		if (collision.HasPenetratedOuterHull == false)
		{
			if (result == EnvironmentCollision::ElementCollisionResult::ElementDamaged)
			{
				// Treat this as a deflection.  Stop evaluating the collision path, and allow the projectile
				// to follow a deflection trajectory that has already been applied by the physics engine
				collision.EndCollision(EnvironmentCollision::EnvironmentCollisionState::Inactive_Deflected);

				DBG_COLLISION_RESULT(concat(m_instancecode)(": \"")(object->GetInstanceCode())("\" was deflected by element ")
					(el.GetID())(" and did not penetrate the environment\n").str().c_str());
			}
			else if (result == EnvironmentCollision::ElementCollisionResult::ElementDestroyed)
			{
				// This is a penetrating impact.  Update the projectile trajectory to pass through the 
				// environment (with no further physics engine calculations between the two objects or contents)
				collision.HasPenetratedOuterHull = true;
				object->SetWorldMomentum(collision.ColliderPreImpactTrajectory);
				object->AddCollisionExclusion(GetID());

				DBG_COLLISION_RESULT(concat(m_instancecode)(": \"")(object->GetInstanceCode())("\" penetrated the hull at element ")
					(el.GetID())(" and is intersecting the environment\n").str().c_str());
			}
		}
	}
}

// Triggers damage to an element (and potentially its contents).  Element may be destroyed if sufficiently damaged
EnvironmentCollision::ElementCollisionResult iSpaceObjectEnvironment::TriggerElementDamage(int element_id, float damage)
{
	// Get a reference to the element
	if (ElementIndexIsValid(element_id) == false) return EnvironmentCollision::ElementCollisionResult::NoImpact;
	ComplexShipElement & el = GetElementDirect(element_id);

	// Check this normalised [0.0 1.0] damage against the element health
	if (damage >= el.GetHealth())
	{
		// If the damage is sufficiently high, trigger immediate destruction of the element and quit immediately
		TriggerElementDestruction(element_id);
		return EnvironmentCollision::ElementCollisionResult::ElementDestroyed;
	}

	// Otherwise we want to apply damage to the element
	el.SetHealth(el.GetHealth() - damage);

	// TODO: We may also apply damage to the contents of the element if the damage state is significant enough
	/*if (el.GetHealth() < Game::C_ELEMENT_DAMAGE_CONTENTS_THRESHOLD)
	{
	}*/

	// Return a value to confirm that the element was damaged by this collision
	return EnvironmentCollision::ElementCollisionResult::ElementDamaged;
}

// SIMULATES damage to an element (and potentially its contents).  Element may be destroyed (in the simulation) if sufficiently damaged
void iSpaceObjectEnvironment::SimulateElementDamage(int element_id, float damage) const
{
	// Get a reference to the element
	if (ElementIndexIsValid(element_id) == false) return;
	const ComplexShipElement & el = GetConstElementDirect(element_id);

	// Check this normalised damage against the element health
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
	DBG_COLLISION_RESULT(concat(m_instancecode)(": Element ")(el.GetID())(" ")(el.GetLocation().ToString())(" was destroyed\n").str().c_str());

	// Perform a recalculation over the entire environment
	UpdateEnvironment();

	// All objects and terrain in the element are in trouble
	if (this->SpatialPartitioningTree)
	{
		EnvironmentTree *node = this->SpatialPartitioningTree->GetNodeContainingElement(el.GetLocation());
		if (node)
		{
			// Get all the objects & terrain in this tree node
			std::vector<iEnvironmentObject*> objects; 
			std::vector<Terrain*> terrain;
			node->GetAllItems(objects, terrain);

			// Destroy all objects currently in the element
			std::vector<iEnvironmentObject*>::size_type n_obj = objects.size();
			for (std::vector<iEnvironmentObject*>::size_type i_obj = 0U; i_obj < n_obj; ++i_obj)
			{
				if (objects[i_obj] && objects[i_obj]->GetElementLocation() == el_loc) objects[i_obj]->DestroyObject();
			}

			// Destroy all terrain currently in the element
			std::vector<Terrain*>::size_type n_ter = terrain.size();
			for (std::vector<Terrain*>::size_type i_ter = 0U; i_ter < n_ter; ++i_ter)
			{
				if (terrain[i_ter] && !terrain[i_ter]->IsDestroyed() &&
					terrain[i_ter]->OverlapsElement(el_loc))
				{
					terrain[i_ter]->DestroyObject();
				}
			}
		}
	}
}


// Primary method called when the object takes damage at a specified (object-local) position.  
// Calculates modified damage value (based on e.g. damage resistances) and applies to the 
// object hitpoints.  Damage is applied in the order in which is was added to the damage 
// set.  Returns true if the object was destroyed by any of the damage in this damage set
// Overrides the base iTakesDamage method to calculate per-element damage
bool iSpaceObjectEnvironment::ApplyDamage(const DamageSet & damage, const GamePhysicsEngine::OBBIntersectionData & impact)
{
	// Get the element that should receive this damage.  We offset the incoming location by
	// environment extent to ensure that [0,0,0] is at bottom-bottom-left (rather than centre)
	// We also get the *closest" element to this position, since we know it is a valid
	// collision, but there is a chance of edge cases at OBB/element boundaries where we may otherwise
	// select an adjacent & destroyed element location
	/*XMVECTOR envpos = XMVectorAdd(location, CollisionOBB.ConstData().ExtentV);
	INTVECTOR3 eloc = GetNearestActiveElementToPosition(envpos);
	OutputDebugString(concat("Pos: ")(Vector3ToString(envpos))(" = El: ")(eloc.ToString())("\n").str().c_str());*/
	if (!impact.OBB) return false;
	INTVECTOR3 eloc = DetermineElementAtOBBLocation(*(impact.OBB), impact.CollisionPointOBBLocal);
	
	int index = GetElementIndex(eloc);
	ComplexShipElement & el = m_elements[index];

	// Iterate over each type of damage being inflicted
	DamageSet::const_iterator it_end = damage.end();
	for (DamageSet::const_iterator it = damage.begin(); it != it_end; ++it)
	{
		// Apply this damage; if the damage is enough to destroy the element we can 
		// terminate the method immediately
		if (ApplyDamageComponentToElement(el, (*it)) == 
			EnvironmentCollision::ElementCollisionResult::ElementDestroyed) return true;
	}

	// This damage was not sufficient to destroy the element
	return false;
}

// Applies a damage component to the specified element.  Returns true if the damage was sufficient
// to destroy the element
EnvironmentCollision::ElementCollisionResult iSpaceObjectEnvironment::ApplyDamageComponentToElement(
	ComplexShipElement & el, Damage damage)
{
	// Make sure this element has not already been destroyed
	if (el.IsDestroyed()) return EnvironmentCollision::ElementCollisionResult::NoImpact;

	// If a tile exists at this element location, apply any damage resistance that it may provide
	ComplexShipTile *tile = el.GetTile();
	if (tile)
	{
		if (tile->IsInvulnerable()) return EnvironmentCollision::ElementCollisionResult::NoImpact;
		if (tile->HasDamageResistance()) damage.ApplyDamageResistance(tile->GetDamageResistance());
	}

	// Apply any remaining damage to this element
	return TriggerElementDamage(el.GetID(), damage.Amount);
}

// Set the destruction state of the specified terrain object
void iSpaceObjectEnvironment::SetTerrainDestructionState(Game::ID_TYPE id, bool is_destroyed)
{
	TerrainCollection::iterator it = std::find_if(TerrainObjects.begin(), TerrainObjects.end(),
		[id](const Terrain* obj) { return (obj->GetID() == id); });

	if (it != TerrainObjects.end()) (*it)->SetObjectDestroyedState(is_destroyed);
}

// Set the destruction state of all terrain objects from the specified tile
void iSpaceObjectEnvironment::SetTileTerrainDestructionState(Game::ID_TYPE tile_id, bool is_destroyed)
{
	std::for_each(TerrainObjects.begin(), TerrainObjects.end(), [tile_id, is_destroyed](Terrain *terrain) 
	{ 
		if (terrain && terrain->GetParentTileID() == tile_id) terrain->SetObjectDestroyedState(is_destroyed); 
	});
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
	std::vector<Terrain*>::const_iterator it_end = source->TerrainObjects.end();
	for (std::vector<Terrain*>::const_iterator it = source->TerrainObjects.begin(); it != it_end; ++it)
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
		el = new (std::nothrow)ComplexShipElement[ecount];
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

			// Loop through all elements in the source collection
			for (int i = 0; i < scount; ++i)
			{
				// See whether this element will exist in our new element space
				const ComplexShipElement & src = source[i];
				const INTVECTOR3 & sloc = src.GetLocation();
				if (IntVector3Between(sloc, NULL_INTVECTOR3, ubound))
				{
					// Copy the source element into the new space
					el[ELEMENT_INDEX_EX(sloc.x, sloc.y, sloc.z, size)] = src;
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
	m_elementbounds = (m_elementsize - ONE_INTVECTOR3);
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
	std::vector<Terrain*>::iterator it_end = TerrainObjects.end();
	for (std::vector<Terrain*>::iterator it = TerrainObjects.begin(); it != it_end; ++it)
	{
		SpatialPartitioningTree->AddItem<Terrain*>((*it));
	}

	// Also add all objects to the tree
	std::vector<ObjectReference<iEnvironmentObject>>::iterator it2_end = Objects.end();
	for (std::vector<ObjectReference<iEnvironmentObject>>::iterator it2 = Objects.begin(); it2 != it2_end; ++it2)
	{
		SpatialPartitioningTree->AddItem<iEnvironmentObject*>((*it2)());
	}
}

// Determines the set of connections from other tiles that surround this element
void iSpaceObjectEnvironment::GetNeighbouringTiles(ComplexShipTile *tile, std::vector<TileAdjacency> & outNeighbours)
{
	// Parameter check
	if (!tile) return;
	const INTVECTOR3 & location = tile->GetElementLocation();
	const INTVECTOR3 & size = tile->GetElementSize();

	// Check each direction in turn
	static const Direction directions[4] = { Direction::Left, Direction::Up, Direction::Right, Direction::Down };
	for (int i = 0; i < 4; ++i)
	{
		Direction dir = directions[i];

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

					// Test whether this element contains a tile; if so, and assuming it is not another 
					// part of the source tile, we want to record that fact
					adj_tile = adj_el.GetTile();
					if (adj_tile && adj_tile != tile)
					{
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

	// Find any tiles neighbouring this one 
	std::vector<TileAdjacency> adjacent;
	GetNeighbouringTiles(tile, adjacent);

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
			if (tile->PossibleConnections.ConnectionExists(type, adj.Location, dirBS) &&		// Tile > Adj
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
	// Deallocate the element space if it exists
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
	ComplexShipElement *elements = new (std::nothrow)ComplexShipElement[m_elementcount];
	if (!elements) return ErrorCodes::CannotAllocateElementSpaceForRotation;

	// Determine new element space dimensions
	INTVECTOR3 newsize = ((rotation == Rotation90Degree::Rotate90 || rotation == Rotation90Degree::Rotate270) ?
		INTVECTOR3(m_elementsize.y, m_elementsize.x, m_elementsize.z) : m_elementsize);

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
			index = ELEMENT_INDEX_EX(loc.y, (m_elementsize.x - 1) - loc.x, loc.z, newsize);							break;
		case Rotation90Degree::Rotate180:
			index = ELEMENT_INDEX_EX((m_elementsize.x - 1) - loc.x, (m_elementsize.y - 1) - loc.y, loc.z, newsize);	break;
		case Rotation90Degree::Rotate270:
			index = ELEMENT_INDEX_EX((m_elementsize.y - 1) - loc.y, loc.x, loc.z, newsize);							break;
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


// Determines the contiguous range of elements between the specified two elements
INTVECTOR2 iSpaceObjectEnvironment::GetElementRange(const INTVECTOR3 & el1, const INTVECTOR3 & el2)
{
	// Get the index of each element
	int i1 = ELEMENT_INDEX(el1.x, el1.y, el1.z); 
	int i2 = ELEMENT_INDEX(el2.x, el2.y, el2.z);

	// Make sure both indices are valid
	if (i1 < 0 || i2 < 0 || i1 >= m_elementcount || i2 >= m_elementcount) return NULL_INTVECTOR2;

	// Return the range of elements between these two points (ignoring the relative order of el1 & el2)
	return (i2 < i1 ? INTVECTOR2(i2, i1) : INTVECTOR2(i1, i2));
}

// Determines the contiguous range of elements on the specified z-level of the environment
INTVECTOR2 iSpaceObjectEnvironment::GetElementRange(int zlevel)
{
	return GetElementRange(INTVECTOR3(0, 0, zlevel), INTVECTOR3(m_elementsize.x - 1, m_elementsize.y - 1, zlevel));
}

// Returns the index of the tile at the specified index, or -1 if no tile is present
// No validation on element_id; relies on this being passed as a valid index
int iSpaceObjectEnvironment::GetTileAtElement(int element_id) const
{
	const ComplexShipTile *tile = m_elements[element_id].GetTile();
	return (tile ? tile->GetID() : -1);
}

// Returns the index of the tile at the specified location, or -1 if no tile is present
// No validation on location; relies on this being passed as a valid element location
int iSpaceObjectEnvironment::GetTileAtElementLocation(const INTVECTOR3 & location) const
{
	const ComplexShipTile *tile = m_elements[ELEMENT_INDEX(location.x, location.y, location.z)].GetTile();
	return (tile ? tile->GetID() : -1);
}

// Determines the set of visible terrain objects, based upon a calculation of visible terrain objects during rendering that 
// if recalculated each frame that the environment is rendered.  Visibility results are available for a short validity period 
// before being expired.  Object pointers should be considered valid for the current frame only
void iSpaceObjectEnvironment::DetermineVisibleTerrain(std::vector<Terrain*> & outTerrain)
{
	// Check every terrain object in turn
	for (Terrain *terrain : TerrainObjects)
	{
		// If the object was rendered during this or the previous frame we consider it visible and return it
		if (terrain->WasRenderedSincePriorFrame()) outTerrain.push_back(terrain);
	}
}

// Renders a 3D overlay showing the properties of each element in the environment
void iSpaceObjectEnvironment::DebugRenderElementState(void)
{
	return DebugRenderElementState(std::unordered_map<bitstring, BasicColourDefinition>());
}

// Renders a 3D overlay showing the properties of each element in the environment
void iSpaceObjectEnvironment::DebugRenderElementState(std::unordered_map<bitstring, BasicColourDefinition> & outLegend)
{
	return DebugRenderElementState(0, m_elementcount - 1, outLegend);
}

// Renders a 3D overlay showing the properties of each element in the environment
void iSpaceObjectEnvironment::DebugRenderElementState(int z_index)
{
	return DebugRenderElementState(z_index, std::unordered_map<bitstring, BasicColourDefinition>());
}

// Renders a 3D overlay showing the properties of each element in the environment
void iSpaceObjectEnvironment::DebugRenderElementState(int z_index, std::unordered_map<bitstring, BasicColourDefinition> & outLegend)
{
	INTVECTOR2 range = GetElementRange(z_index);
	return DebugRenderElementState(range.x, range.y, outLegend);
}

// Renders a 3D overlay showing the properties of each element in the environment
void iSpaceObjectEnvironment::DebugRenderElementState(int start, int end)
{
	return DebugRenderElementState(start, end, std::unordered_map<bitstring, BasicColourDefinition>());
}

// Renders a 3D overlay showing the properties of each element in the environment
void iSpaceObjectEnvironment::DebugRenderElementState(int start, int end, std::unordered_map<bitstring, BasicColourDefinition> & outLegend)
{
	// Parameter check; this must be a contiguous range within the set of environment elements
	if (start > end) std::swap(start, end);
	if (start < 0 || end >= m_elementcount) return;
	unsigned int count = (unsigned int)(end - start + 1);

	// Allocate an array for the rendering data.  We only need to calculate values for the specified range
	std::vector<XMFLOAT4>::size_type data_index = 0U;
	std::vector<XMFLOAT4> data(count);
	for (int i = start; i <= end; ++i, ++data_index)
	{
		bitstring x = m_elements[i].GetProperties();
		std::unordered_map<bitstring, BasicColourDefinition>::const_iterator it = outLegend.find(x);
		if (it == outLegend.end())
		{
			// We do not yet have a colour for this property combination, so add one now
			outLegend[x] = (outLegend.size() < CoreEngine::BASIC_COLOURS.size() ?
				CoreEngine::BASIC_COLOURS.at(outLegend.size()) :
				BasicColourDefinition(XMFLOAT4(frand(), frand(), frand(), 1.0f), "(Other)"));
			it = outLegend.find(x);
		}

		assert(it != outLegend.end());
		data[data_index] = it->second.colour;
	}

	// Render this overlay on the environment
	Game::Engine->GetOverlayRenderer()->RenderEnvironment3DOverlay(*this, data.begin(), data.end(), start);

}

// Internal method; get all objects within a given distance of the specified position, within the 
// specified EnvironmentTree node
void iSpaceObjectEnvironment::_GetAllObjectsWithinDistance(EnvironmentTree *tree_node, const FXMVECTOR position, float distance,
	std::vector<iEnvironmentObject*> *outObjects, std::vector<Terrain*> *outTerrain)
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
				Terrain *obj;
				std::vector<Terrain*>::const_iterator it_end = node->GetNodeTerrain().end();
				for (std::vector<Terrain*>::const_iterator it = node->GetNodeTerrain().begin(); it != it_end; ++it)
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
	std::vector<iEnvironmentObject*> *outObjects, std::vector<Terrain*> *outTerrain)
{
	if (focal_object) _GetAllObjectsWithinDistance(focal_object->GetEnvironmentTreeNode(),
		focal_object->GetEnvironmentPosition(), distance, outObjects, outTerrain);
}

// Find all objects within a given distance of the specified object.  Object & Terrain output
// vectors will be populated if valid pointers are supplied
void iSpaceObjectEnvironment::GetAllObjectsWithinDistance(Terrain *focal_object, float distance,
	std::vector<iEnvironmentObject*> *outObjects, std::vector<Terrain*> *outTerrain)
{
	if (focal_object) _GetAllObjectsWithinDistance(focal_object->GetEnvironmentTreeNode(),
		focal_object->GetEnvironmentPosition(), distance, outObjects, outTerrain);
}

// Find all objects within a given distance of the specified location.  Object & Terrain output
// vectors will be populated if valid pointers are supplied.  Less efficient than the method
// which supplies a focal object, since the relevant node has to be determined based on the position
void iSpaceObjectEnvironment::GetAllObjectsWithinDistance(EnvironmentTree *spatial_tree, const FXMVECTOR position, float distance,
	std::vector<iEnvironmentObject*> *outObjects, std::vector<Terrain*> *outTerrain)
{
	if (spatial_tree)
	{
		// Determine the relevant tree node and pass to the primary search function
		_GetAllObjectsWithinDistance(spatial_tree->GetNodeContainingPoint(position), position, distance, outObjects, outTerrain);
	}
}

// Finds all visible objects within a given distance of the specified location, with visibility determined
// by the given frustum object
void iSpaceObjectEnvironment::GetAllVisibleObjectsWithinDistance(EnvironmentTree *spatial_tree, const FXMVECTOR position, const FXMVECTOR search_radius, const Frustum *frustum,
																 std::vector<iEnvironmentObject*> *outObjects, std::vector<Terrain*> *outTerrain)
{
	// Parameter check
	if (!spatial_tree || !frustum) return;

	// Determine the relevant tree node that covers the search area
	XMVECTOR pos_min = XMVectorMax(NULL_VECTOR, XMVectorSubtract(position, search_radius));
	XMVECTOR pos_max = XMVectorMin(m_size, XMVectorAdd(position, search_radius));
	XMVECTOR threshold_dist;

	EnvironmentTree *node = spatial_tree->GetNodeContainingPoint(position);
	while (!node->ContainsAreaFullyInclusive(pos_min, pos_max) && node->GetParent())
	{
		node = node->GetParent();
	}

	// The node now contains the entire search area; push it into the search vector and begin searching
	m_search_nodes.clear();
	m_search_nodes.push_back(node);

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
				if (child->IntersectsAreaFullyInclusive(pos_min, pos_max)) m_search_nodes.push_back(child);
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
					// Object must exist and be within the search radius
					obj = (*it); if (!obj) continue;
					threshold_dist = XMVectorAdd(search_radius, XMVectorReplicate(obj->GetCollisionSphereRadius()));
					if (XMVector2Greater(XMVector3LengthSq(XMVectorSubtract(obj->GetEnvironmentPosition(), position)),
										 XMVectorMultiply(threshold_dist, threshold_dist))) continue;

					// If the object also lies within our view frustum then return it
					if (frustum->CheckSphere(obj->GetEnvironmentPosition(), obj->GetCollisionSphereRadius()))
						outObjects->push_back(obj);
				}
			}

			// Also process any terrain objects
			if (outTerrain)
			{
				Terrain *obj;
				std::vector<Terrain*>::const_iterator it_end = node->GetNodeTerrain().end();
				for (std::vector<Terrain*>::const_iterator it = node->GetNodeTerrain().begin(); it != it_end; ++it)
				{
					// Terrain object must exist and be within the search radius
					obj = (*it); if (!obj || !obj->GetDefinition() || !obj->GetDefinition()->HasModel()) continue;
					threshold_dist = XMVectorAdd(search_radius, XMVectorReplicate(obj->GetCollisionRadius()));
					if (XMVector2Greater(XMVector3LengthSq(XMVectorSubtract(obj->GetEnvironmentPosition(), position)),
						XMVectorMultiply(threshold_dist, threshold_dist))) continue;

					// If the terrain object also lies within our view frustum then return it
					if (frustum->CheckSphere(XMVector3TransformCoord(obj->GetEnvironmentPosition(), m_zeropointworldmatrix), obj->GetCollisionRadius()))
						outTerrain->push_back(obj);
				}
			}
		}
	}
}

// Default destructor
iSpaceObjectEnvironment::~iSpaceObjectEnvironment(void)
{

}

// Custom debug string function
std::string	iSpaceObjectEnvironment::DebugString(void) const
{
	return iObject::DebugString(DebugEnvironmentString());
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
	REGISTER_DEBUG_ACCESSOR_FN(GetOxygenLevel, command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(GetOxygenLevelAtLocation, INTVECTOR3(command.ParameterAsInt(2), command.ParameterAsInt(3), command.ParameterAsInt(4)))
	REGISTER_DEBUG_ACCESSOR_FN(GetTotalOxygenInEnvironment)
	REGISTER_DEBUG_ACCESSOR_FN(GetPowerLevel, command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(GetPowerLevelAtLocation, INTVECTOR3(command.ParameterAsInt(2), command.ParameterAsInt(3), command.ParameterAsInt(4)))
	REGISTER_DEBUG_ACCESSOR_FN(GetTileAtElement, command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(GetTileAtElementLocation, INTVECTOR3(command.ParameterAsInt(2), command.ParameterAsInt(3), command.ParameterAsInt(4)))
	REGISTER_DEBUG_ACCESSOR_FN(DetermineTotalElementImpactStrength, command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(DetermineTotalElementImpactStrengthAtLocation, INTVECTOR3(command.ParameterAsInt(2), command.ParameterAsInt(3), command.ParameterAsInt(4)))
	REGISTER_DEBUG_ACCESSOR_FN(SupportsPortalBasedRendering)

	// Mutator methods
	REGISTER_DEBUG_FN(BuildSpatialPartitioningTree)
	REGISTER_DEBUG_FN(SimulateObject)
	REGISTER_DEBUG_FN(PerformPostSimulationUpdate)
	REGISTER_DEBUG_FN(SetSimulationStateOfEnvironmentContents, (iObject::ObjectSimulationState)command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(UpdateGravity)
	REGISTER_DEBUG_FN(UpdateOxygen)
	REGISTER_DEBUG_FN(RefreshPositionImmediate)
	REGISTER_DEBUG_FN(UpdateEnvironment)
	REGISTER_DEBUG_FN(SuspendEnvironmentUpdates)
	REGISTER_DEBUG_FN(ResumeEnvironmentUpdates)
	REGISTER_DEBUG_FN(NotifyIsContainerOfSimulationHubs, command.ParameterAsBool(2))
	REGISTER_DEBUG_FN(ClearAllTerrainObjects)
	REGISTER_DEBUG_FN(ShutdownNavNetwork)
	REGISTER_DEBUG_FN(UpdateNavigationNetwork)
	REGISTER_DEBUG_FN(UpdateElementSpaceStructure)
	REGISTER_DEBUG_FN(TriggerElementDamage, command.ParameterAsInt(2), command.ParameterAsFloat(3))
	REGISTER_DEBUG_FN(TriggerElementDestruction, command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(BuildAllEnvironmentMaps)
	REGISTER_DEBUG_FN(RevalidateEnvironmentMaps)
	REGISTER_DEBUG_FN(OverrideLocalGravity, command.ParameterAsFloat(2))
	REGISTER_DEBUG_FN(RemoveLocalGravityOverride)
	REGISTER_DEBUG_FN(DeterminePortalRenderingSupport)
	REGISTER_DEBUG_FN(OverridePortalBasedRenderingSupport, command.ParameterAsBool(2))
	REGISTER_DEBUG_FN(RemoveOverrideOfPortalBasedRenderingSupport)

	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		iContainsComplexShipTiles::ProcessDebugCommand(command);
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		Ship::ProcessDebugCommand(command);
}



