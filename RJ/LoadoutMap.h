#pragma once

#ifndef __LoadoutMapH__
#define __LoadoutMapH__

#include "iLoadoutMap.h"
class Hardpoint;
class Equipment;

class LoadoutMap : iLoadoutMap
{
public:
	string HP;			// String code of the hardpoint being assigned to
	Equipment *Equip;	// The equipment to be mounted; pointer into the global equipment collection

	LoadoutMap(void);
	LoadoutMap(string hp, Equipment *equip);
	~LoadoutMap(void);

	// Interface virtual methods are a simple retrieval for the direct loadout map
	virtual string GetHardpoint(void) const		{ return this->HP; }
	virtual Equipment *GetEquipment(void) const	{ return this->Equip; }
};


#endif