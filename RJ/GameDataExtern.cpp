#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_map>
#include <tchar.h>

#include "CompilerSettings.h"
#include "Ship.h"
#include "SimpleShip.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "ComplexShipTile.h"
#include "ComplexShipTileClass.h"
#include "ComplexShipTileDefinition.h"
#include "ComplexShipObjectClass.h"
#include "Model.h"
#include "Equipment.h"
#include "Engine.h"
#include "Resource.h"
#include "SkinnedModel.h"
#include "ActorBase.h"
#include "SpaceTurret.h"
#include "ProjectileLauncher.h"
#include "BasicProjectileDefinition.h"
#include "SpaceProjectileDefinition.h"
#include "TerrainDefinition.h"
#include "DynamicTerrain.h"
#include "DynamicTileSet.h"
class ImmediateRegion;
class SystemRegion;
class UserInterface;

#include "GameDataExtern.h"

namespace D {

	// Primary data collections
	DataRegister<SimpleShip>					SimpleShips(true);					// The details of all simple ship classes in the game	
	DataRegister<SimpleShipLoadout>				SSLoadouts(false);					// The details of all simple ship loadouts in the game
	DataRegister<class Equipment>				Equipment(false);					// The details of all equipment items in the game
	DataRegister<ComplexShip>					ComplexShips(true);					// The details of all complex ships in the game
	DataRegister<ComplexShipSection>			ComplexShipSections(true);			// The details of all complex ship sections in the game
	DataRegister<ComplexShipTileDefinition>		ComplexShipTiles(true);				// The details of all complex ship tiles in the game
	DataRegister<ComplexShipTileClass>			ComplexShipTileClasses(false);		// The details of all complex ship tile classes in the game
	DataRegister<DynamicTileSet>				DynamicTileSets(false);				// Details of all dynamic tile sets in the game
	DataRegister<ComplexShipObjectClass>		ComplexShipObjectClasses(false);	// Details of all object classes in the game
	DataRegister<TerrainDefinition>				TerrainDefinitions(false);			// Details of all static terrain classes in the game
	DataRegister<DynamicTerrain>				DynamicTerrainDefinitions(false);	// Details of all dynamic terrain classes in the game
	DataRegister<Resource>						Resources(false);					// Details of all resources in the game
	DataRegister<SpaceTurret>					Turrets(true);						// Details of all turret types in the game
	DataRegister<ProjectileLauncher>			ProjectileLaunchers(false);			// Details of all projectile launcher types in the game
	DataRegister<BasicProjectileDefinition>		BasicProjectiles(false);			// Details of all basic projectile types in the game
	DataRegister<SpaceProjectileDefinition>		SpaceProjectiles(false);			// Details of all space projectile types in the game
	DataRegister<SkinnedModel>					SkinnedModels(false);				// Details of all skinned models in the game
	DataRegister<ActorBase>						Actors(false);						// Details of all actor types in the game

	// The user interface
	UserInterface *					UI;

	// Primary game regions
	namespace Regions
	{
		ImmediateRegion *			Immediate;
		SystemRegion *				System;
	}

	// Game data location; can be updated by configuration
	const char *DATA = "../RJ/Data";
	std::string DATA_S = DATA;
	const char *IMAGE_DATA = "../RJ/Data/ImageContent/Data";
	std::string IMAGE_DATA_S = IMAGE_DATA;

	// Data read/edit/load constants
	const char *NODE_GameData = "gamedata";
	const char *NODE_Config= "config";
	const char *NODE_FileIndex = "include";
	const char *NODE_SimpleShip = "simpleship";
	const char *NODE_SimpleShipLoadout = "simpleshiploadout";
	const char *NODE_Engine = "engine";
	const char *NODE_System = "system";
	const char *NODE_FireEffect = "fireeffect";
	const char *NODE_ParticleEmitter = "particleemitter";
	const char *NODE_UILayout = "uilayout";
	const char *NODE_Image2DGroup = "image2dgroup";
	const char *NODE_ComplexShip = "complexship";
	const char *NODE_ComplexShipSection = "complexshipsection";
	const char *NODE_ComplexShipSectionInstance = "complexshipsectioninstance";
	const char *NODE_ComplexShipTileClass = "complexshiptileclass";
	const char *NODE_ComplexShipTileDefinition = "complexshiptiledefinition";
	const char *NODE_ComplexShipTile = "complexshiptile";
	const char *NODE_ComplexShipTileBaseData = "basetiledata";
	const char *NODE_ComplexShipElement = "complexshipelement";	
	const char *NODE_Hardpoint = "hardpoint";
	const char *NODE_Model = "model";
	const char *NODE_ArticulatedModel = "articulatedmodel";
	const char *NODE_UIManagedControlDefinition = "uimanagedcontroldefinition";
	const char *NODE_Resource = "resource";
	const char *NODE_BasicProjectileDefinition = "basicprojectiledefinition";
	const char *NODE_SpaceProjectileDefinition = "spaceprojectiledefinition";
	const char *NODE_SkinnedModel = "skinnedmodel";
	const char *NODE_ActorAttributeGeneration = "actorattributegeneration";
	const char *NODE_ActorBase = "actorbase";
	const char *NODE_Portal = "portal";
	const char *NODE_Terrain = "terrain";
	const char *NODE_TerrainDefinition = "terraindefinition";
	const char *NODE_DynamicTerrainDefinition = "dynamicterraindefinition";
	const char *NODE_Faction = "faction";
	const char *NODE_Turret = "turret";
	const char *NODE_ProjectileLauncher = "projectilelauncher";
	const char *NODE_DynamicTileSet = "dynamictileset";
	const char *NODE_ModifierDetails = "modifierdetails";
	const char *NODE_Damage = "damage";
	const char *NODE_DamageSet = "damageset";
	const char *NODE_DamageResistance = "damageresistance";
	const char *NODE_DamageResistanceSet = "damageresistanceset";
	const char *NODE_Audio = "audio";

	// String constant data for specific game data files, typically those core ones updated by the program such as the ship register
	const char *FILE_ComplexShipRegister = "Ships\\ComplexShipRegister.xml";




}


