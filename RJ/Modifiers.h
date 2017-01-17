#pragma once

#ifndef __ModifiersH__
#define __ModifiersH__

#include "HashIndexedVector.h"
#include "ModifierDetails.h"

class Modifiers
{
public:
	
	// Collection of all modifier details
	typedef HashIndexedVector<ModifierDetails, std::string>		ModifierDetailsCollection;
	static ModifierDetailsCollection							AllModifiers;

	// Constant limit on the number of modifiers that can be defined
	static const ModifierDetails::ModifierID					MODIFIER_LIMIT;

	// Returns the requested modifier
	CMPINLINE static ModifierDetails &							Get(ModifierDetails::ModifierID id) { return AllModifiers.GetReference(id); }
	CMPINLINE static ModifierDetails &							Get(const std::string & name)
	{
		ModifierDetailsCollection::CollectionType::iterator it = AllModifiers.Get(name);
		return (it == AllModifiers.Items().end() ? Modifiers::NoModifier() : (*it));
	}

	// Add a new modifier
	CMPINLINE static void										Add(const std::string & name, const std::string & description)
	{
		AllModifiers.Add(ModifierDetails(AllModifiers.Size(), name, description));
	}
	
	// Add a new modifier with the specified ID.  If an item already exists with this ID it will be replaced
	// If the collection is not currently large enough to incorporate this ID it will be expanded with default elements
	static void													AddWithID(ModifierDetails::ModifierID id, const std::string & name, const std::string & description);

	// Sets the name index for the specified modifier
	CMPINLINE static void										SetName(ModifierDetails::ModifierID index, std::string name) { AllModifiers.SetIndex(index, name); }

	// Returns a reference to the null, or "no modifier", entry
	static ModifierDetails &									NoModifier(void);


protected:


};


#endif