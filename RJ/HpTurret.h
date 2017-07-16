#pragma once

#ifndef __HpTurretH__
#define __HpTurretH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
class Hardpoint;
class Weapon;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class HpTurret : public ALIGN16<HpTurret>, public Hardpoint
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(HpTurret)

	// Returns the type of this hardpoint subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::Turret; }

	void							RecalculateHardpointData(void);	// Recalculates hardpoint stats based on mounted equipment
	
	CMPINLINE Weapon*				GetWeapon(void);				// Returns a pointer to the currently-mounted missile launcher (if there is one)
	void							MountWeapon(Weapon *weapon);	// Mounts a new missile launcher on this hardpoint

	// Virtual override method to clone the subclass when called through the base class
	virtual HpTurret *				Clone() const 
	{
		HpTurret *hp = new HpTurret(*this);				// Clone the hardpoint and all fields
		hp->MountEquipment(NULL);						// Remove any reference to mounted equipment; we only clone the HP, not the equipment on it
		return hp;
	}

	// Virtual override method to delete the subclass when called through the base class
	virtual void					Delete() const { delete this; }
	
	// Virtual override method to mount equipment on this hardpoint, if called from the base class
	virtual void					MountEquipment(Equipment *e);

	// Read hardpoint content in from XML; must be implemented by child classes.  Accepts the hashed item key as a parameter
	// to avoid duplication of effort.  All children should fall back to Hardpoint::ReadBaseHardpointXML if they cannot process an item themselves
	virtual Result					ReadFromXML(TiXmlElement *node, HashVal hashed_key);


	HpTurret(void);
	~HpTurret(void);

private:
	Weapon *			m_Weapon;				// Pointer to the currently-mounted missile launcher

};

CMPINLINE Weapon *HpTurret::GetWeapon() { return m_Weapon; }


#endif