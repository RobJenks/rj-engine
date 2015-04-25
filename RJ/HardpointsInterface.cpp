#include "Utility.h"
#include "Equip.h"
#include "Hardpoints.h"

#include "HardpointsInterface.h"

// Constructor
HardpointsInterface::HardpointsInterface(void)
{
	// Initialise the interface ready for operation
	InitialiseHardpointsInterface();
}

// Initialises the interface on startup to be ready for operation
void HardpointsInterface::InitialiseHardpointsInterface(void)
{
	// Clear the collection of hardpoints links
	m_hplinks.clear();

	// Clear the collection of all hardpoints
	m_hardpoints.clear();

	// First, make sure we don't already have HP groups assigned. Loop through any that exist and clear them, then clear the collection
	for (int i=0; i<(int)m_hpgroups.size(); i++) m_hpgroups.at(i).clear();
	m_hpgroups.clear();

	// Now we want to initialise the HP groups collection with a vector for each potential HP type
	for (int i=0; i<Equip::Class::Z_COUNT; i++) m_hpgroups.push_back(iHardpoints::HardpointCollection());
}


// Method which rebuilds the interface to hardpoint collections based on the set of hardpoints links
void HardpointsInterface::RebuildHardpointsInterface(void)
{
	// Sanity check: make sure we have vectors allocated for all HP groups.  If not, reinitialise the whole interface to be sure
	if (m_hpgroups.size() != Equip::Class::Z_COUNT) InitialiseHardpointsInterface();

	// Clear the interface collections to HP groups and individual hardpoints, before we reload them
	for (int i = 0; i < Equip::Class::Z_COUNT; i++)		m_hpgroups.at(i).clear();
	m_hardpoints.clear();

	// We want to process each HP link in turn
	HardpointsLinkCollection::const_iterator it_end = m_hplinks.end();
	for (HardpointsLinkCollection::const_iterator it = m_hplinks.begin(); it != it_end; ++it)
	{
		// Get a reference to this hardpoints link and make sure it is valid
		Hardpoints *hps = (*it);
		if (!hps) continue;

		// Get a reference to the full hardpoint collection 
		iHardpoints::IndexedHardpointCollection *items = hps->GetAllHardpoints();

		// Process each hardpoint in turn and assign it to the correct group
		iHardpoints::IndexedHardpointCollection::const_iterator it_end = items->end();
		for (iHardpoints::IndexedHardpointCollection::const_iterator it = items->begin(); it != it_end; ++it)
		{
			// Get a reference to the hardpoint at this index
			Hardpoint *hp = (it->second);

			// Make sure the hardpoint type is valid
			int type = (int)hp->GetType();
			if (type < 0 || type >= (int)Equip::Class::Z_COUNT) continue;

			// Add to the collection of all hardpoints, assuming we do not already have an item with this unique ID
			if (hp->Code == NullString || m_hardpoints.count(hp->Code) > 0) continue;
			m_hardpoints[hp->Code] = hp;

			// Also add this HP to the relevant group
			m_hpgroups[type].push_back(hp);
		}
	}
}

// Adds a link to a new hardpoints interface
void HardpointsInterface::AddHardpointsLink(Hardpoints *hardpoints)
{
	// Make sure we don't already have a link to this hardpoints collection, and that it is valid
	if (!hardpoints || HasHardpointsLink(hardpoints)) return;

	// Add to the collection
	m_hplinks.push_back(hardpoints);

	// Now rebuild the interface to account for these new hardpoints
	RebuildHardpointsInterface();
}

// Locates a hardpoints link in the collection, if one exists
int HardpointsInterface::FindHardpointsLink(Hardpoints *hardpoints)
{
	return (FindInVector<Hardpoints*>(m_hplinks, hardpoints));
}

// Indicates whether a given hardpoints collection is already linked to this interface
bool HardpointsInterface::HasHardpointsLink(Hardpoints *hardpoints)
{
	return (FindHardpointsLink(hardpoints) > -1);
}

// Removes a hardpoints link from this interface, if one exists
void HardpointsInterface::RemoveHardpointsLink(Hardpoints *hardpoints)
{
	// Remove this hardpoints link from the collection
	RemoveFromVector<Hardpoints*>(m_hplinks, hardpoints);

	// Rebuild the interface to account for the removal of hardpoints
	RebuildHardpointsInterface();
}

// Removes a hardpoints link from this interface, based on its current index
void HardpointsInterface::RemoveHardpointsLink(int index)
{
	// Remove from the vector at the specified index
	RemoveFromVectorAtIndex<Hardpoints*>(m_hplinks, index);

	// Rebuild the interface to account for the removal of hardpoints
	RebuildHardpointsInterface();
}

// Clears the entire set of hardpoints links
void HardpointsInterface::ClearHardpointsLinks(void)
{
	// Clear the collection of hardpoints links
	m_hplinks.clear();

	// Rebuild the interface to initialise back to starting state
	RebuildHardpointsInterface();
}

// Returns a hardpoints link at the specified index
Hardpoints * HardpointsInterface::GetHardpointsLink(int index)
{
	// Make sure the supplied index is valid
	if (index < 0 || index >= (int)m_hplinks.size()) return NULL;

	// It is, so return the hardpoint links collection
	return m_hplinks.at(index);
}

// Shutdown method which clears all the collections (without deallocating, since this just an interface)
void HardpointsInterface::Shutdown(void)
{
	// Clear all links to hardpoint collections; this will then automatically rebuild and clear the dependent collections
	ClearHardpointsLinks();
}

// Destructor
HardpointsInterface::~HardpointsInterface(void)
{
}
