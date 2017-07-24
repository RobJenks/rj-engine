#include "Equip.h"
#include "Logging.h"
#include "Hardpoint.h"
#include "Weapon.h"
#include "SpaceTurret.h"

#include "HpTurret.h"


HpTurret::HpTurret(void)
	:
	m_turret_id(SpaceTurret::NULL_TURRET), m_yaw_limited(false), m_yawmin(-0.15f), m_yawmax(+0.15f), m_pitchmin(-0.15f), m_pitchmax(+0.15f)
{
}

// Recalculate any properties of this turret hardpoint based on the mounted equipment (or lack thereof)
void HpTurret::RecalculateHardpointData()
{


}

// Virtual method to allow mounting of class-specific equipment by a call to the base class instance
void HpTurret::MountEquipment(Equipment *equipment) 
{ 
	// Remove any existing weapon attached to this hardpoint
	UnmountCurrentWeapon();

	// Make sure the supplied equipment is valid
	if (equipment && equipment->GetType() == Equip::Class::Turret)
	{
		MountWeapon((Weapon*)equipment);
	}
	else
	{
		// Otherwise mount NULL, which signifies nothing is mounted
		MountWeapon(NULL);
	}
}

void HpTurret::MountWeapon(Weapon *weapon)
{
	// Get a reference to our parent
	if (!m_parent) return;
	iContainsHardpoints *parent = m_parent->GetHPParent();
	if (!parent) return;

	// Process the new equipment
	if (weapon != NULL)
	{
		// Attempt to create a new turret corresponding to this weapon
		SpaceTurret *turret = CreateHardpointTurret(weapon);
		if (!turret)
		{
			Game::Log << LOG_WARN << "Attempted to create turret \"" << weapon->GetTurretCode() << "\" when mounting hardpoint \"" << Code << "\" equipment but could not create turret object\n";
			m_equipment = NULL;
		}

		// We are now associated with this turret
		m_turret_id = turret->GetTurretID();

		// Mount the equipment
		m_equipment = weapon;

		// Update the parent and add this new turret
		parent->TurretController.AddTurret(turret);
	}
	else
	{
		// We are mounting a null weapon which is effectively leaving the hardpoint empty
		m_equipment = NULL;
	}

	// Recalculate hardpoint data
	RecalculateHardpointData();
}

// Unmount any equipment that we currently have mounted
void HpTurret::UnmountCurrentWeapon(void)
{
	if (m_equipment)
	{
		// Get a reference to our parent
		if (!m_parent) return;
		iContainsHardpoints *parent = m_parent->GetHPParent();
		if (!parent) return;

		// Tell our parent to remove the turret associated with this weapon
		Weapon *weapon = (Weapon*)m_equipment;
		if (!parent->TurretController.RemoveTurretByID(GetTurretID()))
		{
			Game::Log << LOG_WARN << "Attempted to unmount current weapon with turret ID " << GetTurretID() << " but could not remove corresponding turret object\n";
		}

		// We are no longer associated with any turret
		m_turret_id = SpaceTurret::NULL_TURRET;

		// TODO: we do not deallocate the equipment, for now.  Fix this via smart ptr or something similar
		m_equipment = NULL;
	}
}

// Create the turret instance that will be associated with this hardpoint.  Returns NULL if the 
// turret cannot be created for any reason
SpaceTurret * HpTurret::CreateHardpointTurret(Weapon *weapon)
{
	// Parameter check
	if (!weapon) return NULL;

	// Attempt to instantiate the turret based on our defined turret code
	SpaceTurret *turret = SpaceTurret::Create(weapon->GetTurretCode());
	if (!turret) return NULL;

	// Set relative position and orientation based on hardpoint properties
	turret->SetRelativePosition(Position);
	turret->SetBaseRelativeOrientation(Orientation);
	
	// Update pitch limit to the more restrictive of { turret limits, hardpoint limits }
	turret->SetPitchLimits(max(turret->GetMinPitchLimit(), m_pitchmin), min(turret->GetMaxPitchLimit(), m_pitchmax));

	// Update yaw limit to the more restrictive of { turret limits, hardpoint limits }
	// Only need to consider yaw limits if this hardpoint is actually limited; otherwise we just retain any turret limit
	if (m_yaw_limited)
	{
		float yawmin = (turret->IsYawLimited() ? max(turret->GetMinYawLimit(), m_yawmin) : m_yawmin);
		float yawmax = (turret->IsYawLimited() ? max(turret->GetMaxYawLimit(), m_yawmax) : m_yawmax);

		turret->SetYawLimitFlag(true);
		turret->SetYawLimits(yawmin, yawmax);
	}
	
	// Return a reference to the new pointer
	return turret;
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
		
		SetYawLimit((float)atof(cmin), (float)atof(cmax));
	}
	else if (hashed_key == HashedStrings::H_PitchLimit) {
		const char *cmin = node->Attribute("min");
		const char *cmax = node->Attribute("max");
		if (!cmin && cmax) return ErrorCodes::HardpointElementMissingRequiredAttributes;

		SetPitchLimit((float)atof(cmin), (float)atof(cmax));
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


