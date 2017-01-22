#pragma once

#ifndef __GameDataExternH__
#define __GameDataExternH__

#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_map>
#include "DataRegister.h"
#include "Equipment.h"

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
class SpaceTurret;
class ProjectileLauncher;
class BasicProjectileDefinition;
class SpaceProjectileDefinition;
class SkinnedModel;
class ActorBase;
class DynamicTileSet;

// This file contains no objects with special alignment requirements
namespace D {

	// Primary data collections
	extern DataRegister<SimpleShip>						SimpleShips;				// The details of all simple ship classes in the game
	extern DataRegister<SimpleShipLoadout>				SSLoadouts;					// The details of all simple ship loadouts in the game
	extern DataRegister<class Equipment>				Equipment;					// The details of all equipment in the game

	extern DataRegister<ComplexShip>					ComplexShips;				// The details of all complex ships in the game
	extern DataRegister<ComplexShipSection>				ComplexShipSections;		// The details of all complex ship sections in the game
	extern DataRegister<ComplexShipTileDefinition>		ComplexShipTiles;			// The details of all complex ship tiles in the game
	extern DataRegister<ComplexShipTileClass>			ComplexShipTileClasses;		// The details of all complex ship tile classes in the game
	extern DataRegister<DynamicTileSet>					DynamicTileSets;			// Details of all dynamic tile sets in the game
	extern DataRegister<ComplexShipObjectClass>			ComplexShipObjectClasses;	// The details of all object classes in the game

	extern DataRegister<StaticTerrainDefinition>		StaticTerrainDefinitions;	// The details of all static terrain classes in the game

	extern DataRegister<Resource>						Resources;					// The details of each resource in the game

	extern DataRegister<SpaceTurret>					Turrets;					// The details of each turret type in the game
	extern DataRegister<ProjectileLauncher>				ProjectileLaunchers;		// The details of each projectile launcher in the game
	extern DataRegister<BasicProjectileDefinition>		BasicProjectiles;			// The details of each basic projectile type in the game
	extern DataRegister<SpaceProjectileDefinition>		SpaceProjectiles;			// The details of each space projectile type in the game

	extern DataRegister<SkinnedModel>					SkinnedModels;				// The details of all skinned models in the game
	extern DataRegister<ActorBase>						Actors;						// The details of all actor base objects in the game


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
	extern std::string DATA_S;
	extern const char *IMAGE_DATA;
	extern std::string IMAGE_DATA_S;

	extern const char *NODE_GameData;
	extern const char *NODE_Config;
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
	extern const char *NODE_ArticulatedModel;
	extern const char *NODE_UIManagedControlDefinition;
	extern const char *NODE_Resource;
	extern const char *NODE_BasicProjectileDefinition; 
	extern const char *NODE_SpaceProjectileDefinition;
	extern const char *NODE_SkinnedModel;
	extern const char *NODE_ActorAttributeGeneration;
	extern const char *NODE_ActorBase;
	extern const char *NODE_StaticTerrain;
	extern const char *NODE_StaticTerrainDefinition;
	extern const char *NODE_Faction;
	extern const char *NODE_Turret;
	extern const char *NODE_ProjectileLauncher;
	extern const char *NODE_DynamicTileSet;
	extern const char *NODE_ModifierDetails;
	extern const char *NODE_Damage;
	extern const char *NODE_DamageSet;

	// String constant data for specific game data files, typically those core ones updated by the program such as the ship register
	extern const char *FILE_ComplexShipRegister;

}



#endif