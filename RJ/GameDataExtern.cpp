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
#include "DynamicTerrainDefinition.h"
#include "DynamicTileSet.h"
class ImmediateRegion;
class SystemRegion;
class UserInterface;

#include "GameDataExtern.h"

// Define a data register and add a type-indexed method to retrieve the collection
#define DefineDataRegister(Type, Name, PerformShutdown) \
DataRegister<class Type> D::Name(PerformShutdown); 

// Primary data collections
DefineDataRegister(SimpleShip, SimpleShips, true);								// The details of all simple ship classes in the game	
DefineDataRegister(SimpleShipLoadout, SSLoadouts, false);						// The details of all simple ship loadouts in the game
DefineDataRegister(Equipment, Equipment, false);								// The details of all equipment items in the game
DefineDataRegister(ComplexShip, ComplexShips, true);							// The details of all complex ships in the game
DefineDataRegister(ComplexShipSection, ComplexShipSections, true);				// The details of all complex ship sections in the game
DefineDataRegister(ComplexShipTileDefinition, ComplexShipTiles, true);			// The details of all complex ship tiles in the game
DefineDataRegister(ComplexShipTileClass, ComplexShipTileClasses, false);		// The details of all complex ship tile classes in the game
DefineDataRegister(DynamicTileSet, DynamicTileSets, false);						// Details of all dynamic tile sets in the game
DefineDataRegister(ComplexShipObjectClass, ComplexShipObjectClasses, false);	// Details of all object classes in the game
DefineDataRegister(TerrainDefinition, TerrainDefinitions, false);				// Details of all static terrain classes in the game
DefineDataRegister(DynamicTerrainDefinition, DynamicTerrainDefinitions, false);	// Details of all dynamic terrain classes in the game
DefineDataRegister(Resource, Resources, false);									// Details of all resources in the game
DefineDataRegister(SpaceTurret, Turrets, true);									// Details of all turret types in the game
DefineDataRegister(ProjectileLauncher, ProjectileLaunchers, false);				// Details of all projectile launcher types in the game
DefineDataRegister(BasicProjectileDefinition, BasicProjectiles, false);			// Details of all basic projectile types in the game
DefineDataRegister(SpaceProjectileDefinition, SpaceProjectiles, false);			// Details of all space projectile types in the game
DefineDataRegister(SkinnedModel, SkinnedModels, false);							// Details of all skinned models in the game
DefineDataRegister(ActorBase, Actors, false);									// Details of all actor types in the game

// The user interface
UserInterface *	D::UI;

// Primary game regions
ImmediateRegion * D::Regions::Immediate;
SystemRegion * D::Regions::System;


// Game data location; can be updated by configuration
const char *D::DATA = "../RJ/Data";
std::string D::DATA_S = DATA;
const char *D::IMAGE_DATA = "../RJ/Data/ImageContent/Data";
std::string D::IMAGE_DATA_S = IMAGE_DATA;

// Data read/edit/load constants
const char * D::NODE_GameData = "gamedata";
const char * D::NODE_Config= "config";
const char * D::NODE_FileIndex = "include";
const char * D::NODE_SimpleShip = "simpleship";
const char * D::NODE_SimpleShipLoadout = "simpleshiploadout";
const char * D::NODE_Engine = "engine";
const char * D::NODE_System = "system";
const char * D::NODE_FireEffect = "fireeffect";
const char * D::NODE_ParticleEmitter = "particleemitter";
const char * D::NODE_UILayout = "uilayout";
const char * D::NODE_Image2DGroup = "image2dgroup";
const char * D::NODE_ComplexShip = "complexship";
const char * D::NODE_ComplexShipSection = "complexshipsection";
const char * D::NODE_ComplexShipSectionInstance = "complexshipsectioninstance";
const char * D::NODE_ComplexShipTileClass = "complexshiptileclass";
const char * D::NODE_ComplexShipTileDefinition = "complexshiptiledefinition";
const char * D::NODE_ComplexShipTile = "complexshiptile";
const char * D::NODE_ComplexShipTileBaseData = "basetiledata";
const char * D::NODE_ComplexShipElement = "complexshipelement";	
const char * D::NODE_Hardpoint = "hardpoint";
const char * D::NODE_Model = "model";
const char * D::NODE_ArticulatedModel = "articulatedmodel";
const char * D::NODE_UIManagedControlDefinition = "uimanagedcontroldefinition";
const char * D::NODE_Resource = "resource";
const char * D::NODE_BasicProjectileDefinition = "basicprojectiledefinition";
const char * D::NODE_SpaceProjectileDefinition = "spaceprojectiledefinition";
const char * D::NODE_SkinnedModel = "skinnedmodel";
const char * D::NODE_ActorAttributeGeneration = "actorattributegeneration";
const char * D::NODE_ActorBase = "actorbase";
const char * D::NODE_Portal = "portal";
const char * D::NODE_Terrain = "terrain";
const char * D::NODE_TerrainDefinition = "terraindefinition";
const char * D::NODE_DynamicTerrain = "dynamicterrain";
const char * D::NODE_DynamicTerrainDefinition = "dynamicterraindefinition";
const char * D::NODE_Faction = "faction";
const char * D::NODE_Turret = "turret";
const char * D::NODE_ProjectileLauncher = "projectilelauncher";
const char * D::NODE_DynamicTileSet = "dynamictileset";
const char * D::NODE_ModifierDetails = "modifierdetails";
const char * D::NODE_Damage = "damage";
const char * D::NODE_DamageSet = "damageset";
const char * D::NODE_DamageResistance = "damageresistance";
const char * D::NODE_DamageResistanceSet = "damageresistanceset";
const char * D::NODE_Audio = "audio";

// String constant data for specific game data files, typically those core ones updated by the program such as the ship register
const char * D::FILE_ComplexShipRegister = "Ships\\ComplexShipRegister.xml";






