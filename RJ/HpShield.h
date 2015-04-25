#pragma once

#ifndef __HpShieldH__
#define __HpShieldH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
class Hardpoint;
class Shield;

class HpShield : public Hardpoint
{
public:
	// Returns the type of this hardpoint subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::Shield; }

	void							RecalculateHardpointData(void);	// Recalculates hardpoint stats based on mounted equipment
	
	CMPINLINE Shield*				GetShield(void);			// Returns a pointer to the currently-mounted missile launcher (if there is one)
	void							MountShield(Shield *shield);		// Mounts a new missile launcher on this hardpoint

	// Virtual override method to clone the subclass when called through the base class
	virtual HpShield*				Clone() const 
	{
		HpShield *hp = new HpShield(*this);			// Clone the hardpoint and all fields
		hp->MountEquipment(NULL);					// Remove any reference to mounted equipment; we only clone the HP, not the equipment on it
		return hp;
	}

	// Virtual override method to delete the subclass when called through the base class
	virtual void					Delete() const { delete this; }

	// Virtual override method to mount equipment on this hardpoint, if called from the base class
	virtual void					MountEquipment(Equipment *e);

	HpShield(void);
	~HpShield(void);

private:
	Shield*			m_Shield;				// Pointer to the currently-mounted missile launcher

};

CMPINLINE Shield *HpShield::GetShield() { return m_Shield; }


#endif