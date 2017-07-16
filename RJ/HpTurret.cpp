#include "Equip.h"
#include "Hardpoint.h"
#include "Weapon.h"

#include "HpTurret.h"


HpTurret::HpTurret(void)
	:
	m_yaw_limited(false), m_yawmin(-0.15f), m_yawmax(+0.15f), m_pitchmin(-0.15f), m_pitchmax(+0.15f)
{
}

void HpTurret::RecalculateHardpointData()
{
}

// Virtual method to allow mounting of class-specific equipment by a call to the base class instance
void HpTurret::MountEquipment(Equipment *e) 
{ 
	if (!e || (e && e->GetType() == Equip::Class::Turret)) this->MountWeapon((Weapon*)e); 
}

void HpTurret::MountWeapon(Weapon *weapon)
{
	// Mount the equipment even if it is NULL; mounting a NULL item is essentially unmounting the equiment
	m_Weapon = weapon;

	// Recalculate hardpoint data
	RecalculateHardpointData();
}

// Set the hardpoint yaw limit
void HpTurret::SetYawLimit(float yaw_min, float yaw_max)
{
	m_yaw_limited = true;
	if (yaw_min > yaw_max) std::swap(yaw_min, yaw_max);
	m_yawmin = yaw_min;
	m_yawmax = yaw_max;
}

// Set the hardpoint pitch limit
void HpTurret::SetPitchLimit(float pitch_min, float pitch_max)
{
	if (pitch_min > pitch_max) std::swap(pitch_min, pitch_max);
	m_pitchmin = pitch_min;
	m_pitchmax = pitch_max;
}

// Read hardpoint content in from XML; must be implemented by child classes.  Accepts the hashed item key as a parameter
// to avoid duplication of effort.  All children should fall back to Hardpoint::ReadBaseHardpointXML if they cannot process an item themselves
Result HpTurret::ReadFromXML(TiXmlElement *node, HashVal hashed_key)
{
	if (!node) return ErrorCodes::CannotLoadHardpointDataFromNullResources;

	// Process any class-specific properties
	if (hashed_key == HashedStrings::H_YawLimit) {
		const char *cmin = node->Attribute("min");
		const char *cmax = node->Attribute("max");
		if (!cmin && cmax) return ErrorCodes::HardpointElementMissingRequiredAttributes;
		
		SetYawLimit(atof(cmin), atof(cmax));
	}
	else if (hashed_key == HashedStrings::H_PitchLimit) {
		const char *cmin = node->Attribute("min");
		const char *cmax = node->Attribute("max");
		if (!cmin && cmax) return ErrorCodes::HardpointElementMissingRequiredAttributes;

		SetPitchLimit(atof(cmin), atof(cmax));
	}

	// Otherwise pass back to the base class
	else
	{
		return ReadBaseHardpointXML(node, hashed_key);
	}

	// We processed this element in the subclass logic above
	return ErrorCodes::NoError;
}


HpTurret::~HpTurret(void)
{
}
