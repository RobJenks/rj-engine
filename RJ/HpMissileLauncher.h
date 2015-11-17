#pragma once

#ifndef __HpMissileLauncherH__
#define __HpMissileLauncherH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
class Hardpoint;
class CoreEngine;
class MissileLauncher;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class HpMissileLauncher : public ALIGN16<HpMissileLauncher>, public Hardpoint
{
public:
	// Returns the type of this hardpoint subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::MissileLauncher; }

	void							RecalculateHardpointData(void);		// Recalculates hardpoint stats based on mounted equipment
	
	CMPINLINE MissileLauncher*		GetMissileLauncher(void);			// Returns a pointer to the currently-mounted missile launcher (if there is one)
	void							MountMissileLauncher(MissileLauncher* missile);		// Mounts a new missile launcher on this hardpoint

	// Virtual override method to clone the subclass when called through the base class
	// Virtual override method to clone the subclass when called through the base class
	virtual HpMissileLauncher *		Clone() const
	{
		HpMissileLauncher *hp = new HpMissileLauncher(*this);		// Clone the hardpoint and all fields
		hp->MountEquipment(NULL);									// Remove any reference to mounted equipment; we only clone the HP, not the equipment on it
		return hp;
	}

	// Virtual override method to delete the subclass when called through the base class
	virtual void					Delete() const { delete this; }

	// Virtual override method to mount equipment on this hardpoint, if called from the base class
	virtual void					MountEquipment(Equipment *e);

	HpMissileLauncher(void);
	~HpMissileLauncher(void);

private:
	MissileLauncher*			m_MissileLauncher;				// Pointer to the currently-mounted missile launcher

};

CMPINLINE MissileLauncher *HpMissileLauncher::GetMissileLauncher() { return m_MissileLauncher; }


#endif