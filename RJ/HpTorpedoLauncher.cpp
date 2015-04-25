#include "Equip.h"
#include "Hardpoint.h"
#include "TorpedoLauncher.h"

#include "HpTorpedoLauncher.h"


void HpTorpedoLauncher::RecalculateHardpointData()
{
}

// Virtual method to allow mounting of class-specific equipment by a call to the base class instance
void HpTorpedoLauncher::MountEquipment(Equipment *e)  
{ 
	if (!e || (e && e->GetType() == Equip::Class::TorpedoLauncher)) this->MountTorpedoLauncher((TorpedoLauncher*)e); 
}

void HpTorpedoLauncher::MountTorpedoLauncher(TorpedoLauncher *torpedoLauncher)
{
	// Mount the equipment even if it is NULL; mounting a NULL item is essentially unmounting the equiment
	m_TorpedoLauncher = torpedoLauncher;

	// Recalculate hardpoint data
	RecalculateHardpointData();
}


HpTorpedoLauncher::HpTorpedoLauncher(void)
{

}

HpTorpedoLauncher::~HpTorpedoLauncher(void)
{
}
