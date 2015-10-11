#include <string>
#include <vector>
#include "DX11_Core.h"

#include "Utility.h"
#include "FastMath.h"
#include "ErrorCodes.h"
#include "Octree.h"
#include "Model.h"
#include "Ship.h"
#include "Hardpoints.h"
#include "ComplexShipSection.h"
#include "CapitalShipPerimeterBeacon.h"
#include "ComplexShipTile.h"
#include "HpEngine.h"
#include "Engine.h"
#include "SpaceSystem.h"
#include "NavNetwork.h"
#include "GameSpatialPartitioningTrees.h"
#include "CSLifeSupportTile.h"
#include "CopyObject.h"
#include "iSpaceObjectEnvironment.h"

#include "ComplexShip.h"


// Default constructor for complex ship objects
ComplexShip::ComplexShip(void)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::ComplexShipObject);

	// Set the class of ship 
	m_shipclass = Ships::Class::Complex;

	// Set ship properties to default values on object creation
	m_navnetwork = NULL;
	m_perimeterbeacons.clear();
	m_activeperimeternodes.clear();
	m_activebeacons = 0;
	m_forcerenderinterior = false;
	m_suspendupdates = false;
	m_gravityupdaterequired = true;
	m_oxygenupdaterequired = true;
	m_sdoffset = INTVECTOR3(0, 0, 0);
	m_directlygeneratedfromSD = false;
}


// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void ComplexShip::InitialiseCopiedObject(ComplexShip *source)
{
	// Pass control to all base classes
	iSpaceObjectEnvironment::InitialiseCopiedObject((iSpaceObjectEnvironment*)source);

	/* Now perform ComplexShip-specific initialisation logic for new objects */

	// Clear the ship hardpoints collection so that it is rebuilt as new sections are added
	GetHardpoints().SuspendUpdates();
	GetHardpoints().ClearData();

	// Clear the set of ship sections copied from the source, and re-add those ship sections one-by-one
	// This will also bring across hardpoints etc. and create references within the ship
	this->GetSections()->clear();
	ComplexShip::ComplexShipSectionCollection::const_iterator s_it_end = source->GetSections()->end();
	for (ComplexShip::ComplexShipSectionCollection::const_iterator s_it = source->GetSections()->begin(); s_it != s_it_end; ++s_it)
	{
		// Create a copy of the ship section, using this section as a template
		ComplexShipSection *section = ComplexShipSection::Create(*s_it);

		// Assuming the section was copied successfully, add it to this ship
		if (section) this->AddShipSection(section);
	}

	// Resume updates once data has been pulled from each section
	GetHardpoints().ResumeUpdates();

	// Run complex ship-specific method to remove 'standard' flags, since it also needs to propogate to all sections as well
	this->RemoveStandardComponentFlags();

	// Recalculate ship size based on the sections above, before we start placing tiles & terrain etc
	this->CalculateShipSizeData();

	// Remove all references to ship tiles and generate copies for the new ship
	this->RemoveAllShipTiles();
	this->CopyTileDataFromObject((iContainsComplexShipTiles*)source);

	// Copy all terrain objects from the source to the target ship.  First, do a direct removal from the target ship to remove references carried
	// over in the copy constructor.  We need to replicate and re-add so that new objects are created, and so per-element pointers are added
	//cs->TerrainObjects.clear();
	//cs->CopyTerrainDataFromObject(template_ship);

	// Remove the nav network pointer in this ship, since we want to generate a new one for the ship when first required
	this->RemoveNavNetworkLink();

	// Avoid replicating any behaviour from the source ship in the target
	this->CancelAllOrders();
	this->SetTargetThrustOfAllEngines(0.0f);

	// Run the light recalculation method to regenerate ship statistics based on its component sections, tiles, equipment etc
	this->RecalculateShipDataFromCurrentState();

	// Perform an initial derivation of the world/zero point matrices, as a starting point
	RefreshPositionImmediate();
}


ComplexShipSection *ComplexShip::GetSection(ComplexShip::ComplexShipSectionCollection::size_type index)
{
	// Perform bounds checking before we attempt to return the ship section
	if (index >= m_sections.size()) return NULL;

	// Return the section at the specified index
	return m_sections[index];
}


// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
void ComplexShip::SimulationStateChanged(iObject::ObjectSimulationState prevstate, iObject::ObjectSimulationState newstate)
{
	// Call the superclass event before proceeding
	Ship::SimulationStateChanged(prevstate, newstate);

	// If we were not being simulated, and we now are, then we may need to take some ComplexShip-specific actions here
	// TODO: this will not always be true in future when we have more granular simulation states 
	if (prevstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// Perform a full recalculation, which will generate new perimeter beacons, a nav network etc. for use during active simulation
		RecalculateAllShipData();
	}

	// Conversely, if we are no longer going to be simulated, we can remove the perimeter beacons / nav network etc. 
	if (newstate == iObject::ObjectSimulationState::NoSimulation)
	{
		ShutdownPerimeterBeacons();
		ShutdownNavNetwork();
	}

	// We also want to pass this state down to each complex ship section, since the sections are branching off the iObject>this inheritance hierarchy
	ComplexShipSectionCollection::iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::iterator it = m_sections.begin(); it != it_end; ++it)
		(*it)->SetSimulationState(newstate);
}

Result ComplexShip::AddShipSection(ComplexShipSection *section)
{
	// Validate and then add this section to the collection
	if (!section) return ErrorCodes::CannotAddInvalidShipSectionToShip;
	m_sections.push_back(section);

	// Set the pointer from the section details back to its parent ship object
	section->SetParent(this);

	// Update the ship based on the addition of this section, unless updates are currently suspended
	if (!m_suspendupdates)
	{
		// Perform a full rebuild of anything dependent on sections, e.g. hardpoints
		BuildHardpointCollection();

		// Perform an update of ship statistics based on the current configuration
		RecalculateAllShipData();
	}
		
	// Return success now that the section is successfully integrated into the ship
	return ErrorCodes::NoError;
}

void ComplexShip::RemoveShipSection(ComplexShipSection *section)
{
	// Sanity check; we need a valid section in order to remove it
	if (!section) return;
	
	// Loop through the collection and attempt to locate this item
	std::vector<ComplexShip::ComplexShipSectionCollection>::size_type n = m_sections.size();
	for (std::vector<ComplexShip::ComplexShipSectionCollection>::size_type i = 0; i < n; ++i)
	{
		if (m_sections.at(i) == section)
		{
			// Clear the pointer from this section back to its parent ship object
			section->SetParent(NULL);

			// Remove this section from the collection
			m_sections.erase(m_sections.begin() + i);

			// Update the ship based on the removal of this section, unless updates are currently suspended
			if (!m_suspendupdates)
			{
				// Perform a full rebuild of anything dependent on sections, e.g. hardpoints
				BuildHardpointCollection();

				// Perform an update of ship statistics based on the current configuration
				RecalculateAllShipData();
			}

			// End the method here if we have removed the specified item
			return;
		}
	}
}

void ComplexShip::RemoveShipSection(std::vector<ComplexShip::ComplexShipSectionCollection>::size_type index)
{
	// Make sure this is a valid section
	if (index >= m_sections.size() || m_sections.at(index) == NULL) return;

	// Clear the pointer from this section back to its parent ship object
	m_sections.at(index)->SetParent(NULL);

	// Remove this section from the collection
	m_sections.erase(m_sections.begin() + index);

	// Update the ship based on the removal of this section, unless updates are currently suspended
	if (!m_suspendupdates)
	{
		// Perform a full rebuild of anything dependent on sections, e.g. hardpoints
		BuildHardpointCollection();

		// Perform an update of ship statistics based on the current configuration
		RecalculateAllShipData();
	}
}

// Method to handle the addition of a ship tile to this object
void ComplexShip::ShipTileAdded(ComplexShipTile *tile)
{
	// (Implement any complex ship-specific logic here)

	// Pass control down to base classes
	iSpaceObjectEnvironment::ShipTileAdded(tile);
}

// Method to handle the removal of a ship tile from this object
void ComplexShip::ShipTileRemoved(ComplexShipTile *tile)
{
	// (Implement any complex ship-specific logic here)

	// Pass control down to base classes
	iSpaceObjectEnvironment::ShipTileRemoved(tile);
}

// Builds the complex ship hardpoint collection based on its constituent ship sections
void ComplexShip::BuildHardpointCollection(void)
{
	// Suspend hardpoint updates and then clear the whole collection
	m_hardpoints.SuspendUpdates();
	m_hardpoints.ClearData();

	// We will pull hardpoints from each ship section in turn
	ComplexShip::ComplexShipSectionCollection::iterator it_end = m_sections.end();
	for (ComplexShip::ComplexShipSectionCollection::iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Get a reference to the section hardpoint collection
		const std::vector<Hardpoint*> & h = (*it)->GetHardpoints();
		std::vector<Hardpoint*>::size_type n = h.size();

		// Add a reference to each hardpoint (i.e. NOT a clone) to the ship in turn
		for (std::vector<Hardpoint*>::size_type i = 0; i < n; ++i) if (h[i]) m_hardpoints.AddHardpoint(h[i]);
	}

	// Resume hardpoint updates, which will trigger the event that updates the ship based on all hardpoints
	m_hardpoints.ResumeUpdates();
}

// Method triggered when the layout (e.g. active/walkable state, connectivity) of elements is changed
void ComplexShip::ElementLayoutChanged(void)
{
	// We want to update the ship navigation network if the element layout has changed.  
	UpdateNavigationNetwork();
}

// Overrides the virtual iSpaceObject method to ensure that all ship sections are also moved into 
// the environment along with the 'ship' itself
void ComplexShip::MoveIntoSpaceEnvironment(SpaceSystem *system, const D3DXVECTOR3 & location)
{
	// Move the ship itself into the environment by calling the base iSpaceObject method
	iSpaceObject::MoveIntoSpaceEnvironment(system, location);

	// Perform an initial derivation of the world/zero point matrices, as a starting point, since these 
	// will otherwise only be recalculated once the ship moves for the first time in its new environment
	RefreshPositionImmediate();

	// Now move each ship section into the environment in turn
	std::vector<ComplexShip::ComplexShipSectionCollection>::size_type n = m_sections.size();
	for (std::vector<ComplexShip::ComplexShipSectionCollection>::size_type i = 0; i < n; ++i)
	{
		if (m_sections[i])
		{
			// Move into the environment
			m_sections[i]->MoveIntoSpaceEnvironment(system, location);

			// Have the section recalculate its own position based on the parent ship location
			m_sections[i]->UpdatePositionFromParent();
		}
	}
}

// Force rendering state of a ship interior even when it does not otherwise meet any criteria for doing so
void ComplexShip::ForceRenderingOfInterior(bool render)
{
	// Store the new render state
	m_forcerenderinterior = render;

	// Pass state to all ship sections
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Pass this state down to the ship section
		if ((*it)) (*it)->ForceRenderingOfInterior(render);
	}	
}

// Default shutdown method; calls the overloaded function with default shutdown options
void ComplexShip::Shutdown(void)
{
	// By default we DO NOT want to shut down standard ship/section definitions when removing a ship.  We also do not want to
	// shutdown individual sections or perimeter beacons since these are space objects in their own right, so when deallocating centrally 
	// we will actually delete those objects when they are reached in the space object register
	Shutdown(false, false, false, false);
}

/* 
	Terminates the ship object and all associated storage.  
	    IncludeStandardShips    -  flag determining whether we shut down any object marked as "standard", or otherwise leave it untouched
		ShutdownSections		 - flag determining whether sections are deallocated.  Should be true unless doing a central shutdown, in which case
								   sections will already be deallocated via the central space object register
		UnlinkTiles				 - flag determining whether tile links should be broken.  Tiles will still be deallocated.  Should be set to
								   true for all normal/ad-hoc shutdowns.  Set to false for (default) central shutdown since in that case
								   a parent object (e.g. ship) could be deallocated centrally before the unlinking is performed (e.g. by the section)
		ShutdownBeacons			 - flag determining whether beacons should be deallocated.  Only set when we are deleting ships on an ad-hoc
								   basis; the default method will set this to false since perimeter beacons are also space objects that will
								   be deallocated separately by the central method.
*/
void ComplexShip::Shutdown(bool IncludeStandardObjects, bool ShutdownSections, bool UnlinkTiles, bool ShutdownBeacons)
{
	// Shutdown the interior environment of the ship (including tiles, elements) by calling the superclass shutdown method
	iSpaceObjectEnvironment::Shutdown(UnlinkTiles);

	// Detach and deallocate the navigation network assigned to this ship
	ShutdownNavNetwork();

	// Clear the hardpoints collection, but don't deallocate the hardpoints.  These are the responsibility of the ship 
	// section that owns them
	m_hardpoints.ClearData();

	// Shut down the collection of capital ship perimeter beacons, if required
	if (ShutdownBeacons) ShutdownPerimeterBeacons();

	// Also shut down each ship section in turn, if required
	if (ShutdownSections)
	{
		int sectioncount = (int)m_sections.size();
		for (int i = 0; i < sectioncount; ++i)
		{
			// Make sure the section is valid
			if (m_sections[i])
			{
				// We don't want to shut down standard objects unless the relevant flag is set
				if (!m_sections[i]->IsStandardObject() || IncludeStandardObjects)
				{
					m_sections[i]->Shutdown();
					SafeDelete(m_sections[i]);
				}
			}
		}
	}

	// Clear the ship section collection now that its contents have been deallocated
	m_sections.clear();

	// Pass control back to the primary base class for further shutdown activities
	Ship::Shutdown();
}

// Deallocates and clears the set of capital ship perimeter beacons attached to this ship
void ComplexShip::ShutdownPerimeterBeacons(void)
{
	// Iterate over any perimeter beacons we have attached to this ship
	CapitalShipPerimeterBeacon *beacon;
	PerimeterBeaconCollection::iterator it_end = m_perimeterbeacons.end();
	for (PerimeterBeaconCollection::iterator it = m_perimeterbeacons.begin(); it != it_end; ++it)
	{
		// Make sure this item is valid
		beacon = (*it);
		if (!beacon) continue;

		// Shut down the beacon (which will detach it from this ship) and then deallocate it
		beacon->Shutdown();
		delete beacon; beacon = NULL;
	}

	// Finally clear the collection of (now invalid) pointers to perimeter beacons
	m_perimeterbeacons.clear();
}

// Recalculates the ship position.  Extends on the method of the base Ship class
void ComplexShip::SimulateObject(void)
{
	// First call the base class method to recalculate position of the ship itself
	Ship::SimulateObject();

	// Also call the base environment class method to update the interior of the ship
	iSpaceObjectEnvironment::SimulateObject();

	// Also recalculate the world position & world matrix of each section in this ship
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Make sure this section is active & valid
		ComplexShipSection *sec = (*it);
		if (!sec) continue;

		// Take this opportunity to check whether the section has been updated.  If it has, we should pull those changes into the ship object now
		if (sec->SectionIsUpdated()) RecalculateShipDataFromCurrentState();

		// Recalculate the section position & orientation based on that of the overall ship, IFF the ship has moved since the last frame
		if (SpatialDataChanged())
		{
			sec->UpdatePositionFromParent();
		}
	}

	// Update properties of the ship if required
	if (m_gravityupdaterequired)		PerformShipGravityUpdate();
	if (m_oxygenupdaterequired)			PerformShipOxygenUpdate();

	// Make sure that the spatial tree node for this ship contains the WHOLE ship
	if (m_treenode)
	{
		if (m_position.x - m_size.x >= m_treenode->m_xmin && m_position.x + m_size.x < m_treenode->m_xmax &&
			m_position.y - m_size.y >= m_treenode->m_ymin && m_position.y + m_size.y < m_treenode->m_ymax &&
			m_position.z - m_size.z >= m_treenode->m_zmin && m_position.z + m_size.z < m_treenode->m_zmax)
		{
			// It does, so just make sure that we have no perimeter beacons active
			if (m_activebeacons != 0) DeactivatePerimeterBeacons();
		}
		else
		{
			// It does not, so we may need to spawn perimeter beacons
			UpdatePerimeterBeacons();
		}
	}
}

// Performs an update of ship gravity levels, based on each life support system in the ship
void ComplexShip::PerformShipGravityUpdate(void)
{
	CSLifeSupportTile *tile;
	INTVECTOR3 elmin, elmax;
	int maxrange;
	float gravity;

	// Reset the update flag now we are performing an update
	m_gravityupdaterequired = false;

	// First, reset the gravity strength at every element to zero
	for (int x = 0; x < m_elementsize.x; ++x) {
		for (int y = 0; y < m_elementsize.y; ++y) {
			for (int z = 0; z < m_elementsize.z; ++z) {
				m_elements[x][y][z].ChangeGravityStrength(0.0f);
			}
		}
	}

	// Now process each life support system in turn
	std::vector<ComplexShipTile*>::iterator it_end = GetTilesOfType(D::TileClass::LifeSupport).end();
	for (std::vector<ComplexShipTile*>::iterator it = GetTilesOfType(D::TileClass::LifeSupport).begin(); it != it_end; ++it)
	{
		// Make sure this is a valid life support system
		tile = (CSLifeSupportTile*)(*it);
		if (!tile) continue;

		// Determine the maximum effective tile range of this system.  E.g. Range=3 --> MaxRange=ceil(3*1.41...) = ceil(4.24) = 5
		maxrange = (int)ceilf(tile->GetGravityRange() * ROOT2);

		// Now determine the range of elements that need to be considered for the system
		elmin = INTVECTOR3(	max(0, tile->GetElementLocationX() - maxrange), max(0, tile->GetElementLocationY() - maxrange),
							max(0, tile->GetElementLocationZ() - maxrange));
		elmax = INTVECTOR3(	min(m_elementsize.x, tile->GetElementLocationX() + maxrange),
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
					if (gravity > m_elements[x][y][z].GetGravityStrength())
					{
						m_elements[x][y][z].ChangeGravityStrength(gravity);
					}
				}
			}
		}	// x/y/z
	}		// For each life support system
}

// Performs an update of ship oxygen levels, based on each life support system in the ship
void ComplexShip::PerformShipOxygenUpdate(void)
{
	// Reset the update flag now we are performing an update
	m_oxygenupdaterequired = false;

	// TODO: Do this
}


// Updates the ship navigation network based on the set of elements and their properties
void ComplexShip::UpdateNavigationNetwork(void)
{
	// Make sure the network exists.  If it doesn't, create the network object first
	if (!m_navnetwork) m_navnetwork = new NavNetwork();

	// Initialise the nav network with data from this complex ship
	m_navnetwork->InitialiseNavNetwork(this);

	// TODO: Find any actors currently following a path provided by the previous network, and have them recalculate their paths
}

void ComplexShip::ShutdownNavNetwork(void)
{
	if (m_navnetwork)
	{
		m_navnetwork->Shutdown();
		SafeDelete(m_navnetwork);
	}
}

// Recalculates any perimeter beacons required to maintain spatial position in the world
void ComplexShip::UpdatePerimeterBeacons(void)
{
	D3DXVECTOR3 pos;
	CapitalShipPerimeterBeacon *beacon;
	Octree<iSpaceObject*> *node;

	// We will recalculate any additional nodes during this method, so clear the collection now
	m_activeperimeternodes.clear();
	m_activebeacons = 0;

	// We won't do anything at all if the ship is not present in a system
	if (!m_spaceenvironment) return;

	// Iterate over the collection of all perimeter beacons
	ComplexShip::PerimeterBeaconCollection::iterator it_end = m_perimeterbeacons.end();
	for (ComplexShip::PerimeterBeaconCollection::iterator it = m_perimeterbeacons.begin(); it != it_end; ++it)
	{
		// Make sure the beacon is valid
		beacon = (*it); if (!beacon) continue;

		// Translate the perimeter beacon offset into a world location
		D3DXVec3TransformCoord(&(pos), &(beacon->BeaconPos), &m_worldmatrix);
		beacon->SetPosition(pos);

		// Discount this beacon if it is within the ship node (v likely)
		if (pos.x >= m_treenode->m_xmin && pos.x < m_treenode->m_xmax && 
			pos.y >= m_treenode->m_ymin && pos.y < m_treenode->m_ymax && 
			pos.z >= m_treenode->m_zmin && pos.z < m_treenode->m_zmax )	
		{ 
			if (beacon->Active)
			{
				beacon->Active = false; 
				if (beacon->GetSpatialTreeNode()) beacon->GetSpatialTreeNode()->RemoveItem(beacon);
			}
			continue;
		}

		// Otherwise, also discount it if it is in another node that we already have a beacon for
		bool found = false;
		vector<Octree<iSpaceObject*>*>::const_iterator it_end = m_activeperimeternodes.end();
		for (vector<Octree<iSpaceObject*>*>::const_iterator it = m_activeperimeternodes.begin(); it != it_end; ++it)
		{
			// Get a reference to the node
			node = (*it); if (!node) continue;

			// If the beacon pos is within this node then we already have it covered, so mark as found and stop searching
			if (pos.x >= node->m_xmin && pos.x < node->m_xmax && 
				pos.y >= node->m_ymin && pos.y < node->m_ymax && 
				pos.z >= node->m_zmin && pos.z < node->m_zmax )		{ found = true; break; }
		}

		// If we don't already have a beacon to cover this node then we will use this one
		if (!found)
		{
			// If the beacon is already active in this node (i.e. from last cycle) then that's great; just make sure it should still belong
			node = beacon->GetSpatialTreeNode();
			if (node)
			{
				// If the item has moved then refresh its position for safety
				if (pos.x < node->m_xmin || pos.x < node->m_xmax || 
					pos.y < node->m_ymin || pos.y < node->m_ymax || 
					pos.z < node->m_zmin || pos.z < node->m_zmax )		
				{ 
					node->ItemMoved(beacon, pos); 
					node = beacon->GetSpatialTreeNode();
				}
			}
			else
			{
				// We want to insert the beacon into the spatial partitioning tree now
				node = m_spaceenvironment->SpatialPartitioningTree->AddItem(beacon, pos);
			}

			// In either case, set the beacon to be active and record the new node
			beacon->Active = true;
			++m_activebeacons;
			m_activeperimeternodes.push_back(node);
		}
		else
		{
			// This beacon should not be active; make sure this is the case
			if (beacon->Active)
			{
				beacon->Active = false; 
				if (beacon->GetSpatialTreeNode()) beacon->GetSpatialTreeNode()->RemoveItem(beacon);
				continue;
			}
		}
	}
}

// Deactivates all perimeter beacons
void ComplexShip::DeactivatePerimeterBeacons(void)
{
	CapitalShipPerimeterBeacon *beacon;

	// Iterate over the collection of all perimeter beacons
	ComplexShip::PerimeterBeaconCollection::iterator it_end = m_perimeterbeacons.end();
	for (ComplexShip::PerimeterBeaconCollection::iterator it = m_perimeterbeacons.begin(); it != it_end; ++it)
	{
		// If this beacon is active then we need to take action on it
		beacon = (*it);
		if (beacon && beacon->Active)
		{
			// Remove from the spatial partitioning tree
			if ( beacon->GetSpatialTreeNode() ) beacon->GetSpatialTreeNode()->RemoveItem(beacon);
			
			// Set to inactive
			beacon->Active = false;
		}
	}

	// We now have no active beacons
	m_activebeacons = 0;
}

// Generats the set of capital ship perimeter beacons used for navigation and collision avoidance
void ComplexShip::GenerateCapitalShipPerimeterBeacons(void)
{
	// Make sure we have the latest size data before beginning
	CalculateShipSizeData();

	// Detach and delete any existing beacons attached to this ship
	ShutdownPerimeterBeacons();

	// Determine the number of beacons required in each dimension and the distance between each
	int nx = (int)ceil(m_size.x / Game::C_CS_PERIMETER_BEACON_FREQUENCY) + 1;
	int ny = (int)ceil(m_size.y / Game::C_CS_PERIMETER_BEACON_FREQUENCY) + 1;
	int nz = (int)ceil(m_size.z / Game::C_CS_PERIMETER_BEACON_FREQUENCY) + 1;
	float dx = m_size.x / (float)(nx-1); 
	float dy = m_size.y / (float)(ny-1);
	float dz = m_size.z / (float)(nz-1);

	// First generate beacons in the XY plane, i.e. the ship front and back
	D3DXVECTOR3 pos;
	for (int ix = 0; ix < nx; ix++)
	{
		for (int iy = 0; iy < ny; iy++)
		{
			// We will create each xy beacon in two places; once at z=0 and once at z=zmax.  First, at 0 (back)
			pos = D3DXVECTOR3(ix * dx, iy * dy, 0.0f);
			AttachCapitalShipBeacon(pos);

			// Now also create the beacon at the far z plane (front)
			pos.z = m_size.z;
			AttachCapitalShipBeacon(pos);
		}
	}

	// Now process the XZ dimension, i.e. the top & bottom faces of the ship
	for (int ix = 0; ix < nx; ix++)
	{
		for (int iz = 0; iz < nz; iz++)
		{
			// We want to skip any position where z = 0 or z = zmax, because we have already populated those edges in the previous loop
			if (iz == 0 || iz == (nz-1)) continue;

			// This is a valid intermediate (i.e. not on a z-edge) beacon point.  We want to generate both y = 0 and y = ymax.  
			// First, where y=0 (bottom)
			pos = D3DXVECTOR3(ix * dx, 0.0f, iz * dz);
			AttachCapitalShipBeacon(pos);

			// Now also create the beacon at the max y plane (top)
			pos.y = m_size.y;
			AttachCapitalShipBeacon(pos);
		}
	}

	// Finally, process the YZ dimension, i.e. the left and right sides of the ship
	for (int iy = 0; iy < ny; iy++)
	{
		for (int iz = 0; iz < nz; iz++)
		{
			// We want to skip anywhere that y = [0 or max] or z = [0 or max], since these are already covered (TODO: could shrink loop params)
			if (iy == 0 || iy == (ny-1) || iz == 0 || iz == (nz-1)) continue;

			// This is a valid intermediate (i.e. not on a z-edge or y-edge) beacon point.  We want to generate both x = 0 and x = xmax.  
			// First, where x=0 (left)
			pos = D3DXVECTOR3(0.0f, iy * dy, iz * dz);
			AttachCapitalShipBeacon(pos);

			// Now also create the beacon at the max x plane (right)
			pos.x = m_size.x;
			AttachCapitalShipBeacon(pos);
		}
	}
}

// Method to attach a capital ship perimeter beacon to the ship at the specified location
void ComplexShip::AttachCapitalShipBeacon(D3DXVECTOR3 position)
{
	// Create and assign the beacon to this ship
	CapitalShipPerimeterBeacon *beacon = new CapitalShipPerimeterBeacon();
	beacon->AssignToShip(this, position);

	// Keep a record of the beacon attachment in this capital ship
	m_perimeterbeacons.push_back(beacon);
}

void ComplexShip::RecalculateShipDataFromCurrentState() 
{
	// Perform an update of the ship based on all hardpoints
	HardpointChanged(NULL);

	// Recalculate ship properties based on all our component sections, loadout and any modifiers
	CalculateShipSizeData();
	CalculateShipMass();
	CalculateVelocityLimits();
	//CalculateBrakeFactor();		// Dependent on velocity limit, so is called directly from within that function instead
	CalculateTurnRate();
	CalculateBankRate();
	CalculateBankExtents();

	// Iterate through each ship section in turn, pulling any required info and also resetting the update flags
	bool sections_updated = false;
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		if ((*it)->SectionIsUpdated())
		{
			// Extract information required to perform the ship refresh

			// Record the fact that at least one section has changed, in order to perform any global updates afterwards
			sections_updated = true;
		}

		// Reset the section update flag now that we have accounted for its latest info
		(*it)->ClearSectionUpdateFlag();
	}

	// Perform any global update, in case any section was updated
	if (sections_updated)
	{
		BuildHardpointCollection();
	}

	// Recalculate all terrain object positions based on this potential change to the environment size
	std::vector<StaticTerrain*>::iterator it2_end = TerrainObjects.end();
	for (std::vector<StaticTerrain*>::iterator it2 = TerrainObjects.begin(); it2 != it2_end; ++it2)
	{
		if ((*it2)) (*it2)->RecalculatePositionalData();
	}
}

void ComplexShip::CalculateShipSizeData(void)
{
	ComplexShipSection *sec = NULL;
	D3DXVECTOR3 bmin, bmax;

	// We need at least one ship section in order to derive meaningful data
	if (m_sections.size() == 0)
	{
		this->MinBounds = D3DXVECTOR3(-0.5f, -0.5f, -0.5f);
		this->MaxBounds = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
		this->SetSize(D3DXVECTOR3(1.0f, 1.0f, 1.0f));
		this->SetCentreOffsetTranslation(NULL_VECTOR);
		this->SetZeroPointTranslation(NULL_VECTOR);
		return; 
	}
	 
	// Otherwise we want to determine the min and max bounds of the ship based on each section
	this->MinBounds = D3DXVECTOR3(99999.0f, 99999.0f, 99999.0f);
	this->MaxBounds = D3DXVECTOR3(-99999.0f, -99999.0f, -99999.0f);

	// Iterate over each ship section in turn
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Make sure the section is valid
		sec = (*it); 
		if (!sec || !sec->GetModel() || !sec->GetModel()->IsGeometryLoaded()) continue;

		// Have the section recalculate its own size
		sec->CalculateShipSizeData();

		// Determine the min and max bounds of this ship section, based on the model size about its centre point
		bmin = sec->GetRelativePosition() - (sec->GetSize() * 0.5f);
		bmax = sec->GetRelativePosition() + (sec->GetSize() * 0.5f);

		// Add the ship section offset to each, to get the actual bounds in world space
		//bmin += sec->GetPosition(); bmax += sec->GetPosition();

		// If the bounds are now outside our current min & max, record them as the new ship limits
		if (bmin.x < this->MinBounds.x) this->MinBounds.x = bmin.x;
		if (bmax.x > this->MaxBounds.x) this->MaxBounds.x = bmax.x;
		if (bmin.y < this->MinBounds.y) this->MinBounds.y = bmin.y;
		if (bmax.y > this->MaxBounds.y) this->MaxBounds.y = bmax.y;
		if (bmin.z < this->MinBounds.z) this->MinBounds.z = bmin.z;
		if (bmax.z > this->MaxBounds.z) this->MaxBounds.z = bmax.z;
	}
	
	// Now calculate ship size based on these bounds
	// *** OVERRIDE ship size based on the size of this environment in elements ***
	/*this->SetSize(D3DXVECTOR3(	this->MaxBounds.x - this->MinBounds.x, this->MaxBounds.y - this->MinBounds.y,
								this->MaxBounds.z - this->MinBounds.z));*/
	this->SetSize(Game::ElementLocationToPhysicalPosition(this->GetElementSize()));

	// Also recalculate the ship centre offset
	this->SetCentreOffsetTranslation(-(D3DXVECTOR3(	(this->MaxBounds.x + this->MinBounds.x) * 0.5f,
													(this->MaxBounds.y + this->MinBounds.y) * 0.5f,
													(this->MaxBounds.z + this->MinBounds.z) * 0.5f)));

	// Complex ships are all space environments.  Calculate the environment zero-element translation at this point as well, 
	// based upon the overall object size.  This translates from the object centre to its (0,0,0) element position
	this->SetZeroPointTranslation(m_size * -0.5f);
}


void ComplexShip::CalculateShipMass()
{
	// We need at least one ship section in order to derive meaningful data
	if (m_sections.size() == 0) { this->SetMass(1.0f); return; }
	
	// Start from zero mass and incorporate the mass of each ship section in turn
	float mass = 0.0f;
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Have the ship section recalculate its own mass, then add its contribution to the overall ship value
		(*it)->CalculateShipMass();
		mass += (*it)->GetMass();
	}

	// Make sure we have a nonzero mass as a sanity check, to prevent divide-by-zero errors later on
	if (mass == 0.0f) mass = 1.0f;

	// Set the base value before applying any modifiers
	BaseMass = mass;

	// TODO: Add the contribution of cargo to ship mass

	// Apply other factors, equipment modifiers etc.

	// Assign this total mass to the ship
	this->SetMass(mass);
}

void ComplexShip::CalculateVelocityLimits()
{
	// We need at least one ship section in order to derive meaningful data
	if (m_sections.size() == 0) { this->VelocityLimit.SetAllValues(1.0f); this->AngularVelocityLimit.SetAllValues(1.0f); return; }
	
	// The ship velocity limits will be the minimum of all component ship section limits
	float vlimit = 99999999.0f; float alimit = 9999999.0f; bool havelimit = false;

	// Consider each ship section in turn
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Have the ship section recalculate its own velocity limit first
		(*it)->CalculateVelocityLimits();

		// If the velocity limit of this section is lower than the current ship limit, take the new value as our limiting factor
		if ((*it)->GetVelocityLimit() < vlimit) vlimit = (*it)->GetVelocityLimit();
		if ((*it)->GetAngularVelocityLimit() < alimit) alimit = (*it)->GetAngularVelocityLimit();

		havelimit = true;
	}

	// If we didn't retrieve any limit from component sections then select a default
	if (!havelimit) { vlimit = 1.0f; alimit = 1.0f; }

	// Set the base values before applying modifiers
	this->VelocityLimit.BaseValue = vlimit;
	this->AngularVelocityLimit.BaseValue = alimit;

	// Apply other factors, equipment modifiers etc.

	// Apply the final velocity limits to this ship
	this->VelocityLimit.Value = vlimit;
	this->AngularVelocityLimit.Value = alimit;

	// Finally, also trigger a recalculation of the (dependent) brake factors for this ship
	CalculateBrakeFactor();
}

void ComplexShip::CalculateBrakeFactor()
{
	// We need at least one ship section in order to derive meaningful data
	if (m_sections.size() == 0) { this->BrakeFactor.SetAllValues(1.0f); this->BrakeAmount = 1.0f; return; }
	
	// We will determine the total braking amount as a percentage of total velocity limits, then scale to the ship parameters
	float amount = 0.0f; float maxvelocity = 0.0f;

	// Consider each ship section in turn
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Have the ship section recalculate its own braking amounts first
		(*it)->CalculateBrakeFactor();

		// Add the brake amount contributed by this section to the overall total, and also keep a running total of the max velocity
		amount += (*it)->GetBrakeAmount();
		maxvelocity += (*it)->GetVelocityLimit();
	}

	// Make sure we have valid data to work with
	if (amount == 0.0f || maxvelocity == 0.0f) { this->BrakeFactor.SetAllValues(1.0f); this->BrakeAmount = 1.0f; return; }

	// Set base brake factor value before applying any modifiers
	this->BrakeFactor.BaseValue = (amount / maxvelocity);

	// Apply other factors, equipment modifiers etc.

	// Determine final values; divide the total braking strength by the total max velocity to get a brake factor for the ship as a whole
	this->BrakeFactor.Value = (amount / maxvelocity);

	// The ship velocity limit is however Min(L1, L2, ..., Ln), so apply this braking factor against the actual ship velocity limit to get a ship braking amount
	this->BrakeAmount = (this->BrakeFactor.Value * this->VelocityLimit.Value);
}

void ComplexShip::CalculateTurnRate()
{
	// We need at least one ship section in order to derive meaningful data
	if (m_sections.size() == 0) { this->TurnRate.SetAllValues(0.01f); this->TurnAngle.SetAllValues(0.01f); return; }
	
	// As a base value, the ship turn rate will be the minimum of all component ship turn rate limits
	float turnrate = 99999999.0f, turnangle = 99999999.0f; 
	bool havevalues = false;

	// Consider each ship section in turn
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		ComplexShipSection *sec = (*it);
		if (!sec) continue;

		// Have the ship section recalculate its own turn rate first
		sec->CalculateTurnRate();

		// If the turn rate of this section is lower than the current ship limit, take the new value as our limiting factor
		// TOOD: Note section turn rate should also take into account the number of lateral thrusters installed
		if (sec->GetTurnRate() < turnrate) turnrate = sec->GetTurnRate();

		// If the turn angle of this section is lower than the current ship limit, take this new value as our limiting factor
		if (sec->GetTurnAngle() < turnangle) turnangle = sec->GetTurnAngle();

		// We have received values for at least one ship section
		havevalues = true;
	}

	// If we didn't retrieve any limit from component sections then select a default
	if (!havevalues) { turnrate = 0.01f; turnangle = 0.01f; }
	
	// Set base values before applying any modifiers
	this->TurnRate.BaseValue = turnrate;
	this->TurnAngle.BaseValue = turnangle;

	// Apply other factors, equipment modifiers etc.

	// Apply the turn rate limit to this ship
	this->TurnRate.Value = turnrate;
	this->TurnAngle.Value = turnangle;
}

void ComplexShip::CalculateBankRate()
{
	// We need at least one ship section in order to derive meaningful data
	if (m_sections.size() == 0) { this->BankRate.SetAllValues(0.0f); return; }
	
	// As a base value, the ship turn rate will be the minimum of all component ship turn rate limits
	float bankrate = 99999999.0f; bool haverate = false;

	// Consider each ship section in turn
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Have the ship section recalculate its own turn bank first
		(*it)->CalculateBankRate();

		// If the bank rate of this section is lower than the current ship limit, take the new value as our limiting factor
		if ((*it)->GetBankRate() < bankrate) bankrate = (*it)->GetBankRate();
		haverate = true;
	}

	// If we didn't retrieve any limit from component sections then select a default
	if (!haverate) bankrate = 0.0f;

	// Set base values before applying any modifiers
	this->BankRate.BaseValue = bankrate;
	
	// Apply other factors, equipment modifiers etc.

	// Apply the turn rate limit to this ship
	this->BankRate.Value = bankrate;
}

void ComplexShip::CalculateBankExtents()
{
	// We need at least one ship section in order to derive meaningful data
	if (m_sections.size() == 0) { this->BankExtent = NULL_VECTOR; return; }
	
	// As a base value, the ship turn rate will be the minimum of all component ship turn rate limits
	D3DXVECTOR3 bankextent = D3DXVECTOR3(TWOPI, TWOPI, TWOPI); bool haverate = false;

	// Consider each ship section in turn
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Have the ship section recalculate its own turn bank first
		(*it)->CalculateBankExtents();

		// If the bank extent of this section is lower in any dimension than the current ship limit, take the new value as our limiting factor
		D3DXVECTOR3 section = (*it)->GetBankExtents();
		bankextent = D3DXVECTOR3(min(bankextent.x, section.x), min(bankextent.y, section.y), min(bankextent.z, section.z));
		haverate = true;
	}

	// If we didn't retrieve any limit from component sections then select a default
	if (!haverate) bankextent = NULL_VECTOR;
	
	// Apply other factors, equipment modifiers etc.

	// Apply the turn rate limit to this ship
	this->BankExtent = bankextent;
}


void ComplexShip::CalculateEngineStatistics()
{
	// We need at least one ship section in order to derive meaningful data
	if (m_sections.size() == 0) { this->EngineAngularAcceleration.SetAllValues(1000.0f); return; }

	// Retrieve the vector of all engine hardpoints on the ship
	Hardpoints::HardpointCollection & engines = m_hardpoints.GetHardpointsOfType(Equip::Class::Engine);

	// Now sum the total engine acceleration value from each engine to determine an engine angular acceleration value
	HpEngine *hp; Engine *e; float accel = 0.0f;
	int n = (int)engines.size();

	for (int i = 0; i < n; ++i)
	{
		// Attempt to get a reference to the hardpoint, and the engine mounted on it (if there is one)
		hp = (HpEngine*)engines.at(i);	if (!hp) continue;
		e = (Engine*)hp->GetEngine();	if (!e) continue;

		// Add the engine acceleration 
		accel += e->Acceleration;
	}

	// Set the base value before applying any modifiers
	this->EngineAngularAcceleration.BaseValue = accel;

	// Apply any modifiers based on e.g. equipment

	// Store the final value after applying modifers
	this->EngineAngularAcceleration.Value = accel;
}

ComplexShip *ComplexShip::Create(const string & code)
{
	// Attempt to get the ship template matching this code; if it doesn't exist then return NULL
	ComplexShip *template_ship = D::GetComplexShip(code);
	if (template_ship == NULL) return NULL;

	// Invoke the spawn function using these template details & return the result
	return (ComplexShip::Create(template_ship));
}

ComplexShip *ComplexShip::Create(ComplexShip *template_ship)
{
	// If we are passed an invalid class pointer then return NULL immediately
	if (template_ship == NULL) return NULL;

	// Create a new instance of the ship from this template; class-specific initialisation will all be performed
	// automatically up the inheritance hierarchy as part of this operation.  If any part of the operation fails, 
	// the returned value will be NULL (which we then pass on as the return value from this function)
	ComplexShip *cs = CopyObject<ComplexShip>(template_ship);

	// Return a pointer to the newly-created ship (or NULL if creation or initialisation failed)
	return cs;
}

void ComplexShip::RecalculateAllShipData(void)
{
	// Generate a set of perimeter beacons for the new capital ship
	GenerateCapitalShipPerimeterBeacons();

	// Update the ship navigation network based on its component elements
	UpdateNavigationNetwork();

	// Recalculate ship statistics via the 'light' update method before releasing it
	RecalculateShipDataFromCurrentState();

}

// Method called when this object collides with another.  Overriden method providing a section reference
// is the one that will be used for CS, since only the sections themselves can collide with anything
void ComplexShip::CollisionWithObject(iObject *object, ComplexShipSection *collidingsection, const GamePhysicsEngine::ImpactData & impact)
{
	
}

// Copies all tiles from another object and adds the copies to this object
Result ComplexShip::CopyTileDataFromObject(iContainsComplexShipTiles *src)
{
	// Parameter check
	if (!src) return ErrorCodes::CannotCopyTileDataFromNullSource;

	// Remove any tile data that currently exists for the environment; do not use the RemoveTile() methods since the
	// existing tiles belong to the source environment, which we just copied from
	RemoveAllShipTiles();

	// Remove all terrain objects which were introduced as part of a tile; create a new vector, populate with any 
	// terrain objects that are still valid, and then swap the vectors at the end.  More efficient than removing
	// items from the main vector since the majority will likely be removed here.
	std::vector<StaticTerrain*> terrain;
	std::vector<StaticTerrain*>::size_type n = TerrainObjects.size();
	for (std::vector<StaticTerrain*>::size_type i = 0; i < n; ++i)
	{
		if (TerrainObjects[i]->GetParentTileID() == 0) terrain.push_back(TerrainObjects[i]);
	}
	TerrainObjects = terrain;

	//  Iterate through each tile in the source in turn
	iContainsComplexShipTiles::ConstTileIterator it_end = src->GetTileIteratorEnd();
	for (iContainsComplexShipTiles::ConstTileIterator it = src->GetTileIteratorStart(); it != it_end; ++it)
	{
		// Get a reference to this tile and make a copy via virtual subclass copy method
		ComplexShipTile *tile = (*it)->Copy();

		// Link this cloned tile to the new ship
		if (tile) tile->LinkToParent(this);
	}

	// Return success
	return ErrorCodes::NoError;
}

// Fits the element space around this ship, eliminating any extra space allocated outside of the (cuboid) bounds it requires
Result ComplexShip::FitElementSpaceToShip(void)
{
	// Retrieve the maximum bounds occupied by the sections of this ship
	INTVECTOR3 bounds = GetShipMaximumBounds();

	// Now request a reallocation to this maximum extent; if there is no change to be made it will simply return
	return ReallocateElementSpace(bounds);
}

// Returns the maximum bounds occupied by this ship.  May not necessarily == elementsize when sections are being moved around
INTVECTOR3 ComplexShip::GetShipMaximumBounds(void)
{
	INTVECTOR3 bounds = INTVECTOR3(0, 0, 0);

	// Iterate over the collection of ship sections to determine the maximum bounds we are using
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Get a reference to this ship section
		ComplexShipSection *sec = (*it);						
		if (!sec) continue;

		// Determine the location of the far extent of this section
		INTVECTOR3 size = sec->GetElementLocation();
		size = INTVECTOR3(	size.x + sec->GetElementSizeX() - 1, size.y + sec->GetElementSizeY() - 1,
							size.z + sec->GetElementSizeZ() - 1);

		// Store the extent of this section if it extends beyond the current bounds in any dimension
		if (size.x > bounds.x) bounds.x = size.x;
		if (size.y > bounds.y) bounds.y = size.y;
		if (size.z > bounds.z) bounds.z = size.z;
	}

	// Return the maximum bounds
	return bounds;
}

// Returns the maximum bounds occupied by this ship.  May not necessarily == elementsize when sections are being moved around
INTVECTOR3 ComplexShip::GetShipMinimumBounds(void)
{
	// Special case; if there are no ship sections then return (0,0,0) by default
	if (this->GetSectionCount() == 0) return INTVECTOR3(0, 0, 0);

	// Otherwise we want to determine the minimum element value in each dimension that is occupied (i.e. our min bounds)
	INTVECTOR3 bounds = INTVECTOR3(INT_MAX, INT_MAX, INT_MAX);

	// Iterate over the collection of ship sections to determine the minimum bounds we are using
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Get a reference to this ship section
		ComplexShipSection *sec = (*it);						
		if (!sec) continue;

		// Determine the location of this section
		INTVECTOR3 location = sec->GetElementLocation();

		// Store the section location if it is less than the current minimum bounds we have recorded
		if (location.x < bounds.x) bounds.x = location.x;
		if (location.y < bounds.y) bounds.y = location.y;
		if (location.z < bounds.z) bounds.z = location.z;
	}

	// Return the maximum bounds
	return bounds;
}

// Returns a reference to the ship section that contains the element at this location
ComplexShipSection *ComplexShip::GetShipSectionContainingElement(INTVECTOR3 location)
{
	// Iterate through the ship section collection
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Get a reference to this section
		ComplexShipSection *sec = (*it);
		if (!sec) continue;

		// Determine the bounds of this section in ship space
		INTVECTOR3 lbound = sec->GetElementLocation();
		INTVECTOR3 ubound = sec->GetElementSize();
		ubound.x += (lbound.x - 1); ubound.y += (lbound.y - 1); ubound.z += (lbound.z - 1);

		// If the specified location falls within this ship section then return a reference to it
		if (location.x >= lbound.x && location.y >= lbound.y && location.z >= lbound.z &&
			location.x <= ubound.x && location.y <= ubound.y && location.z <= ubound.z)
			return sec;
	}

	// If no section contains this element then return NULL
	return NULL;
}


// Removes the 'standard' flag from this ship's definition and it's sections.  Used following a copy from a standard template ship
void ComplexShip::RemoveStandardComponentFlags(void)
{
	// Remove the flag from this ship object first
	m_standardobject = false;

	// Now update each ship section in turn
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Remove the 'standard' flag from this section
		if ((*it)) (*it)->SetIsStandardObject(false);
	}
}


// Performs a text output of perimeter beacon data for debug purposes
std::string ComplexShip::DebugOutputPerimeterBeacons(void)
{
	CapitalShipPerimeterBeacon *beacon;
	Octree<iSpaceObject*> *node;
	string s = "";

	// Iterate over each beacon attached to the ship
	ComplexShip::PerimeterBeaconCollection::const_iterator it_end = m_perimeterbeacons.end();
	for (ComplexShip::PerimeterBeaconCollection::const_iterator it = m_perimeterbeacons.begin(); it != it_end; ++it)
	{
		// Concatemate details on this beacon to the debug string
		beacon = (*it); if (!beacon) continue;
		node = beacon->GetSpatialTreeNode();

		// Add basic info from this beacon
		s = concat(s)("Beacon ")(beacon->GetID())(beacon->Active ? (": ACTIVE") : (": Inactive"))
												 (", Offset = ")(VectorToString(beacon->BeaconPos))
												 (", Position = ")(VectorToString(beacon->GetPosition())).str();

		// Add additional info on the node this beacon sits within, if relevant
		if (node)
			s = concat(s)(", Node (Depth ")(beacon->GetSpatialTreeNode()->DetermineTreeDepth())(") = ")
						 (VectorToString(beacon->GetSpatialTreeNode()->ConstructMinBoundsVector()))(" to ")
						 (VectorToString(beacon->GetSpatialTreeNode()->ConstructMaxBoundsVector()))("\n").str();
		else
			s = concat(s)(", No spatial positioning node\n").str();
	}

	// Return the complete string
	return s;
}

ComplexShip::~ComplexShip(void)
{
	
}

// Returns the file path where XML data relating to this ship should be stored
string ComplexShip::DetermineXMLDataPath(void)
{
	return concat("\\Ships\\ShipData\\")(m_code).str();
}
// Returns the name of the XML file to be generated for this ship
string ComplexShip::DetermineXMLDataFilename(void)
{
	return concat(m_code)(".xml").str();
}
// Returns the full expected filename for the XML data relating to this ship
string ComplexShip::DetermineXMLDataFullFilename(void)
{
	return concat(DetermineXMLDataPath())("\\")(DetermineXMLDataFilename()).str();
}

