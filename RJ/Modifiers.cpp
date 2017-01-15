#include <vector>
#include "Modifiers.h"

// Initialise static fields
Modifiers::ModifierDetailsCollection Modifiers::AllModifiers;
const Modifiers::ModifierDetailsID Modifiers::MODIFIER_LIMIT = 1000000U;		// We can define a maximum of one million modifier types


// Add a new modifier with the specified ID.  If an item already exists with this ID it will be replaced
// If the collection is not currently large enough to incorporate this ID it will be expanded with default elements
void Modifiers::AddWithID(ModifierDetailsID id, const std::string & name, const std::string & description)
{
	if (id > MODIFIER_LIMIT) return;		// Safety check

	ModifierDetailsID size = AllModifiers.Size();
	if (id == size)
	{
		// We are trying to add this at the next location in the vector, so simply perform a normal "Add()"
		AllModifiers.Add(ModifierDetails(id, name, description), name);
	}
	else if (id < size)
	{
		// An item already exists at this location, so we want to replace it
		AllModifiers.RemoveIndex(id);
		AllModifiers.Set(id, ModifierDetails(id, name, description));
		AllModifiers.SetIndex(id, name);
	}
	else
	{
		// The collection needs to be expanded (to index-1) before adding this element (at index)
		AllModifiers.Items().insert(AllModifiers.Items().end(), (id - size), ModifierDetails());
		AllModifiers.Add(ModifierDetails(id, name, description), name);
	}
}