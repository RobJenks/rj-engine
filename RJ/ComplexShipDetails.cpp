/* *** NO LONGER IN USE *** 

#include "time.h"
#include "Utility.h"
#include "GameDataExtern.h"
#include "ComplexShipSectionDetails.h"
#include "ComplexShipElement.h"
#include "iContainsComplexShipTiles.h"
#include "ComplexShipTile.h"
#include "HardpointsInterface.h"
#include "NavNetwork.h"

#include "ComplexShipDetails.h"

ComplexShipDetails::ComplexShipDetails(void)
{
	m_code = "";
	m_elements = NULL;
	m_elementsize = INTVECTOR3(0, 0, 0);
	m_hardpoints = new HardpointsInterface();
	m_navnetwork = NULL;

	m_tilecount = 0;
	m_hastiles = m_multitile = false;

	m_sdoffset = INTVECTOR3(0, 0, 0);
	m_directlygeneratedfromSD = false;
}


ComplexShipSection *ComplexShipDetails::GetSection(int index)
{
	// Perform bounds checking before we attempt to return the ship section
	if (index < 0 || index >= m_sections.size()) return NULL;

	// Return the section at the specified index
	return m_sections.at(index);
}

Result ComplexShipDetails::AddShipSection(ComplexShipSection *section)
{
	// Validate and then add this section to the collection
	if (!section || !section->GetDetails()) return ErrorCodes::CannotAddInvalidShipSectionToShip;
	m_sections.push_back(section);

	// Set the pointer from the section details back to its parent ship object
	section->GetDetails()->SetParent(this);

	// Also propogate these section elements into the main ship; any ship elements (i.e. below) are a delta on the standard section data
	Result result = this->ApplyComplexShipSectionToShip(section);
	if (result != ErrorCodes::NoError) return result;
	
	// Link this section to our ship hardpoints collection and (automatically) rebuild the interface
	if (section->GetDetails()->GetHardpoints()) m_hardpoints->AddHardpointsLink(section->GetDetails()->GetHardpoints());

	// Return success now that the section is successfully integrated into the ship
	return ErrorCodes::NoError;
}

void ComplexShipDetails::RemoveShipSection(ComplexShipSection *section)
{
	// Sanity check; we need a valid section in order to remove it
	if (!section) return;

	// Loop through the collection and attempt to locate this item
	int n = m_sections.size();
	for (int i = 0; i < n; i++)
	{
		if (m_sections.at(i) == section)
		{
			// Unlink this section from the ship hardpoints collection and (automatically) rebuild the interface
			if (section->GetDetails() && section->GetDetails()->GetHardpoints()) 
				m_hardpoints->RemoveHardpointsLink(section->GetDetails()->GetHardpoints());

			// Swap-and-pop this element to efficiently remove it from the collection
			std::swap(m_sections.at(i), m_sections.at(n-1));
			m_sections.pop_back();

			// Clear the pointer from this section back to its parent ship object
			section->GetDetails()->SetParent(NULL);

			// End the method here if we have removed the specified item
			return;
		}
	}
}

void ComplexShipDetails::RemoveShipSection(int index)
{
	// Perform bounds checking before we attempt to remove the ship section
	if (index < 0 || index >= m_sections.size()) return;

	// Unlink this section from the ship hardpoints collection and (automatically) rebuild the interface
	if (m_sections.at(index)->GetDetails() && m_sections.at(index)->GetDetails()->GetHardpoints()) 
		m_hardpoints->RemoveHardpointsLink(m_sections.at(index)->GetDetails()->GetHardpoints());

	// If this item isn't already the last section in the collection then swap to the end now
	if (index != (m_sections.size()-1))
		std::swap(m_sections.at(index), m_sections.at(m_sections.size()-1));

	// Set the pointer from this section back to its parent ship to NULL
	m_sections.at(m_sections.size()-1)->GetDetails()->SetParent(NULL);

	// Now that this element is at the end, pop it from the collection to remove
	m_sections.pop_back();
}

// Method to handle the addition of a ship tile to this object
void ComplexShipDetails::ShipTileAdded(ComplexShipTile *tile)
{
	// TODO: To be implemented
}

// Method to handle the removal of a ship tile from this object
void ComplexShipDetails::ShipTileRemoved(ComplexShipTile *tile)
{
	// TODO: To be implemented
}

// Method triggered when the layout (e.g. active/walkable state, connectivity) of elements is changed
void ComplexShipDetails::ElementLayoutChanged(void)
{
	// We want to update the ship navigation network if the element layout has changed.  
	UpdateNavigationNetwork();
}




// Fits the element space around this ship, eliminating any extra space allocated outside of the (cuboid) bounds it requires
Result ComplexShipDetails::FitElementSpaceToShip(void)
{
	// Retrieve the maximum bounds occupied by the sections of this ship
	INTVECTOR3 bounds = GetShipMaximumBounds();

	// Now request a reallocation to this maximum extent; if there is no change to be made it will simply return
	return ReallocateElementSpace(bounds);
}

// Returns the maximum bounds occupied by this ship.  May not necessarily == elementsize when sections are being moved around
INTVECTOR3 ComplexShipDetails::GetShipMaximumBounds(void)
{
	INTVECTOR3 bounds = INTVECTOR3(0, 0, 0);

	// Iterate over the collection of ship sections to determine the maximum bounds we are using
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Get a reference to this ship section
		ComplexShipSection *sec = (*it);						if (!sec) continue;
		ComplexShipSectionDetails *details = sec->GetDetails();	if (!details) continue;

		// Determine the location of the far extent of this section
		INTVECTOR3 size = sec->GetElementLocation();
		size = INTVECTOR3(	size.x + details->GetElementSizeX() - 1, size.y + details->GetElementSizeY() - 1, 
							size.z + details->GetElementSizeZ() - 1);

		// Store the extent of this section if it extends beyond the current bounds in any dimension
		if (size.x > bounds.x) bounds.x = size.x;
		if (size.y > bounds.y) bounds.y = size.y;
		if (size.z > bounds.z) bounds.z = size.z;
	}

	// Return the maximum bounds
	return bounds;
}

// Returns the maximum bounds occupied by this ship.  May not necessarily == elementsize when sections are being moved around
INTVECTOR3 ComplexShipDetails::GetShipMinimumBounds(void)
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
		ComplexShipSection *sec = (*it);						if (!sec) continue;
		ComplexShipSectionDetails *details = sec->GetDetails();	if (!details) continue;

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

// Loads the contents of a ship section element space into the ship itself.  Any subsequent ship element definitions are a delta on this standard data
Result ComplexShipDetails::ApplyComplexShipSectionToShip(ComplexShipSection *section)
{
	ComplexShipElement *src, *dest;

	// Parameter check 
	if (!section) return ErrorCodes::CannotLoadSectionElementsIntoShipWithNullData;
	ComplexShipSectionDetails *details = section->GetDetails();
	if (!details) return ErrorCodes::CannotLoadSectionElementsIntoShipWithNullData;

	// Get the size and location of this ship section
	INTVECTOR3 size = section->GetDetails()->GetElementSize();
	INTVECTOR3 location = section->GetElementLocation();

	// Loop over the section element space
	for (int x = 0; x < size.x; x++) {
		for (int y = 0; y < size.y; y++) {
			for (int z = 0; z < size.z; z++) 
			{
				// Attempt to retrieve the corresponding element from the ship
				dest = this->GetElement(x + location.x, y + location.y, z + location.z);
				if (!dest) continue;

				// Make sure we also have a valid element within the ship section
				src = details->GetElement(x, y, z);
				if (!src) continue;

				// Both the source and target elements exist, so copy data across from the section to the ship
				ComplexShipElement::CopyData(src, dest);
			}
		}
	}

	// Return success
	return ErrorCodes::NoError;
}

// Returns a reference to the ship section that contains the element at this location
ComplexShipSection *ComplexShipDetails::GetShipSectionContainingElement(INTVECTOR3 location)
{
	// Iterate through the ship section collection
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Get a reference to this section
		ComplexShipSection *sec = (*it);
		if (!sec || !sec->GetDetails()) continue;

		// Determine the bounds of this section in ship space
		INTVECTOR3 lbound = sec->GetElementLocation();
		INTVECTOR3 ubound = sec->GetDetails()->GetElementSize();
		ubound.x += (lbound.x - 1); ubound.y += (lbound.y - 1); ubound.z += (lbound.z - 1); 

		// If the specified location falls within this ship section then return a reference to it
		if (location.x >= lbound.x && location.y >= lbound.y && location.z >= lbound.z &&
			location.x <= ubound.x && location.y <= ubound.y && location.z <= ubound.z) 
				return sec;
		
	}

	// If no section contains this element then return NULL
	return NULL;
}

// Returns a reference to the SHIP SECTION element at this location in the ship blueprint.  Contrast with GetElement(...)
ComplexShipElement *ComplexShipDetails::GetShipSectionElement(INTVECTOR3 location)
{
	// Iterate through the ship section collection
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Determine the bounds of this section in ship space
		if (!(*it)) continue;
		INTVECTOR3 lbound = (*it)->GetElementLocation();
		INTVECTOR3 ubound = (*it)->GetDetails()->GetElementSize();
		ubound.x += (lbound.x - 1); ubound.y += (lbound.y - 1); ubound.z += (lbound.z - 1); 

		// If the specified location falls within this ship section then return a reference to the SECTION element
		if (location.x >= lbound.x && location.y >= lbound.y && location.z >= lbound.z &&
			location.x <= ubound.x && location.y <= ubound.y && location.z <= ubound.z) 
				return (*it)->GetDetails()->GetElement(location.x - lbound.x, location.y - lbound.y, location.z - lbound.z);
		
	}

	// If no section contains this element then return NULL
	return NULL;
}

// Returns a reference to the SHIP SECTION element at this location in the ship blueprint.  Contrast with GetElement(...)
ComplexShipElement *ComplexShipDetails::GetShipSectionElement(int x, int y, int z)
{
	// Iterate through the ship section collection
	ComplexShipSectionCollection::const_iterator it_end = m_sections.end();
	for (ComplexShipSectionCollection::const_iterator it = m_sections.begin(); it != it_end; ++it)
	{
		// Determine the bounds of this section in ship space
		if (!(*it)) continue;
		INTVECTOR3 lbound = (*it)->GetElementLocation();
		INTVECTOR3 ubound = (*it)->GetDetails()->GetElementSize();
		ubound.x += (lbound.x - 1); ubound.y += (lbound.y - 1); ubound.z += (lbound.z - 1); 

		// If the specified location falls within this ship section then return a reference to the SECTION element
		if (x >= lbound.x && y >= lbound.y && z >= lbound.z &&
			x <= ubound.x && y <= ubound.y && z <= ubound.z) 
				return (*it)->GetDetails()->GetElement(x - lbound.x, y - lbound.y, z - lbound.z);
		
	}

	// If no section contains this element then return NULL
	return NULL;
}

// Returns the file path where XML data relating to this ship should be stored
string ComplexShipDetails::DetermineXMLDataPath(void)
{
	return concat("\\Ships\\")(m_code).str();
}
// Returns the name of the XML file to be generated for this ship
string ComplexShipDetails::DetermineXMLDataFilename(void)
{
	return concat(m_code)(".xml").str();
}
// Returns the full expected filename for the XML data relating to this ship
string ComplexShipDetails::DetermineXMLDataFullFilename(void)
{
	return concat(DetermineXMLDataPath())("\\")(DetermineXMLDataFilename()).str();
}

// Static method that attempts to load the set of details specified by the string code parameter
ComplexShipDetails *ComplexShipDetails::Get(string code)
{
	// If the code is invalid or does not exist then return a NULL pointer
	if (code == "" || D::ComplexShips.count(code) == 0) return NULL;

	// Otherwise return the relevant ship section details now
	return D::ComplexShips[code];
}

ComplexShipDetails *ComplexShipDetails::Copy(ComplexShipDetails *ship)
{
	// Create a new ship details object to act as the clone, using the copy
	// constructor for basic fields that will simply be copied across
	ComplexShipDetails *s = new ComplexShipDetails(*ship);

	// Make a copy of the hardpoints collection, and the ship hardpoints interface
	s->HP = ship->HP->Copy();
	s->ReplaceShipHardpoints(ship->GetShipHardpoints());

	// Clear the key component pointers that have been copied across from the source ship; they will be recreated manually
	s->GetSections()->clear();
	s->RemoveAllShipTiles();
	s->SetElements(NULL);
	s->GetShipHardpoints()->ClearHardpointsLinks();

	// Copy the size of the elements collection
	INTVECTOR3 esize = ship->GetElementSize();
	s->SetElementSize(esize);

	// Allocate element storage in the new ship
	s->InitialiseAllElements();

	// Copy each ship section in turn
	ComplexShipDetails::ComplexShipSectionCollection::const_iterator s_it_end = ship->GetSections()->end();
	for (ComplexShipDetails::ComplexShipSectionCollection::const_iterator s_it = ship->GetSections()->begin(); s_it != s_it_end; ++s_it)
	{
		// Get a reference to this section and make a copy
		ComplexShipSection *section = ComplexShipSection::Copy(*s_it);

		// Assign the section a new space object ID since it will otherwise inherit from the template section
		section->GenerateSpaceObjectDetails();

		// Add this cloned section to the new ship
		if (section) s->AddShipSection(section);
	}

	// Now copy element data across from the source ship
	for (int x = 0; x < esize.x; x++) {
		for (int y = 0; y < esize.y; y++) {
			for (int z = 0; z < esize.z; z++)
			{
				// Get a reference to the target element in the new ship
				ComplexShipElement *e = s->GetElementDirect(x, y, z);

				// Copy element data across from the source element
				ComplexShipElement::CopyData(ship->GetElementDirect(x, y, z), e);
			}
		}
	}

	// Copy and link each complex ship tile in turn
	iContainsComplexShipTiles::ConstTileIterator t_it_end = ship->GetTileIteratorEnd();
	for (iContainsComplexShipTiles::ConstTileIterator t_it = ship->GetTileIteratorStart(); t_it != t_it_end; ++t_it)
	{
		// Get a reference to this tile and make a copy via virtual subclass copy method
		ComplexShipTile *tile = (*t_it)->Copy();

		// Link this cloned tile to the new ship
		if (tile) tile->LinkToParent(s);
	}
	
	// Remove the 'standard' flag from this ship following the copy
	s->IsStandardShip = false;

	// Return the cloned ship details
	return s;
}

// Replaces the hardpoints interface for this ship with a new one, 
void ComplexShipDetails::ReplaceShipHardpoints(HardpointsInterface *hp)
{
	// Remove any existing hardpoints interface, if one exists
	if (m_hardpoints) { m_hardpoints->Shutdown(); SafeDelete(m_hardpoints); }

	// Create a new hardpoints interface for this ship
	m_hardpoints = new HardpointsInterface();
	
	// Replicate each hardpoints link from the template collection
	int n = hp->GetHardpointsLinkCount();
	for (int i = 0; i < n; i++)
	{
		// Get a reference to the hardpoints collection
		Hardpoints *h = hp->GetHardpointsLink(i);
		if (!h) continue;

		// Add a link to this collection, which will automatically recalculate hardpoints data for the ship
		m_hardpoints->AddHardpointsLink(h);
	}
}

// Updates the ship navigation network based on the set of elements and their properties
void ComplexShipDetails::UpdateNavigationNetwork(void)
{
	// Make sure the network exists.  If it doesn't, create the network object first
	if (!m_navnetwork) m_navnetwork = new NavNetwork();
	
	// Initialise the nav network with data from this complex ship
	m_navnetwork->InitialiseNavNetwork((iContainsComplexShipElements*)this);

	// TODO: Find any actors currently following a path provided by the previous network, and have them recalculate their paths
}

// Destructor
ComplexShipDetails::~ComplexShipDetails(void)
{
}

// Shutdown method; parameter allows us to specify whether the ship sections will also be deallocated
void ComplexShipDetails::Shutdown(bool ShutdownSections, bool UnlinkTiles, bool IncludeStandardSections)
{
	// Detach and deallocate the navigation network assigned to this ship
	if (m_navnetwork) 
	{
		m_navnetwork->Shutdown();
		SafeDelete(m_navnetwork);
	}

	// Shutdown the hardpoints interface, which will unlink it from all component ship sections
	if (m_hardpoints)
	{
		m_hardpoints->Shutdown();
		SafeDelete(m_hardpoints);
	}

	// Deallocate all tiles assigned to this ship; only needs to be performed at the ship level, and section/element are handled at the same time
	ShutdownAllTileData(true, UnlinkTiles);

	// Deallocate all element storage assigned to the ship itself
	if (m_elements) ComplexShipElement::DeallocateElementStorage(&m_elements, m_elementsize);

	// Also deallocate each ship section in turn, if required
	if (ShutdownSections)
	{
		ComplexShipSectionCollection::iterator it_end = m_sections.end();
		for (ComplexShipSectionCollection::iterator it = m_sections.begin(); it != it_end; ++it)
		{
			// Get a handle to the section and make sure it is valid
			ComplexShipSection *sec = (*it);
			if (!sec) continue;

			// Call the section shutdown method; parameter determines whether we will shutdown its contents (the details object)
			sec->Shutdown(IncludeStandardSections);

			// Delete the section
			delete sec;
		}
	}

	// Clear the ship section collection to complete deallocation of the objects
	m_sections.clear();

	// Pass control to the base class to shut down all common ship details data
	ShipDetails::Shutdown();
}

// Default execution for the shutdown method.  Calls the overloaded method with default parameter, i.e. do not 
// deallocate sections/tiles or standard section details by default on ship shutdown
void ComplexShipDetails::Shutdown(void) { Shutdown(false, false, false); }


*/