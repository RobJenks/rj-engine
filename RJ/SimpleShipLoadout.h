#pragma once

#ifndef __SimpleShipLoadoutH__
#define __SimpleShipLoadoutH__

#include <cstdlib>
#include <string>
#include <unordered_map>
using namespace std;
using namespace std::tr1;

#include "ErrorCodes.h"
class Equipment;
class Hardpoint;
class SimpleShip;
class SimpleShipDetails;
class iLoadoutMap;
class LoadoutMap;
class CompoundLoadoutMap;

class SimpleShipLoadout
{
public:
	typedef std::tr1::unordered_map<string, LoadoutMap*> LoadoutMapCollection;
	typedef std::tr1::unordered_map<string, CompoundLoadoutMap*> CompoundLoadoutMapCollection;

	static SimpleShipLoadout *SimpleShipLoadout::Get(const string &code);

	// Static methods to handle assignment of loadouts to ship instances
	static Result	AssignDefaultLoadoutToSimpleShip	(SimpleShip *s);
	static Result	AssignLoadoutToSimpleShip			(SimpleShip *s, SimpleShipLoadout *l);
	static void		ApplyLoadoutMap						(SimpleShip *s, iLoadoutMap *map);
	
	std::string Code;								// Unique string code identifying this loadout
	std::string Ship;							// The ship to which this loadout relates
	LoadoutMapCollection Maps;					// Collection of direct loadout mappings
	CompoundLoadoutMapCollection CompoundMaps;	// Collection of compound (i.e. conditional) mappings

	void AddMap(string hp, Equipment *equip);					// Adds a new standard map to the loadout
	void AddCompoundMap(string hp, CompoundLoadoutMap *map);	// Adds a new compound map to the loadout

	SimpleShipLoadout(void);
	~SimpleShipLoadout(void);
};

#endif