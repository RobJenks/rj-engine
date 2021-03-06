#include "Damage.h"
#include "GameConsoleCommand.h"
#include "GamePhysicsEngine.h"
#include "iTakesDamage.h"

// Static local vector used to translate Damage > DamageSet without the need for an additional virtual call
DamageSet iTakesDamage::_tmp_damageset = DamageSet(1U, Damage(DamageType::ANY, 0));

// Protected method called to apply a particular component of incoming damage.  Calculates modified 
// damage value (based on e.g. damage resistances) and applies to the object hitpoints.  Returns 
// true if the object was destroyed by this damage
bool iTakesDamage::ApplyDamageComponent(Damage damage)
{
	// Invulnerable objects ignore all damage
	if (m_is_invulnerable) return false;

	// Check whether we have any damage resistance that could reduce the impact
	if (HasDamageResistance())
	{
		damage.ApplyDamageResistance(m_damageresistance);
	}

	// Apply the damage to entity health; returns a flag indicating whether this change to entity health resulted in its destruction
	return DecreaseHealth(damage.Amount);
}

// Primary method called when the object takes multiple types of damage.  Calculates modified 
// damage value (based on e.g. damage resistances) and applies to the object hitpoints.  Damage
// is applied in the order in which is was added to the damage set.  Returns true if the object
// was destroyed by any of the damage in this damage set
bool iTakesDamage::ApplyDamage(const DamageSet & damage, const GamePhysicsEngine::OBBIntersectionData & location)
{
	// Invulnerable objects ignore all damage
	if (m_is_invulnerable) return false;

	// Apply each damage type in the order specified in the damage set
	DamageSet::const_iterator it_end = damage.end();
	for (DamageSet::const_iterator it = damage.begin(); it != it_end; ++it)
	{
		// Test whether each damage component was enough to destroy the entity; if so, stop
		// processing at that point and return true for destruction
		if (ApplyDamageComponent(*it) == true) return true;		
	}

	// The object was damaged but not destroyed, so return false
	return false;
}


// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void iTakesDamage::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(GetHealth)
	REGISTER_DEBUG_ACCESSOR_FN(GetMaxHealth)
	REGISTER_DEBUG_ACCESSOR_FN(IsInvulnerable)
	REGISTER_DEBUG_ACCESSOR_FN(HasDamageResistance)

	// Mutator methods
	REGISTER_DEBUG_FN(SetHealth, command.ParameterAsFloat(2))
	REGISTER_DEBUG_FN(IncreaseHealth, command.ParameterAsFloat(2))
	REGISTER_DEBUG_FN(DecreaseHealth, command.ParameterAsFloat(2))
	REGISTER_DEBUG_FN(SetMaxHealth, command.ParameterAsFloat(2))
	REGISTER_DEBUG_FN(SetInvulnerabilityFlag, command.ParameterAsBool(2))
	REGISTER_DEBUG_FN(ApplyDamage, command.ParameterAsFloat(2))
	REGISTER_DEBUG_FN(DestroyObject)
}

// Destructor
iTakesDamage::~iTakesDamage(void)
{

}