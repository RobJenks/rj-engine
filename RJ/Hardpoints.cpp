#include <string>
#include <vector>
#include <unordered_map>
#include "Hardpoint.h"
#include "iContainsHardpoints.h"
#include "Equip.h"
#include "HpEngine.h"

#include "Hardpoints.h"


Hardpoints::Hardpoints(void)
{
	// Initialise the HP groups collection with a vector for each potential HP type
	for (int i=0; i<Equip::Class::Z_COUNT; ++i) m_hpgroups.push_back(Hardpoints::HardpointCollection());

	// Initialise other variables with default values
	m_parent = NULL;
	m_hpparent = NULL;
	m_suspendupdates = false;
}

// Resume updates based on hardpoint changes.  Invokes an initial update upon resume
void Hardpoints::ResumeUpdates(void)
{
	m_suspendupdates = false;

	RecalculateHardpoints();
	if (m_hpparent) m_hpparent->HardpointChanged(NULL);
}

// Default destructor
Hardpoints::~Hardpoints(void)
{
	// We do not want to deallocate hardpoint data here, since some objects will be using shared
	// references (e.g. complex ships hold a reference to the hardpoints owned by each of their
	// sections).  Deallocation of hardpoints is the responsibility of the object which owns
	// the hardpoints
}

// Clears all hardpoint data.  Hardpoints themselves are unaffected; this just clears the collections
void Hardpoints::ClearData(void)
{
	// Clear the primary hardpoints vector
	m_items.clear();

	// Clear each of the hardpoint groups
	HardpointGroups::size_type n = m_hpgroups.size();
	for (HardpointGroups::size_type i = 0; i < n; ++i)
		m_hpgroups[i].clear();
}

// Clears all hardpoint data, deallocating all hardpoints held within the collection
void Hardpoints::ShutdownAllHardpoints(void)
{
	// Deallocate each hardpoint item & its contents in turn
	Hardpoints::IndexedHardpointCollection::const_iterator it_end = m_items.end();
	for (Hardpoints::IndexedHardpointCollection::const_iterator it = m_items.begin(); it != it_end; ++it) 
	{
		// Make sure the HP is valid
		Hardpoint *hp = it->second;
		if (!hp) continue;
		
		// If there is equipment mounted on the HP then delete that first
		Equipment *equip = hp->GetEquipment();
		if (equip) { delete equip; }

		// Now delete the hardpoint itself
		hp->Delete();	
	}
	m_items.clear();

	// Clear the hardpoints groups
	for (int i = 0; i < (int)Equip::Class::Z_COUNT; ++i)
	{
		m_hpgroups[i].clear();
	}
	m_hpgroups.clear();
}

void Hardpoints::RecalculateHardpoints(void)
{
	// Make sure we have the correct number of HP groups; if not, rebuild them
	if (m_hpgroups.size() != Equip::Class::Z_COUNT)
	{
		m_hpgroups.clear();
		for (int i=0; i<Equip::Class::Z_COUNT; i++) m_hpgroups.push_back(Hardpoints::HardpointCollection());
	}
	else
	{
		// Otherwise we just need to clear each HP group before repopulating them
		for (int i = 0; i<Equip::Class::Z_COUNT; i++) m_hpgroups[i].clear();
	}

	// Process each hardpoint in turn
	Hardpoints::IndexedHardpointCollection::const_iterator it_end = m_items.end();
	for (Hardpoints::IndexedHardpointCollection::const_iterator it = m_items.begin(); it != it_end; ++it) 
	{
		// Get the type of this hardpoint and make sure it is valid
		if (it->second)
		{
			Equip::Class hptype = it->second->GetType();
			if ((int)hptype >= 0 && (int)hptype < Equip::Class::Z_COUNT)
			{
				// Add to the relevant HP group
				m_hpgroups[(int)hptype].push_back(it->second);
			}
		}
	}
}	

// Clones the contents of another hardpoints collection into this one.  Will create new copies of all
// hardpoints within the collection (rather than shared pointers)
void Hardpoints::Clone(Hardpoints & source)
{
	// Suspend updates before making changes
	SuspendUpdates();

	// Clear our current hardpoints collection, which currently holds references to the hardpoints of the source object
	ClearData();

	// Copy each hardpoint in turn
	Hardpoints::IndexedHardpointCollection::const_iterator it_end = source.GetAllHardpoints().end();
	for (Hardpoints::IndexedHardpointCollection::const_iterator it = source.GetAllHardpoints().begin(); it != it_end; ++it)  
	{
		AddHardpoint(it->second->Clone());
	}

	// Recalculate hardpoint collection data based on the individual hardpoints
	RecalculateHardpoints();

	// Resume updates now that all hardpoint data has been copied
	ResumeUpdates();
}

void Hardpoints::AddHardpoint(Hardpoint *hp)
{
	// Null pointer check
	if (!hp || hp->Code == NullString || m_items.count(hp->Code) > 0) return;		
		
	// Add to the hardpoints collection, and also create the reverse pointer back to the HP's parent ship
	m_items[hp->Code] = hp;				
	hp->SetParent(this);

	// Only perform a broader update if we have not suspended updates
	if (!m_suspendupdates)
	{
		// Recalculate the internal hardpoint data based on this addition
		RecalculateHardpoints();

		// Raise a hardpoint change event to be handled by the parent object
		if (m_hpparent) m_hpparent->HardpointChanged(hp);
	}

}

void Hardpoints::DeleteHardpoint(const std::string & code)
{
	if (code == NullString) return;
	
	// Attempt to locate this hardpoint
	IndexedHardpointCollection::iterator it = m_items.find(code);
	if (it == m_items.end()) return;

	// If it exists, deallocate the hardpoint now
	Hardpoint *hp = (*it).second;
	if (hp) hp->Delete();

	// Remove this entry from the collection
	m_items.erase(it);

	// Only perform a broader update if we have not suspended updates
	if (!m_suspendupdates)
	{
		// Recalculate the internal hardpoint data based on this addition
		RecalculateHardpoints();

		// Raise a hardpoint change event to be handled by the parent object.  Pass no
		// hardpoint, which will initiate a full & hp-type-independent update of the environment
		if (m_hpparent) m_hpparent->HardpointChanged(NULL);
	}
}