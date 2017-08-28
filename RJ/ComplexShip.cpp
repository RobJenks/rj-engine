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
	m_shipclass = Ship::ShipClass::Complex;

	// Set ship properties to default values on object creation
	m_perimeterbeacons.clear();
	m_activeperimeternodes.clear();
	m_activebeacons = 0;
	m_forcerenderinterior = false;
	m_suspendupdates = false;

	// This class of space object will perform full collision detection by default (iSpaceObject default = no collision)
	SetCollisionMode(Game::CollisionMode::FullCollision);

	// Recalculate ship stats so they are consistent with the properties set during object construction
	RecalculateShipDataFromCurrentState();
}


// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void ComplexShip::InitialiseCopiedObject(ComplexShip *source)
{
	// Pass control to all base classes
	iSpaceObjectEnvironment::InitialiseCopiedObject(source);

	/* Now perform ComplexShip-specific initialisation logic for new objects */

	// Clear the ship hardpoints collection so that it is rebuilt as new sections are added
	GetHardpoints().SuspendUpdates();
	GetHardpoints().ClearData();

	// Also clear the perimeter beacon collection so we can recreate our own instances
	m_perimeterbeacons.clear();

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
	iSpaceObjectEnvironment::SimulationStateChanged(prevstate, newstate);

	// If we were not being simulated, and we now are, then we may need to take some ComplexShip-specific actions here
	// TODO: this will not always be true in future when we have more granular simulation states 
	if (prevstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// Perform a full recalculation, which will generate new perimeter beacons, a nav network etc. for use during active simulation
		// TODO: This will call the nav network generation function twice, since the iSpaceObjectEnvironment class (which owns the nav
		// network) will call this during its SimulationStateChanged() event.  Fix this in future
		RecalculateAllShipData();
	}

	// Conversely, if we are no longer going to be simulated, we can remove the perimeter beacons etc. 
	if (newstate == iObject::ObjectSimulationState::NoSimulation)
	{
		ShutdownPerimeterBeacons();
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

// Set the base properties of the complex ship environment; implementation of virtual environment method
void ComplexShip::SetBaseEnvironmentProperties(void)
{
	// Base properties are applied by our ship sections
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end(); 
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		const ComplexShipSection *sec = (*it);
		INTVECTOR3 size = sec->GetElementSize();
		INTVECTOR3 loc = sec->GetElementLocation();

		INTVECTOR3 section;
		for (section.x = 0; section.x < size.x; ++section.x) {
			for (section.y = 0; section.y < size.y; ++section.y) {
				for (section.z = 0; section.z < size.z; ++section.z)
				{
					ComplexShipElement *el = GetElement(loc + section);
					if (el) el->ApplyElementState(sec->DefaultElementState.GetElementState(section, sec->GetRotation()));
				}
			}
		}
	}
}

// Method to handle the addition of a ship tile to this object
void ComplexShip::TileAdded(ComplexShipTile *tile)
{
	// Pass control down to base classes
	iSpaceObjectEnvironment::TileAdded(tile);
	
	// (Implement any complex ship-specific logic here)

}

// Method to handle the removal of a ship tile from this object
void ComplexShip::TileRemoved(ComplexShipTile *tile)
{
	// Pass control down to base classes
	iSpaceObjectEnvironment::TileRemoved(tile);

	// (Implement any complex ship-specific logic here)

}


// Method triggered when the layout (e.g. active/walkable state, connectivity) of elements is changed
void ComplexShip::ElementLayoutChanged(void)
{
	// Pass control down to base classes
	iSpaceObjectEnvironment::ElementLayoutChanged();

	// (Implement any complex ship-specific logic here)

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

// Overrides the virtual iSpaceObject method to ensure that all ship sections are also moved into 
// the environment along with the 'ship' itself
void ComplexShip::MoveIntoSpaceEnvironment(SpaceSystem *system)
{
	// Move the ship itself into the environment by calling the base iSpaceObject method
	iSpaceObject::MoveIntoSpaceEnvironment(system);

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
			ComplexShipSection *sec = m_sections[i];
			iSpaceObject *sobj = (iSpaceObject*)sec;
			OutputDebugString(sec->GetName().c_str());
			OutputDebugString(sec->GetSpaceEnvironment() ? sec->GetSpaceEnvironment()->GetName().c_str() : "");

			m_sections[i]->MoveIntoSpaceEnvironment(system);

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

	// Make sure that the spatial tree node for this ship contains the WHOLE ship
	if (m_treenode)
	{
		// Test whether the ship lies completely within the node
		if (XMVector3GreaterOrEqual(XMVectorSubtract(m_position, m_size), m_treenode->m_min) && 
			XMVector3Less(XMVectorAdd(m_position, m_size), m_treenode->m_max))
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

// Recalculates any perimeter beacons required to maintain spatial position in the world
void ComplexShip::UpdatePerimeterBeacons(void)
{
	XMVECTOR pos;
	CapitalShipPerimeterBeacon *beacon;
	Octree<iObject*> *node;

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
		pos = XMVector3TransformCoord(beacon->BeaconPos, m_worldmatrix);
		beacon->SetPosition(pos);

		// Discount this beacon if it is within the ship node (v likely)
		if (m_treenode->ContainsPoint(pos))
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
		vector<Octree<iObject*>*>::const_iterator it2_end = m_activeperimeternodes.end();
		for (vector<Octree<iObject*>*>::const_iterator it2 = m_activeperimeternodes.begin(); it2 != it2_end; ++it2)
		{
			// Get a reference to the node
			node = (*it2); if (!node) continue;

			// If the beacon pos is within this node then we already have it covered, so mark as found and stop searching
			if (node->ContainsPoint(pos)) { found = true; break; }
		}

		// If we don't already have a beacon to cover this node then we will use this one
		if (!found)
		{
			// If the beacon is already active in this node (i.e. from last cycle) then that's great; just make sure it should still belong
			node = beacon->GetSpatialTreeNode();
			if (node)
			{
				// If the item has moved then refresh its position for safety
				if (!node->ContainsPoint(pos))
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
	// For each component:	int nx = (int)ceil(m_size.x / Game::C_CS_PERIMETER_BEACON_FREQUENCY) + 1;
	//						float dx = m_size.x / (float)(nx-1); 
	XMVECTOR nv = XMVectorAdd(XMVectorCeiling(XMVectorDivide(m_size, Game::C_CS_PERIMETER_BEACON_FREQUENCY_V)), ONE_VECTOR);
	XMVECTOR dv = XMVectorDivide(m_size, XMVectorSubtract(nv, ONE_VECTOR));
	
	// Store a local integer representation of the beacon count
	XMFLOAT3 nvf, dvf, sizef; 
	XMStoreFloat3(&nvf, nv);
	XMStoreFloat3(&dvf, dv);
	XMStoreFloat3(&sizef, m_size);
	int nx = (int)nvf.x; int ny = (int)nvf.y; int nz = (int)nvf.z;

	// First generate beacons in the XY plane, i.e. the ship front and back
	XMVECTOR pos;
	for (int ix = 0; ix < nx; ++ix)
	{
		for (int iy = 0; iy < ny; ++iy)
		{
			// We will create each xy beacon in two places; once at z=0 and once at z=zmax.  First, at 0 (back)
			pos = XMVectorSet(dvf.x * ix, dvf.y * iy, 0.0f, 0.0f);
			AttachCapitalShipBeacon(pos);

			// Now also create the beacon at the far z plane (front)
			AttachCapitalShipBeacon(XMVectorSetZ(pos, sizef.z));
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
			pos = XMVectorSet(dvf.x * ix, 0.0f, dvf.z * iz, 0.0f);
			AttachCapitalShipBeacon(pos);

			// Now also create the beacon at the max y plane (top)
			AttachCapitalShipBeacon(XMVectorSetY(pos, sizef.y));
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
			pos = XMVectorSet(0.0f, dvf.y * iy, dvf.z * iz, 0.0f);
			AttachCapitalShipBeacon(pos);

			// Now also create the beacon at the max x plane (right)
			AttachCapitalShipBeacon(XMVectorSetX(pos, sizef.x));
		}
	}
}

// Method to attach a capital ship perimeter beacon to the ship at the specified location
void ComplexShip::AttachCapitalShipBeacon(FXMVECTOR position)
{
	// Create and assign the beacon to this ship
	CapitalShipPerimeterBeacon *beacon = new CapitalShipPerimeterBeacon();
	beacon->AssignToShip(this, position);

	// Keep a record of the beacon attachment in this capital ship
	m_perimeterbeacons.push_back(beacon);
}

void ComplexShip::RecalculateShipDataFromCurrentState() 
{
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

	// Set all dirty flags to force recalculation of derived data next cycle
	SetThrustVectorChangeFlag();
	SetShipMassChangeFlag();
}

void ComplexShip::CalculateShipSizeData(void)
{
	// We need at least one ship section in order to derive meaningful data
	if (m_sections.size() == 0)
	{
		this->MinBounds = HALF_VECTOR_N;
		this->MaxBounds = HALF_VECTOR_P;
		this->SetSize(ONE_VECTOR);
		this->SetCentreOffsetTranslation(NULL_VECTOR);
		this->SetZeroPointTranslation(NULL_VECTOR);
		return; 
	}
	 
	// Otherwise we want to determine the min and max bounds of the ship based on each section
	this->MinBounds = LARGE_VECTOR_P;
	this->MaxBounds = LARGE_VECTOR_N;
	XMVECTOR bmin, bmax;
	
	// Iterate over each ship section in turn
	ComplexShipSection *sec = NULL; int processed = 0;
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Make sure the section is valid
		sec = (*it); 
		if (!sec || !sec->GetModel() || !sec->GetModel()->IsGeometryLoaded()) continue;

		// Have the section recalculate its own size
		sec->CalculateShipSizeData();

		// Determine the min and max bounds of this ship section, based on the model size about its centre point
		bmin = XMVectorSubtract(sec->GetRelativePosition(), XMVectorMultiply(sec->GetSize(), HALF_VECTOR));
		bmax = XMVectorAdd(sec->GetRelativePosition(), XMVectorMultiply(sec->GetSize(), HALF_VECTOR));

		// Add the ship section offset to each, to get the actual bounds in world space
		//bmin += sec->GetPosition(); bmax += sec->GetPosition();

		// If the bounds are now outside our current min & max, record them as the new ship limits
		MinBounds = XMVectorMin(MinBounds, bmin);
		MaxBounds = XMVectorMax(MaxBounds, bmax);

		// We have processed this section
		++processed;
	}
	
	// Make sure we actually processed something
	if (processed == 0)
	{
		this->MinBounds = HALF_VECTOR_N;
		this->MaxBounds = HALF_VECTOR_P;
		this->SetSize(ONE_VECTOR);
		this->SetCentreOffsetTranslation(NULL_VECTOR);
		this->SetZeroPointTranslation(NULL_VECTOR);
		return;
	}

	// Ship size OVERRIDE - must match size in elements.  TODO: Remove above and based all on elements in future?
	this->SetSize(Game::ElementLocationToPhysicalPosition(this->GetElementSize()));

	// Also recalculate the ship centre offset
	// SetCentreOffsetTranslation(-(D3DXVECTOR3(	(this->MaxBounds.x + this->MinBounds.x) * 0.5f, ..., ...)))
	SetCentreOffsetTranslation(XMVectorNegate(XMVectorMultiply(XMVectorAdd(MinBounds, MaxBounds), HALF_VECTOR)));

	// Complex ships are all space environments.  Calculate the environment zero-element translation at this point as well, 
	// based upon the overall object size.  This translates from the object centre to its (0,0,0) element position
	this->SetZeroPointTranslation(XMVectorMultiply(m_size, HALF_VECTOR_N));
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

	// Set the mass change flag so that derived data can be updated
	SetShipMassChangeFlag();

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

	// Also store in vectorised form for fast per-frame calculations
	m_vlimit_v = XMVectorReplicate(VelocityLimit.Value);
	m_avlimit_v = XMVectorReplicate(AngularVelocityLimit.Value);

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

	// Also store turn rate in vectorised form for fast per-frame calculations
	m_turnrate_v = XMVectorReplicate(TurnRate.Value);
	m_turnrate_nv = XMVectorNegate(m_turnrate_v);
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
	if (m_sections.size() == 0) { SetBankExtent(NULL_VECTOR3); return; }
	
	// As a base value, the ship turn rate will be the minimum of all component ship turn rate limits
	XMVECTOR bankextent = XMVectorReplicate(TWOPI); bool haverate = false;

	// Consider each ship section in turn
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Have the ship section recalculate its own turn bank first
		(*it)->CalculateBankExtents();

		// If the bank extent of this section is lower in any dimension than the current ship limit, take the new value as our limiting factor
		bankextent = XMVectorMin(bankextent, (*it)->GetBankExtents());
		haverate = true;
	}

	// If we didn't retrieve any limit from component sections then select a default
	if (!haverate) bankextent = NULL_VECTOR;
	
	// Apply other factors, equipment modifiers etc.

	// Apply the turn rate limit to this ship
	SetBankExtent(bankextent);
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
		accel += e->GetAcceleration();
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
	ComplexShip *template_ship = D::ComplexShips.Get(code);
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

void ComplexShip::CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact)
{
	// Pass to the base class method
	iActiveObject::CollisionWithObject(object, impact);

	// Pass to the environment method to determine any internal ship damage 
	RegisterEnvironmentImpact(object, impact);
}

// Method called when this object collides with another.  Overriden method providing a section reference
// is the one that will be used for CS, since only the sections themselves can collide with anything
void ComplexShip::CollisionWithObject(iActiveObject *object, ComplexShipSection *collidingsection, const GamePhysicsEngine::ImpactData & impact)
{
	throw "SECTIONS SHOULD NOT COLLIDE ANYMORE";
}

// Fits the element space around this ship, eliminating any extra space allocated outside of the (cuboid) bounds it requires
Result ComplexShip::FitElementSpaceToShip(void)
{
	// Retrieve the maximum bounds occupied by the sections of this ship
	INTVECTOR3 bounds = GetShipMaximumBounds();

	// Now request a reallocation to this maximum extent; if there is no change to be made it will simply return
	return InitialiseElements(bounds, true);
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
		INTVECTOR3 size = (sec->GetElementLocation() + sec->GetElementSize() - ONE_INTVECTOR3);

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

// Apply a fade effect to all ship tiles in this environment
void ComplexShip::FadeHullToAlpha(float time, float alpha, bool ignore_pause)
{
	// Parameter checks
	time = clamp(time, 0.0f, (1000.0f * 60.0f * 60.0f * 12.0f));	// 0 secs to 12hrs, for sanity
	alpha = clamp(alpha, 0.0f, 1.0f);

	// Iterate through all sections in the collection and apply this fade value
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		(*it)->Fade.FadeToAlpha(time, alpha, ignore_pause);
	}
}

// Fades the entire ship (hull & interior) to the specified alpha level
void ComplexShip::FadeEntireShipToAlpha(float time, float alpha, bool ignore_pause)
{
	// Parameter checks
	time = clamp(time, 0.0f, (1000.0f * 60.0f * 60.0f * 12.0f));	// 0 secs to 12hrs, for sanity
	alpha = clamp(alpha, 0.0f, 1.0f);

	// Fade both the hull and interior of the ship
	FadeHullToAlpha(time, alpha, ignore_pause);
	FadeAllTiles(time, alpha, ignore_pause);
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

// Event triggered upon destruction of the object
void ComplexShip::DestroyObject(void)
{
	// Generate destruction effects and debris

	// Perform any other ComplexShip-specific destruction logic here

	// Call the base class method
	Ship::DestroyObject();

	// Now call the shutdown method to remove this entity from the simulation
	Shutdown();
	OutputDebugString("Destruction of ComplexShip\n");
}


// Performs a text output of perimeter beacon data for debug purposes
std::string ComplexShip::DebugOutputPerimeterBeacons(void)
{
	CapitalShipPerimeterBeacon *beacon;
	Octree<iObject*> *node;
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
												 (", Offset = ")(Vector3ToString(beacon->BeaconPos))
												 (", Position = ")(Vector3ToString(beacon->GetPosition())).str();

		// Add additional info on the node this beacon sits within, if relevant
		if (node)
			s = concat(s)(", Node (Depth ")(beacon->GetSpatialTreeNode()->DetermineTreeDepth())(") = ")
						 (Vector3ToString(beacon->GetSpatialTreeNode()->m_min))(" to ")
						 (Vector3ToString(beacon->GetSpatialTreeNode()->m_max))("\n").str();
		else
			s = concat(s)(", No spatial positioning node\n").str();
	}

	// Return the complete string
	return s;
}

// Default destructor
ComplexShip::~ComplexShip(void)
{

}

// Custom debug string function
std::string	ComplexShip::DebugString(void) const
{
	return iObject::DebugString(concat("Class=")(Ship::TranslateShipClassToString(m_shipclass))(", Prototype=")(m_prototype != NullString ? m_prototype : "(No prototype)")
		(", Environment=[")(DebugEnvironmentString())("]").str());
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


// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void ComplexShip::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(GetSection, command.ParameterAsInt(2))
	REGISTER_DEBUG_ACCESSOR_FN(GetSectionCount)
	REGISTER_DEBUG_ACCESSOR_FN(GetShipSectionContainingElement, INTVECTOR3(command.ParameterAsInt(2), command.ParameterAsInt(3), command.ParameterAsInt(4)))
	REGISTER_DEBUG_ACCESSOR_FN(GetShipMinimumBounds)
	REGISTER_DEBUG_ACCESSOR_FN(GetShipMaximumBounds)
	REGISTER_DEBUG_ACCESSOR_FN(InteriorShouldAlwaysBeRendered)
	REGISTER_DEBUG_ACCESSOR_FN(DebugOutputPerimeterBeacons)

	// Mutator methods
	REGISTER_DEBUG_FN(BuildHardpointCollection)
	REGISTER_DEBUG_FN(FadeHullToAlpha, command.ParameterAsFloat(2), command.ParameterAsFloat(3), command.ParameterAsBool(4))
	REGISTER_DEBUG_FN(FadeEntireShipToAlpha, command.ParameterAsFloat(2), command.ParameterAsFloat(3), command.ParameterAsBool(4))
	REGISTER_DEBUG_FN(SuspendUpdates)
	REGISTER_DEBUG_FN(ResumeUpdates)
	REGISTER_DEBUG_FN(GenerateCapitalShipPerimeterBeacons)
	REGISTER_DEBUG_FN(SimulateObject)
	REGISTER_DEBUG_FN(RecalculateAllShipData)
	REGISTER_DEBUG_FN(RecalculateShipDataFromCurrentState)
	REGISTER_DEBUG_FN(ForceRenderingOfInterior, command.ParameterAsBool(2))


	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		iSpaceObjectEnvironment::ProcessDebugCommand(command);

}
