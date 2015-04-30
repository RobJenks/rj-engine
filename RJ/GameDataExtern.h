#pragma once

#ifndef __GameDataExternH__
#define __GameDataExternH__

#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_map>
#include "Equipment.h"

using namespace std;
using namespace std::tr1;

class Equipment;
class SimpleShipLoadout;
class ImmediateRegion;
class SystemRegion;
class SpaceSystem;
class UserInterface;
class SimpleShip;
class ComplexShip;
class ComplexShipSection;
class ComplexShipTileDefinition;
class ComplexShipTileClass;
class ComplexShipObjectClass;
class StaticTerrainDefinition;
class Resource;
class SpaceProjectileDefinition;
class SkinnedModel;
class ActorBase;

namespace D {

	#ifndef __GameDataExtern_Types_H__
	#define __GameDataExtern_Types_H__
		typedef std::tr1::unordered_map<string, SimpleShip*> SimpleShipRegister;
		typedef std::tr1::unordered_map<string, SimpleShipLoadout*> SSLoadoutRegister;
		
		typedef std::tr1::unordered_map<string, ComplexShip*> ComplexShipRegister; 
		typedef std::tr1::unordered_map<string, ComplexShipSection*> ComplexShipSectionRegister;
		typedef std::tr1::unordered_map<string, ComplexShipTileDefinition*> ComplexShipTileRegister;
		typedef std::tr1::unordered_map<string, ComplexShipTileClass*> ComplexShipTileClassRegister;
		typedef std::tr1::unordered_map<string, ComplexShipObjectClass*> ComplexShipObjectClassRegister;
		
		typedef std::tr1::unordered_map<string, StaticTerrainDefinition*> StaticTerrainRegister;

		typedef std::tr1::unordered_map<string, class Equipment*> EquipRegister;
		typedef std::tr1::unordered_map<string, Resource*> ResourceRegister;
		typedef std::tr1::unordered_map<string, SpaceProjectileDefinition*> ProjectileRegister;

		typedef std::tr1::unordered_map<string, SkinnedModel*> SkinnedModelRegister;
		typedef std::tr1::unordered_map<string, ActorBase*> ActorRegister;
	#endif


	// Primary data collections
	extern SimpleShipRegister				SimpleShips;				// The details of all simple ship classes in the game
	extern SSLoadoutRegister				SSLoadouts;					// The details of all simple ship loadouts in the game
	extern EquipRegister					Equipment;					// The details of all equipment in the game

	extern ComplexShipRegister				ComplexShips;				// The details of all complex ships in the game
	extern ComplexShipSectionRegister		ComplexShipSections;		// The details of all complex ship sections in the game
	extern ComplexShipTileRegister			ComplexShipTiles;			// The details of all complex ship tiles in the game
	extern ComplexShipTileClassRegister		ComplexShipTileClasses;		// The details of all complex ship tile classes in the game
	extern ComplexShipObjectClassRegister	ComplexShipObjectClasses;	// The details of all object classes in the game

	extern StaticTerrainRegister			StaticTerrainDefinitions;	// The details of all static terrain classes in the game

	extern ResourceRegister					Resources;					// The details of each resource in the game
	extern ProjectileRegister				Projectiles;				// The details of each projectile type in the game

	extern SkinnedModelRegister				SkinnedModels;				// The details of all skinned models in the game
	extern ActorRegister					Actors;						// The details of all actor base objects in the game


	// Managed accessor methods for each primary data collection
	CMPINLINE SimpleShip *GetSimpleShip(const string & code) { if (SimpleShips.count(code) > 0) return SimpleShips[code]; else return NULL; }
	CMPINLINE SimpleShipLoadout *GetSSLoadout(const string & code) { if (SSLoadouts.count(code) > 0) return SSLoadouts[code]; else return NULL; }
	CMPINLINE ComplexShip *GetComplexShip(const string & code) { if (ComplexShips.count(code) > 0) return ComplexShips[code]; else return NULL; }
	CMPINLINE ComplexShipSection *GetComplexShipSection(const string & code) { if (ComplexShipSections.count(code) > 0) return ComplexShipSections[code]; else return NULL; }
	CMPINLINE ComplexShipTileDefinition *GetComplexShipTile(const string & code) { if (ComplexShipTiles.count(code) > 0) return ComplexShipTiles[code]; else return NULL; }
	CMPINLINE ComplexShipTileClass *GetComplexShipTileClass(const string & code) { if (ComplexShipTileClasses.count(code) > 0) return ComplexShipTileClasses[code]; else return NULL; }
	CMPINLINE ComplexShipObjectClass *GetComplexShipObjectClass(const string & code) { if (ComplexShipObjectClasses.count(code) > 0) return ComplexShipObjectClasses[code]; else return NULL; }
	CMPINLINE StaticTerrainDefinition *GetStaticTerrain(const string & code) { if (code != NullString && StaticTerrainDefinitions.count(code) > 0) return StaticTerrainDefinitions[code]; else return NULL; }
	CMPINLINE Resource *GetResource(const string & code) { if (Resources.count(code) > 0) return Resources[code]; else return NULL; }
	CMPINLINE SpaceProjectileDefinition *GetProjectile(const string & code) { if (Projectiles.count(code) > 0) return Projectiles[code]; else return NULL; }
	CMPINLINE SkinnedModel *GetSkinnedModel(const string & code) { if (SkinnedModels.count(code) > 0) return SkinnedModels[code]; else return NULL; }
	CMPINLINE ActorBase *GetActor(const string & code) { if (Actors.count(code) > 0) return Actors[code]; else return NULL; }
	CMPINLINE class Equipment *GetEquipment(const string & code) { if (Equipment.count(code) > 0) return Equipment[code]; else return NULL; }

	// Managed methods to add items to each primary data collection.  Unregister any iObjects since we do not want them to be simulated in-game
	void AddStandardSimpleShip(SimpleShip *s); 
	void AddStandardSSLoadout(SimpleShipLoadout *l);
	void AddStandardComplexShip(ComplexShip *s);
	void AddStandardComplexShipSection(ComplexShipSection *s);
	void AddStandardComplexShipTileDefinition(ComplexShipTileDefinition *t);
	void AddStandardComplexShipTileClass(ComplexShipTileClass *c);
	void AddStandardComplexShipObjectClass(ComplexShipObjectClass *c);
	void AddStaticTerrain(StaticTerrainDefinition *d);
	void AddStandardResource(Resource *r);
	void AddStandardProjectile(SpaceProjectileDefinition *p);
	void AddStandardSkinnedModel(SkinnedModel *m);
	void AddStandardActor(ActorBase *a);
	void AddStandardEquipment(class Equipment *e);


	// Enumeration of possible complex ship tile types
	enum TileClass { Unknown = 0, Corridor, Quarters, Power, Coolant, LifeSupport, _COUNT };

	// The user interface
	extern UserInterface				*UI;

	// Primary game regions
	namespace Regions
	{
		extern ImmediateRegion				*Immediate;
		extern SystemRegion					*System;
	}

	// String constant data for loading external game data
	extern const char *DATA;
	extern const char *NODE_GameData;
	extern const char *NODE_FileIndex;
	extern const char *NODE_SimpleShip;
	extern const char *NODE_SimpleShipLoadout;
	extern const char *NODE_Engine;
	extern const char *NODE_System;
	extern const char *NODE_FireEffect;
	extern const char *NODE_ParticleEmitter;
	extern const char *NODE_UILayout;
	extern const char *NODE_Image2DGroup;
	extern const char *NODE_ComplexShip;
	extern const char *NODE_ComplexShipElement;
	extern const char *NODE_Hardpoint;
	extern const char *NODE_ComplexShipSection;
	extern const char *NODE_ComplexShipSectionInstance;
	extern const char *NODE_ComplexShipTileClass;
	extern const char *NODE_ComplexShipTileDefinition;
	extern const char *NODE_ComplexShipTile;
	extern const char *NODE_ComplexShipTileBaseData;
	extern const char *NODE_Model;
	extern const char *NODE_UIManagedControlDefinition;
	extern const char *NODE_Resource;
	extern const char *NODE_SpaceProjectileDefinition;
	extern const char *NODE_SkinnedModel;
	extern const char *NODE_ActorAttributeGeneration;
	extern const char *NODE_ActorBase;
	extern const char *NODE_StaticTerrain;
	extern const char *NODE_StaticTerrainDefinition;

	// String constant data for specific game data files, typically those core ones updated by the program such as the ship register
	extern const char *FILE_ComplexShipRegister;

	// Termination function for all game data registers
	void TerminateAllDataRegisters(void);

	// Termination functions for each individual data register
	void TerminateAllSimpleShipRegisterData(void);
	void TerminateAllSSLoadoutRegisterData(void);
	void TerminateAllEquipmentRegisterData(void);
	void TerminateAllComplexShipRegisterData(void);
	void TerminateAllComplexShipSectionRegisterData(void);
	void TerminateAllComplexShipTileRegisterData(void);
	void TerminateAllComplexShipTileClassRegisterData(void);
	void TerminateAllComplexShipObjectClassRegisterData(void);
	void TerminateAllStaticTerrainRegisterData(void);
	void TerminateAllResourceRegisterData(void);
	void TerminateAllProjectileRegisterData(void);
	void TerminateAllSkinnedModelRegisterData(void);
	void TerminateAllActorRegisterData(void);
}



#endif