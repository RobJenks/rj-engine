#include "Hp.h"

#include <string>
#include "Utility.h"

#include "Hardpoint.h"
#include "HpEngine.h"
#include "HpMissileLauncher.h"
#include "HpShield.h"
#include "HpTorpedoLauncher.h"
#include "HpWeapon.h"

using namespace std;

Equip::Class Hp::GetType(const string &typestring)
{
	const string s = StrLower(typestring);
	if (s == "weapon")
		return Equip::Class::Weapon;
	else if (s == "engine")
		return Equip::Class::Engine;
	else if (s == "missilelauncher")
		return Equip::Class::MissileLauncher;
	else if (s == "shield")
		return Equip::Class::Shield;
	else if (s == "torpedolauncher")
		return Equip::Class::TorpedoLauncher;
	
	else
		return Equip::Class::Unknown;
}

std::string Hp::ToString(const Equip::Class cls)
{
	switch (cls)
	{
		case Equip::Class::Engine:					return "engine";
		case Equip::Class::MissileLauncher:			return "missilelauncher";
		case Equip::Class::Shield:					return "shield";
		case Equip::Class::TorpedoLauncher:			return "torpedolauncher";
		case Equip::Class::Weapon:					return "weapon";
		default:									return "";
	}
}

Hardpoint *Hp::Create(Equip::Class type)
{
	switch (type) {
		case Equip::Class::Engine:
			return (new HpEngine());
			break;
		case Equip::Class::MissileLauncher:
			return (new HpMissileLauncher());
			break;
		case Equip::Class::Shield:
			return (new HpShield());
			break;
		case Equip::Class::TorpedoLauncher:
			return (new HpTorpedoLauncher());
			break;
		case Equip::Class::Weapon:
			return (new HpWeapon());
			break;
		default:
			return NULL;
	}
}