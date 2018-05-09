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
class TerrainDefinition;
class DynamicTerrainDefinition;
class Resource;
class SpaceTurret;
class ProjectileLauncher;
class BasicProjectileDefinition;
class SpaceProjectileDefinition;
class SkinnedModel;
class ActorBase;
class DynamicTileSet;

// This class contains no objects with special alignment requirements
class D
{
public:

	// General templated method to uniquely return a data register based on its type
	template <typename T>
	static CMPINLINE DataRegister<T> &						GetDataRegister(void)
	{
		static_assert(false, "Data register not available for given type");
		return DataRegister<T>();
	}

	// Declare a data register and add a type-indexed method to retrieve the collection
#define DeclareDataRegister(Type, Name) \
	static DataRegister<class Type>							Name; \
	template<> static CMPINLINE DataRegister<class Type> &	GetDataRegister(void) { return Name; }
	
		// Primary data collections
		DeclareDataRegister(SimpleShip, SimpleShips);								// The details of all simple ship classes in the game
		DeclareDataRegister(SimpleShipLoadout, SSLoadouts);							// The details of all simple ship loadouts in the game
		DeclareDataRegister(Equipment, Equipment);									// The details of all equipment in the game

		DeclareDataRegister(ComplexShip, ComplexShips);								// The details of all complex ships in the game
		DeclareDataRegister(ComplexShipSection, ComplexShipSections);				// The details of all complex ship sections in the game
		DeclareDataRegister(ComplexShipTileDefinition, ComplexShipTiles);			// The details of all complex ship tiles in the game
		DeclareDataRegister(ComplexShipTileClass, ComplexShipTileClasses);			// The details of all complex ship tile classes in the game
		DeclareDataRegister(DynamicTileSet, DynamicTileSets);						// Details of all dynamic tile sets in the game
		DeclareDataRegister(ComplexShipObjectClass, ComplexShipObjectClasses);		// The details of all object classes in the game

		DeclareDataRegister(TerrainDefinition, TerrainDefinitions);					// The details of all static terrain classes in the game
		DeclareDataRegister(DynamicTerrainDefinition, DynamicTerrainDefinitions);	// The details of all dynamic terrain classes in the game

		DeclareDataRegister(Resource, Resources);									// The details of each resource in the game

		DeclareDataRegister(SpaceTurret, Turrets);									// The details of each turret type in the game
		DeclareDataRegister(ProjectileLauncher, ProjectileLaunchers);				// The details of each projectile launcher in the game
		DeclareDataRegister(BasicProjectileDefinition, BasicProjectiles);			// The details of each basic projectile type in the game
		DeclareDataRegister(SpaceProjectileDefinition, SpaceProjectiles);			// The details of each space projectile type in the game

		DeclareDataRegister(SkinnedModel, SkinnedModels);							// The details of all skinned models in the game
		DeclareDataRegister(ActorBase, Actors);										// The details of all actor base objects in the game

#undef DeclareDataRegister

	// Enumeration of possible complex ship tile types
	enum TileClass 
	{ 
		Unknown = 0, 
		Corridor, 
		Quarters, 
		PowerGenerator, 
		LifeSupport, 
		EngineRoom,
		Coolant,
		Armour, 
		_COUNT 
	};

	// The user interface
	static UserInterface						* UI;

	// Primary game regions
	struct Regions
	{
		static ImmediateRegion					* Immediate;
		static SystemRegion						* System;
	};

	// String constant data for loading external game data
	static const char *DATA;
	static std::string DATA_S;
	static const char *IMAGE_DATA;
	static std::string IMAGE_DATA_S;

	static const char *NODE_GameData;
	static const char *NODE_Config;
	static const char *NODE_FileIndex;
	static const char *NODE_SimpleShip;
	static const char *NODE_SimpleShipLoadout;
	static const char *NODE_Engine;
	static const char *NODE_System;
	static const char *NODE_FireEffect;
	static const char *NODE_ParticleEmitter;
	static const char *NODE_UILayout;
	static const char *NODE_Image2DGroup;
	static const char *NODE_ComplexShip;
	static const char *NODE_ComplexShipElement;
	static const char *NODE_Hardpoint;
	static const char *NODE_ComplexShipSection;
	static const char *NODE_ComplexShipSectionInstance;
	static const char *NODE_ComplexShipTileClass;
	static const char *NODE_ComplexShipTileDefinition;
	static const char *NODE_ComplexShipTile;
	static const char *NODE_ComplexShipTileBaseData;
	static const char *NODE_Model;
	static const char *NODE_ArticulatedModel;
	static const char *NODE_UIManagedControlDefinition;
	static const char *NODE_Resource;
	static const char *NODE_BasicProjectileDefinition; 
	static const char *NODE_SpaceProjectileDefinition;
	static const char *NODE_SkinnedModel;
	static const char *NODE_ActorAttributeGeneration;
	static const char *NODE_ActorBase;
	static const char *NODE_Portal;
	static const char *NODE_Terrain;
	static const char *NODE_TerrainDefinition;
	static const char *NODE_DynamicTerrain;
	static const char *NODE_DynamicTerrainDefinition;
	static const char *NODE_Faction;
	static const char *NODE_Turret;
	static const char *NODE_ProjectileLauncher;
	static const char *NODE_DynamicTileSet;
	static const char *NODE_ModifierDetails;
	static const char *NODE_Damage;
	static const char *NODE_DamageSet;
	static const char *NODE_DamageResistance;
	static const char *NODE_DamageResistanceSet;
	static const char *NODE_Audio;

	// String constant data for specific game data files, typically those core ones updated by the program such as the ship register
	static const char *FILE_ComplexShipRegister;

};

#endif