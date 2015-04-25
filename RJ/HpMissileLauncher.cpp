#include "Equip.h"
#include "Hardpoint.h"
#include "MissileLauncher.h"
#include "CoreEngine.h"

#include "HpMissileLauncher.h"

void HpMissileLauncher::RecalculateHardpointData()
{
}

// Virtual method to allow mounting of class-specific equipment by a call to the base class instance
void HpMissileLauncher::MountEquipment(Equipment *e) 
{ 
	if (!e || (e && e->GetType() == Equip::Class::MissileLauncher)) this->MountMissileLauncher((MissileLauncher*)e); 
}

void HpMissileLauncher::MountMissileLauncher(MissileLauncher *missile)
{
	// Mount the equipment even if it is NULL; mounting a NULL item is essentially unmounting the equiment
	m_MissileLauncher = missile;

	// Recalculate hardpoint data
	RecalculateHardpointData();
}


HpMissileLauncher::HpMissileLauncher(void)
{

}

HpMissileLauncher::~HpMissileLauncher(void)
{
}
