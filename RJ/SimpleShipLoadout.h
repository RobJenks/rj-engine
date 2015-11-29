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


// This class has no special alignment requirements
class SimpleShipLoadout
{
public:
	typedef std::tr1::unordered_map<string, LoadoutMap*> LoadoutMapCollection;
	typedef std::tr1::unordered_map<string, CompoundLoadoutMap*> CompoundLoadoutMapCollection;

	// Static methods to handle assignment of loadouts to ship instances
	static Result	AssignDefaultLoadoutToSimpleShip	(SimpleShip *s);
	static Result	AssignLoadoutToSimpleShip			(SimpleShip *s, SimpleShipLoadout *l);
	static void		ApplyLoadoutMap						(SimpleShip *s, iLoadoutMap *map);
	
	std::string Code;							// Unique string code identifying this loadout
	std::string Ship;							// The ship to which this loadout relates
	LoadoutMapCollection Maps;					// Collection of direct loadout mappings
	CompoundLoadoutMapCollection CompoundMaps;	// Collection of compound (i.e. conditional) mappings

	std::string GetCode(void) const	{ return Code; }			// Returns the unique code for this loadout

	void AddMap(string hp, Equipment *equip);					// Adds a new standard map to the loadout
	void AddCompoundMap(string hp, CompoundLoadoutMap *map);	// Adds a new compound map to the loadout

	// Shutdown method - not required for this class
	CMPINLINE void Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	SimpleShipLoadout(void);
	~SimpleShipLoadout(void);
};

#endif