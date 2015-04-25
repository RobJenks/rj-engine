#include "Equip.h"
#include "Hardpoint.h"
#include "Shield.h"

#include "HpShield.h"


void HpShield::RecalculateHardpointData()
{
}

// Virtual method to allow mounting of class-specific equipment by a call to the base class instance
void HpShield::MountEquipment(Equipment *e) 
{ 
	if (!e || (e && e->GetType() == Equip::Class::Shield)) this->MountShield((Shield*)e); 
}

void HpShield::MountShield(Shield *shield)
{
	// Mount the equipment even if it is NULL; mounting a NULL item is essentially unmounting the equiment
	m_Shield = shield;

	// Recalculate hardpoint data
	RecalculateHardpointData();
}


HpShield::HpShield(void)
{

}

HpShield::~HpShield(void)
{
}
