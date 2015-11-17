#pragma once

#ifndef __CompoundLoadoutMapH__
#define __CompoundLoadoutMapH__

#include <stdio.h>
#include <utility>

#include "iLoadoutMap.h"
class Equipment;


// This class has no special alignment requirements
class CompoundLoadoutMap : iLoadoutMap
{
public:
	struct CompoundLoadoutMapItem 
		{ Equipment *Equip; float Probability; };		// Tuple relating equipment to its probability of occuring
	typedef std::vector<CompoundLoadoutMapItem*> CompoundLoadoutItems;

	string HP;											// String code of the hardpoint in question
	CompoundLoadoutItems Items;							// Collection of all probability maps for this HP

	void AddItem(Equipment *equip, float probability);		// Add an item to the collection

	// Virtual iLoadoutMap interface methods; equipment retrieval is more complex for compound maps and is defined separately
	virtual string GetHardpoint(void) const { return this->HP; }
	virtual Equipment *GetEquipment(void) const;

	CompoundLoadoutMap(void);
	CompoundLoadoutMap(string hp);
	~CompoundLoadoutMap(void);
};


#endif