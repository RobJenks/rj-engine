#include "iTakesDamageSimple.h"


// Apply damage to the object.  The simple implementation does not account for specific damage 
// resistances, so this effectively reduces object health by the total damage amount.  Returns true if
// the object was destroyed by the damage inflicted
bool iTakesDamageSimple::ApplyDamage(float damage)
{
	if (m_invulnerable) return false;

	return DecreaseHealth(damage);
}


// Apply damage to the object.  The simple implementation does not account for specific damage 
// resistances, so this effectively reduces object health by the total damage amount.  Returns true if
// the object was destroyed by the damage inflicted
bool iTakesDamageSimple::ApplyDamage(Damage damage)
{
	if (m_invulnerable) return false;

	return DecreaseHealth(damage.Amount);
}

// Apply damage to the object.  The simple implementation does not account for specific damage 
// resistances, so this effectively reduces object health by the total damage amount.  Returns true if
// the object was destroyed by the damage inflicted
bool iTakesDamageSimple::ApplyDamage(const DamageSet & damage)
{
	if (m_invulnerable) return false;

	for (Damage d : damage)
	{
		if (ApplyDamage(d) == true) return true;
	}

	return false;
}

// Destroys the object.  Object is not deallocated, rather the destruction flag is set which indicates the object
// no longer exists
void iTakesDamageSimple::DestroyObject(void)
{
	// Simply set the destruction flag on this object
	SetDestroyedFlag(true);
}





