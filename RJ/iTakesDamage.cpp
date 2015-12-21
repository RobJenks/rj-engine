#include "Damage.h"
#include "iTakesDamage.h"


// Primary method called when the object takes damage.  Calculates modified damage value (based on e.g.
// damage resistances) and applies to the object hitpoints.  Returns true if the object was destroyed
// by this damage
bool iTakesDamage::ApplyDamage(Damage damage)
{
	// Invulnerable objects ignore all damage
	if (m_is_invulnerable) return false;

	// Check whether we have any damage resistance that could reduce the impact
	if (HasDamageResistance())
	{
		// Look for resistance to this particular type of damage
		DamageResistanceSet::const_iterator it_end = m_damageresistance.end();
		for (DamageResistanceSet::const_iterator it = m_damageresistance.begin(); it != it_end; ++it)
		{
			if ((*it).Type == damage.Type)
			{
				// We have resistance to this type.  First, if it is below our damage threshold then simply ignore the damage
				const DamageResistance & dr = (*it);
				if (damage.Amount <= dr.Threshold) return false;

				// If not, we can at least scale the damage based on our resistance
				damage.Amount *= dr.Modifier;

				// There is only one DR entry per damage type, so we can stop searching here
				break;
			}
		}
	}

	// Apply the damage to entity health; returns a flag indicating whether this change to entity health resulted in its destruction
	return DecreaseHealth(damage.Amount);
}

// Primary method called when the object takes multiple types of damage.  Calculates modified 
// damage value (based on e.g. damage resistances) and applies to the object hitpoints.  Damage
// is applied in the order in which is was added to the damage set.  Returns true if the object
// was destroyed by any of the damage in this damage set
bool iTakesDamage::ApplyDamage(const DamageSet & damage)
{
	// Invulnerable objects ignore all damage
	if (m_is_invulnerable) return false;

	// Apply each damage type in the order specified in the damage set
	DamageSet::const_iterator it_end = damage.end();
	for (DamageSet::const_iterator it = damage.begin(); it != it_end; ++it)
	{
		// Test whether each damage component was enough to destroy the entity; if so, stop
		// processing at that point and return true for destruction
		if (ApplyDamage(*it) == true) return true;		
	}

	// The object was damaged but not destroyed, so return false
	return false;
}