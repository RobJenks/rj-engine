#pragma once

#ifndef __HpTorpedoLauncherH__
#define __HpTorpedoLauncherH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
class Hardpoint;
class TorpedoLauncher;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class HpTorpedoLauncher : public ALIGN16<HpTorpedoLauncher>, public Hardpoint
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(HpTorpedoLauncher)

	// Returns the type of this hardpoint subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::TorpedoLauncher; }

	void							RecalculateHardpointData(void);	// Recalculates hardpoint stats based on mounted equipment
	
	CMPINLINE TorpedoLauncher*		GetTorpedoLauncher(void);			// Returns a pointer to the currently-mounted missile launcher (if there is one)
	void							MountTorpedoLauncher(TorpedoLauncher* torpedolauncher);		// Mounts a new missile launcher on this hardpoint

	// Virtual override method to clone the subclass when called through the base class
	virtual HpTorpedoLauncher*		Clone() const 
	{
		HpTorpedoLauncher *hp = new HpTorpedoLauncher(*this);		// Clone the hardpoint and all fields
		hp->MountEquipment(NULL);									// Remove any reference to mounted equipment; we only clone the HP, not the equipment on it
		return hp;
	}

	// Virtual override method to delete the subclass when called through the base class
	virtual void					Delete() const { delete this; }
	
	// Virtual override method to mount equipment on this hardpoint, if called from the base class
	virtual void					MountEquipment(Equipment *e);

	// Read hardpoint content in from XML; must be implemented by child classes.  Accepts the hashed item key as a parameter
	// to avoid duplication of effort.  All children should fall back to Hardpoint::ReadBaseHardpointXML if they cannot process an item themselves
	virtual Result					ReadFromXML(TiXmlElement *node, HashVal hashed_key);


	HpTorpedoLauncher(void);
	~HpTorpedoLauncher(void);

private:
	TorpedoLauncher*			m_TorpedoLauncher;				// Pointer to the currently-mounted missile launcher

};

CMPINLINE TorpedoLauncher *HpTorpedoLauncher::GetTorpedoLauncher() { return m_TorpedoLauncher; }


#endif