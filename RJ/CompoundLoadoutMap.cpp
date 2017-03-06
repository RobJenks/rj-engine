#include <math.h>
#include <stdio.h>

#include "FastMath.h"
#include "Equipment.h"

#include "CompoundLoadoutMap.h"

CompoundLoadoutMap::CompoundLoadoutMap(void)
{
	this->HP = ""; 
}

CompoundLoadoutMap::CompoundLoadoutMap(string hp)
{
	this->HP = hp;
}

CompoundLoadoutMap::~CompoundLoadoutMap(void)
{
	CompoundLoadoutItems::iterator it_end = this->Items.end();
	for (CompoundLoadoutItems::iterator it = this->Items.begin(); it != it_end; ++it)
		if (*it) delete (*it);
	this->Items.clear();
}

Equipment *CompoundLoadoutMap::GetEquipment(void) const
{
	// Make sure we have enough valid loadout data
	CompoundLoadoutItems::size_type n = Items.size();
	if (n == 0) return NULL;

	// Generate a cumulative probability array
	register float pval = 0.0f;
	float *cprob = new float[n];
	for (CompoundLoadoutItems::size_type i = 0; i < n; ++i)
	{
		CompoundLoadoutMapItem *item = this->Items.at(i);
		pval += item->Probability;
		cprob[i] = pval;
	}

	// Now generate a value up to this max cumulative probability 
	float r = frand_h(pval);
	
	// Move through the cumulative probability array until we find the relevant item
	for (CompoundLoadoutItems::size_type i = 0; i < n; ++i)
	{
		if (r <= cprob[i])
		{
			delete cprob;
			return this->Items.at(i)->Equip;
		}
	}

	// Default to the last item if necessary, to avoid float rounding errors
	SafeDeleteArray(cprob);
	return Items.at(n-1)->Equip;
}

void CompoundLoadoutMap::AddItem(Equipment *equip, float probability)
{
	// Validation to make sure we have valid probability value; equip CAN be NULL to specify nothing mounted
	if (probability < Game::C_EPSILON) return;

	// Create a new item and add it to the collection
	CompoundLoadoutMapItem *item = new CompoundLoadoutMapItem();
	item->Equip = equip;
	item->Probability = probability;
	this->Items.push_back(item);
}