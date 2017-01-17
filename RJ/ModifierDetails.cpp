#include "StandardModifiers.h"
#include "ModifierDetails.h"


// Indicates whether this object is a "null" modifier
bool ModifierDetails::IsNull(void) const
{
	return (m_id == StandardModifiers::NO_MODIFIER);
}