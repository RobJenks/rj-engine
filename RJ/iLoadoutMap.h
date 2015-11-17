#pragma once

#ifndef __iLoadoutMapH__
#define __iLoadoutMapH__

#include <string>
class Equipment;


// This class has no special alignment requirements
class iLoadoutMap
{
public:
	virtual std::string		GetHardpoint(void) const = 0;
	virtual Equipment *		GetEquipment(void) const = 0;
};





#endif