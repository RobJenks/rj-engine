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

	// Recalculate any properties of this turret hardpoint based on the mounted equipment (or lack thereof)
	void							RecalculateHardpointData(void);	

	// ID of the turret that this hardpoint is associated with
	CMPINLINE SpaceTurret::TurretID	GetTurretID(void) const { return m_turret_id; }

	// Pitch and yaw limits imposed by the hardpoint
	CMPINLINE bool					IsYawLimited(void) const { return m_yaw_limited; }
	CMPINLINE float					GetMinYawLimit(void) const { return m_yawmin; }
	CMPINLINE float					GetMaxYawLimit(void) const { return m_yawmax; }
	CMPINLINE float					GetMinPitchLimit(void) const { return m_yawmin; }
	CMPINLINE float					GetMaxPitchLimit(void) const { return m_yawmax; }

	// Set or remove the hardpoint yaw limit
	void							SetYawLimit(float yaw_min, float yaw_max);
	CMPINLINE void					RemoveYawLimit(void) { m_yaw_limited = false; }

	// Set the hardpoint pitch limit
	void							SetPitchLimit(float pitch_min, float pitch_max);

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

	// ID of the turret that this hardpoint is associated with (or SpaceTurret::NULL_TURRET if no attachment)
	SpaceTurret::TurretID			m_turret_id;

	// Turret hardpoints may impose spatial restrictions, in addition to those defined by the turret itself
	// In these cases the most restrictive intersection of the constraints is used
	bool							m_yaw_limited;
	float							m_yawmin, m_yawmax;
	float							m_pitchmin, m_pitchmax;


	// Methods to update the hardpoint attachment.  Internal and called by MountEquipment
	void							MountWeapon(Weapon *weapon);	// Mounts a new weapon on this hardpoint
	void							UnmountCurrentWeapon(void);		// Unmount any equipment that we currently have mounted

};


#endif