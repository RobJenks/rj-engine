#include "GameDataExtern.h"
#include "Equipment.h"
#include "Hardpoint.h"
#include "SimpleShip.h"
#include "iLoadoutMap.h"
#include "LoadoutMap.h"
#include "CompoundLoadoutMap.h"
#include "CoreEngine.h"

#include "SimpleShipLoadout.h"

Result SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip(SimpleShip *s)
{
	// Ensure that we have been passed a valid ship pointer
	if (!s) return ErrorCodes::CannotAssignLoadoutToNullShip;

	// Get the default ship loadout
	string c_loadout = s->GetDefaultLoadout();
	if (c_loadout == NullString) return ErrorCodes::ShipHasNoDefaultLoadoutSpecified;

	// Look up this loadout code in the global collection
	SimpleShipLoadout *loadout = SimpleShipLoadout::Get(c_loadout);
	if (!loadout) return ErrorCodes::ShipDefaultLoadoutDoesNotExist;

	// If we have got this far the loadout exists; assign it to the ship and return the result
	return SimpleShipLoadout::AssignLoadoutToSimpleShip(s, loadout);
}

Result SimpleShipLoadout::AssignLoadoutToSimpleShip(SimpleShip *s, SimpleShipLoadout *l)
{
	// Validate that we have been given valid ship and loadout parameters
	if (!s) return ErrorCodes::CannotAssignLoadoutToNullShip;
	if (!l) return ErrorCodes::CannotAssignNullLoadoutToShip;

	// Make sure the loadout is intended for this target ship class
	if (l->Ship != s->GetCode()) return ErrorCodes::CannotAssignLoadoutToIncompatibleShipClass;

	// Iterate over the collection of all direct loadout maps
	LoadoutMapCollection::iterator m_it_end = l->Maps.end();
	for (LoadoutMapCollection::iterator m_it = l->Maps.begin(); m_it != m_it_end; ++m_it)
	{
		// Retrieve each loadout map in turn and process it
		iLoadoutMap *map = (iLoadoutMap*)(m_it->second);
		if (map) SimpleShipLoadout::ApplyLoadoutMap(s, map);
	}

	// Now repeat for the collection of all compound loadout maps
	CompoundLoadoutMapCollection::iterator cm_it_end = l->CompoundMaps.end();
	for (CompoundLoadoutMapCollection::iterator cm_it = l->CompoundMaps.begin(); cm_it != cm_it_end; ++cm_it)
	{
		// Retrieve each loadout map in turn and process it
		iLoadoutMap *map = (iLoadoutMap*)(cm_it->second);
		if (map) SimpleShipLoadout::ApplyLoadoutMap(s, map);
	}

	// Recalculate ship details based on the loadout, and then the ship based on its new details
	s->RecalculateShipDataFromCurrentState();
	

	// Return success if we reached this point
	return ErrorCodes::NoError;
}


void SimpleShipLoadout::ApplyLoadoutMap(SimpleShip *s, iLoadoutMap *map)
{
	// Get hardpoint code from the map and make sure it is non-NULL
	string c_hp = map->GetHardpoint();
	if (c_hp == NullString) return;
	
	// Get a pointer to the relevant hardpoint (assuming it exists)
	Hardpoint *h = s->GetHardpoints().Get(c_hp);
	if (!h) return; 
	
	// Locate the equipment, or potentially NULL if nothing to be mounted.  This method handles all logic to e.g. 
	// determine the equipment to be mounted from a set of probablistic options
	Equipment *e = map->GetEquipment();

	// If not NULL then clone the reference item of equipment; otherwise it remains at default of NULL
	Equipment *equip = NULL;
	if (e) equip = e->Clone();
			
	// Mount this equipment (or NULL) on the hardpoint
	h->MountEquipment(equip);

}

SimpleShipLoadout *SimpleShipLoadout::Get(const string &code)
{
	// Attempt to retrieve loadout based on string code
	if (code == NullString) return NULL;
	SimpleShipLoadout *loadout = D::SSLoadouts[code];

	// Return a pointer to the loadout if it is valid
	if (loadout)				return loadout;
	else						return NULL;
}

SimpleShipLoadout::SimpleShipLoadout(void) : Code(""), Ship("")
{
}

void SimpleShipLoadout::AddMap(string hp, Equipment *equip)
{
	// Test whether a map to this hardpoint already exists
	if (hp == NullString) return;
	LoadoutMap *m = this->Maps[hp];

	// If a map exists (not expected result) then delete it before storing the new map
	if (m) delete (m); 
	this->Maps[hp] = new LoadoutMap(hp, equip);
}

void SimpleShipLoadout::AddCompoundMap(string hp, CompoundLoadoutMap *map)
{
	// Test whether a map to this hardpoint already exists
	if (hp == NullString) return;
	CompoundLoadoutMap *m = this->CompoundMaps[hp];

	// If no compound map exists (expected result) then create it, otherwise overwrite the equipment assignment
	if (m) delete (m); 
	this->CompoundMaps[hp] = map;
}

SimpleShipLoadout::~SimpleShipLoadout(void)
{
	// Delete each loadout map in turn
	LoadoutMapCollection::iterator m_end = this->Maps.end();
	for (LoadoutMapCollection::iterator it = this->Maps.begin(); it != m_end; ++it)
		if (it->second) delete (it->second);
	this->Maps.clear();

	// Delete each compound loadout map in turn
	CompoundLoadoutMapCollection::iterator cm_end = this->CompoundMaps.end();
	for (CompoundLoadoutMapCollection::iterator it = this->CompoundMaps.begin(); it != cm_end; ++it)
		if (it->second) delete (it->second);
	this->CompoundMaps.clear();
}