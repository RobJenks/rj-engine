#pragma once

#ifndef __GameDataH__
#define __GameDataH__

#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;
using namespace std::tr1;

#include <unordered_map>

class Ship;
class ShipDetails;
class SimpleShipLoadout;
class Equipment;
class Engine;

namespace D {

	typedef std::tr1::unordered_map<string, ShipDetails*> ShipRegister;					// Custom hash register: ships
	typedef std::tr1::unordered_map<string, SimpleShipLoadout*> SSLoadoutRegister;		// Custom hash register: simple ship loadouts
	typedef std::tr1::unordered_map<string, Equipment*> EquipRegister;					// Custom hash register: equipment	


	
	
	

}




#endif