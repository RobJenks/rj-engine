#include <cstdlib>
#include <unordered_map>
#include <vector>
#include "DX11_Core.h"
using namespace std;
using namespace std::tr1;

#include "iHardpoints.h"
#include "Equip.h"
#include "Hardpoint.h"
#include "HpEngine.h"

#include "Hardpoints.h"


Hardpoints::Hardpoints(void)
{
	// Initialise the HP groups collection with a vector for each potential HP type
	for (int i=0; i<Equip::Class::Z_COUNT; i++) m_hpgroups.push_back(iHardpoints::HardpointCollection());

	// Initialise other variables with default values
	m_parent = NULL;
	m_hp_parent = NULL;
}

Hardpoints::~Hardpoints(void)
{
	Hardpoint *hp = NULL;

	// Deallocate each hardpoint item & its contents in turn
	iHardpoints::IndexedHardpointCollection::const_iterator it_end = m_items.end();
	for (iHardpoints::IndexedHardpointCollection::const_iterator it = m_items.begin(); it != it_end; ++it) 
	{
		// Make sure the HP is valid
		Hardpoint *hp = it->second;
		if (!hp) continue;
		
		// If there is equipment mounted on the HP then delete that first
		Equipment *equip = hp->GetEquipment();
		if (equip) { delete equip; }

		// Now delete the hardpoint itself
		it->second->Delete();	
	}
	m_items.clear();

	// Clear the hardpoints groups
	for (int i=0; i<(int)Equip::Class::Z_COUNT; i++)
		m_hpgroups[i].clear();
}

void Hardpoints::RecalculateHardpoints(void)
{
	// Make sure we have the correct number of HP groups; if not, rebuild them
	if (m_hpgroups.size() != Equip::Class::Z_COUNT)
	{
		m_hpgroups.clear();
		for (int i=0; i<Equip::Class::Z_COUNT; i++) m_hpgroups.push_back(iHardpoints::HardpointCollection());
	}

	// Clear each HP group
	for (int i=0; i<Equip::Class::Z_COUNT; i++) m_hpgroups[i].clear();

	// Process each hardpoint in turn
	iHardpoints::IndexedHardpointCollection::const_iterator it_end = m_items.end();
	for (iHardpoints::IndexedHardpointCollection::const_iterator it = m_items.begin(); it != it_end; ++it) 
	{
		// Get the type of this hardpoint and make sure it is valid
		Equip::Class hptype = it->second->GetType();
		if ((int)hptype >= 0 && (int)hptype < Equip::Class::Z_COUNT)
		{
			// Add to the relevant HP group
			m_hpgroups[(int)hptype].push_back(it->second);
		}
	}
}	

Hardpoints *Hardpoints::Copy(void)
{
	// Copy each hardpoint in turn
	Hardpoints *hh = new Hardpoints();
	iHardpoints::IndexedHardpointCollection::const_iterator it_end = m_items.end();
	for (iHardpoints::IndexedHardpointCollection::const_iterator it = m_items.begin(); it != it_end; ++it)  {
		hh->AddHardpoint(it->second->Clone());
	}

	// Recalculate hardpoint collection data based on the individual hardpoints
	hh->RecalculateHardpoints();

	// Clear the reference to this collection's parent object
	hh->SetParent(NULL, NULL);

	// Return the hardpoint collection
	return hh;
}

void Hardpoints::AddHardpoint(Hardpoint *hp)
{
	// Null pointer check
	if (!hp || hp->Code == NullString || m_items.count(hp->Code) > 0) return;		
		
	// Add to the hardpoints collection, and also create the reverse pointer back to the HP's parent ship
	m_items[hp->Code] = hp;				
	hp->m_parent = this;

	// Perform any required HP-type-specific update to the parent object based on this Hardpoint hp we just added
	if (this->m_hp_parent) this->m_hp_parent->PerformHardpointChangeRefresh(hp);
}