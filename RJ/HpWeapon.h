#pragma once

#ifndef __HpWeaponH__
#define __HpWeaponH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
class Hardpoint;
class Weapon;


class HpWeapon : public Hardpoint
{
public:
	// Returns the type of this hardpoint subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::Weapon; }

	void							RecalculateHardpointData(void);	// Recalculates hardpoint stats based on mounted equipment
	
	CMPINLINE Weapon*				GetWeapon(void);				// Returns a pointer to the currently-mounted missile launcher (if there is one)
	void							MountWeapon(Weapon *weapon);	// Mounts a new missile launcher on this hardpoint

	// Virtual override method to clone the subclass when called through the base class
	virtual HpWeapon*				Clone() const 
	{
		HpWeapon *hp = new HpWeapon(*this);				// Clone the hardpoint and all fields
		hp->MountEquipment(NULL);						// Remove any reference to mounted equipment; we only clone the HP, not the equipment on it
		return hp;
	}

	// Virtual override method to delete the subclass when called through the base class
	virtual void					Delete() const { delete this; }
	
	// Virtual override method to mount equipment on this hardpoint, if called from the base class
	virtual void					MountEquipment(Equipment *e);

	HpWeapon(void);
	~HpWeapon(void);

private:
	Weapon *			m_Weapon;				// Pointer to the currently-mounted missile launcher

};

CMPINLINE Weapon *HpWeapon::GetWeapon() { return m_Weapon; }


#endif