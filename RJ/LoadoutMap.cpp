#include <stdio.h>

#include "Equipment.h"
#include "Hardpoint.h"

#include "LoadoutMap.h"

LoadoutMap::LoadoutMap(void)
{
	this->HP = ""; this->Equip = NULL;
}

LoadoutMap::LoadoutMap(string hp, Equipment *equip)
{
	this->HP = hp; this->Equip = equip;
}

LoadoutMap::~LoadoutMap(void)
{

}