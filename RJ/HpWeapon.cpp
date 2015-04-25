#include "Equip.h"
#include "Hardpoint.h"
#include "Weapon.h"

#include "HpWeapon.h"


void HpWeapon::RecalculateHardpointData()
{
}

// Virtual method to allow mounting of class-specific equipment by a call to the base class instance
void HpWeapon::MountEquipment(Equipment *e) 
{ 
	if (!e || (e && e->GetType() == Equip::Class::Weapon)) this->MountWeapon((Weapon*)e); 
}

void HpWeapon::MountWeapon(Weapon *weapon)
{
	// Mount the equipment even if it is NULL; mounting a NULL item is essentially unmounting the equiment
	m_Weapon = weapon;

	// Recalculate hardpoint data
	RecalculateHardpointData();
}


HpWeapon::HpWeapon(void)
{

}

HpWeapon::~HpWeapon(void)
{
}
