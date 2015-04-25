#pragma once
#ifndef __HPH__
#define __HPH__

#include "Equip.h"
#include <string>
using namespace std;

class Hardpoint;

class Hp
{
public:
	static Equip::Class			GetType(const string &typestring);
	static std::string			ToString(const Equip::Class cls);

	static Hardpoint *			Create(Equip::Class type);

};




#endif