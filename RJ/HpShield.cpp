#include "Equip.h"
#include "Hardpoint.h"
#include "Shield.h"

#include "HpShield.h"



HpShield::HpShield(void)
{
}

void HpShield::RecalculateHardpointData()
{
}

// Virtual method to allow mounting of class-specific equipment by a call to the base class instance
void HpShield::MountEquipment(Equipment *e) 
{ 
	if (!e || (e && e->GetType() == Equip::Class::Shield)) this->MountShield((Shield*)e); 
}

void HpShield::MountShield(Shield *shield)
{
	// Mount the equipment even if it is NULL; mounting a NULL item is essentially unmounting the equiment
	m_Shield = shield;

	// Recalculate hardpoint data
	RecalculateHardpointData();
}

// Read hardpoint content in from XML; must be implemented by child classes.  Accepts the hashed item key as a parameter
// to avoid duplication of effort.  All children should fall back to Hardpoint::ReadBaseHardpointXML if they cannot process an item themselves
Result HpShield::ReadFromXML(TiXmlElement *node, HashVal hashed_key)
{
	if (!node) return ErrorCodes::CannotLoadHardpointDataFromNullResources;

	// Process any class-specific properties
	if (false); /* None to be processed */

	// Otherwise pass back to the base class
	else
	{
		return ReadBaseHardpointXML(node, hashed_key);
	}

	// We processed this element in the subclass logic above
	return ErrorCodes::NoError;
}

HpShield::~HpShield(void)
{
}
