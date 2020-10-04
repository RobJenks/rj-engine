#pragma once

#ifndef __DataInputH__
#define __DataInputH__

#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include "DX11_Core.h"
#include "XML\\tinyxml.h"
#include "ErrorCodes.h"
#include "GameDataExtern.h"
#include "HashFunctions.h"
#include "CollisionSpatialDataF.h"
#include "VariableSizeValue.h"
#include "AudioParameters.h"
#include "InstanceFlags.h"
class iStaticObject;
class iActiveObject;
class iSpaceObject;
class iEnvironmentObject;
class iContainsHardpoints;
class Model;
class ArticulatedModel;
class Hardpoint;
class Engine;
class OrientedBoundingBox;
class SimpleShip;
class ComplexShip;
class ComplexShipSection;
class ComplexShipElement;
class ComplexShipTile;
class iSpaceObjectEnvironment;
class BoundingObject;
class ViewPortal;
class SimpleShipLoadout;
class CompoundLoadoutMap;
class Render2DGroup;
class ProductionCost;
class Faction;
class Terrain;
class DynamicTerrain;
struct DynamicTerrainState;
class UsableObject;
class EffectBase;
class FireEffect;
class TileConnections;
class ElementStateDefinition;
class Damage;
class DamageSet;
class DamageResistance;
class DamageResistanceSet;
namespace fs = std::filesystem;

// This file contains no objects with special alignment requirements
namespace IO { namespace Data 
{
	TiXmlDocument *LoadXMLDocument(const std::string &filename);
	Result LoadGameDataFile(const std::string &filename, bool follow_indices);
	Result LoadGameDataFile(const std::string &filename);
	Result LoadXMLFileIndex(TiXmlElement *node);
	Result LoadConfigFile(const std::string &filename);
	
	// Common methods to load intermediate class data
	bool LoadObjectData(TiXmlElement *node, HashVal hash, iObject *object);									// Loads iObject class data
	bool LoadDamageableEntityData(TiXmlElement *node, HashVal hash, iObject *object);						// Loads iTakesDamage class data
	bool LoadActiveObjectData(TiXmlElement *node, HashVal hash, iActiveObject *object);						// Loads iActiveObject class data
	bool LoadStaticObjectData(TiXmlElement *node, HashVal hash, iStaticObject *object);						// Loads iStaticObject class data
	bool LoadSpaceObjectData(TiXmlElement *node, HashVal hash, iSpaceObject *object);						// Loads iSpaceObject class data
	bool LoadEnvironmentObjectData(TiXmlElement *node, HashVal hash, iEnvironmentObject *object);			// Loads iEnvironmentObject class data
	bool LoadSpaceObjectEnvironmentData(TiXmlElement *node, HashVal hash, iSpaceObjectEnvironment *object);	// Loads iSpaceObjectEnvironment data
	bool LoadShipData(TiXmlElement *node, HashVal hash, Ship *object);										// Loads base ship class data
	bool LoadHardpointContainerData(TiXmlElement *node, HashVal hash, iContainsHardpoints *object);			// Load iContainsHardpoints class data


	/*** Primary load/reload-enabled object types ***/


	// Load internal data and return an instance, either created or updated depending on input parameters
	SimpleShip *						LoadSimpleShipData(TiXmlElement *root, SimpleShip *object = NULL);
	ComplexShip *						LoadComplexShipData(TiXmlElement *root, ComplexShip *object = NULL);
	ComplexShipSection *				LoadComplexShipSectionData(TiXmlElement *root, ComplexShipSection *object = NULL);
	Faction *							LoadFactionData(TiXmlElement *node, Faction *object = NULL);
	SpaceTurret *						LoadTurretData(TiXmlElement *node, SpaceTurret *object = NULL);
	BasicProjectileDefinition *			LoadBasicProjectileDefinitionData(TiXmlElement *node, BasicProjectileDefinition *object = NULL);
	SpaceProjectileDefinition *			LoadSpaceProjectileDefinitionData(TiXmlElement *node, SpaceProjectileDefinition *object = NULL);
	Engine *							LoadEngineData(TiXmlElement *node, Engine *object = NULL);

	// Load an instance of the given primary object class
	/****** TODO: Implement fully-templated model for Load<T>, Reload<T> which both rely on LoadData<T>.  Do as part of ****** 
	 ****** refactor DataInput.cpp -> proper central class.  This method is not currently used but shows the principle  ******/
	template <typename T>
	CMPINLINE Result					LoadObject(TiXmlElement *root)
	{
		// Create a new object and load all data from XML
		if (!root) return ErrorCodes::CannotLoadFromNullDataNode;
		T *object = new T();
		object = LoadPrimaryObjectData(root, object);

		// Validation; make sure key mandatory fields are supplied, and the code is not already in use, otherwise we will not create the ship
		if (object->GetCode() == NullString || D::GetDataRegister<T>().Exists(object->GetCode()))
		{
			SafeDelete(object);
			return ErrorCodes::CannotLoadSimpleShipDetailsWithDuplicateCode;
		}

		// Otherwise store in the central collection and return success
		D::GetDataRegister<T>().Store(object);
		return ErrorCodes::NoError;
	}

	Result								LoadSimpleShip(TiXmlElement *root);																
	Result								LoadComplexShip(TiXmlElement *root);
	Result 								LoadComplexShipSection(TiXmlElement *root);
	Result 								LoadFaction(TiXmlElement *node);
	Result 								LoadTurret(TiXmlElement *node);
	Result 								LoadBasicProjectileDefinition(TiXmlElement *node);
	Result 								LoadSpaceProjectileDefinition(TiXmlElement *node);
	Result 								LoadEngine(TiXmlElement *node);

	Result								ReloadSimpleShip(TiXmlElement *root);
	Result								ReloadComplexShip(TiXmlElement *root);
	Result 								ReloadComplexShipSection(TiXmlElement *root);
	Result 								ReloadFaction(TiXmlElement *node);
	Result 								ReloadTurret(TiXmlElement *node);
	Result 								ReloadBasicProjectileDefinition(TiXmlElement *node);
	Result 								ReloadSpaceProjectileDefinition(TiXmlElement *node);
	Result 								ReloadEngine(TiXmlElement *node);


	/*** End primary object classes which support runtime load/reload ***/


	// Loads an instance of a CS section (which should already be loaded) and adds it to the complex ship, assuming it is valid
	Result LoadComplexShipSectionInstance(TiXmlElement *root, ComplexShip *object);
	VariableSizeValue LoadSizeValue(TiXmlElement *node);
	Result LoadTurretLaunchers(TiXmlElement *node, SpaceTurret *turret);
	Result LoadProjectileLauncher(TiXmlElement *node);
	Hardpoint *LoadHardpoint(TiXmlElement *node);
	Result LoadSimpleShipLoadout(TiXmlElement *node);
	CompoundLoadoutMap *LoadCompoundLoadoutMap(TiXmlElement *node, SimpleShipLoadout *L, SimpleShip *targetshiptype);
	

	// Load geomatry, material, texture data
	Result LoadModelData(TiXmlElement *node);
	Result LoadMaterialData(TiXmlElement *node);
	Result LoadTextureData(TiXmlElement *node);
	Result LoadArticulatedModel(TiXmlElement *node);
	Result LoadArticulatedModelComponent(TiXmlElement *node, ArticulatedModel & parent_model);

	// Load an element in an OBB hierarchy; proceeds recursively until all data is read, or until the maximum depth is reached
	// 'obb' is the node to be updated
	void LoadCollisionOBB(iObject *object, TiXmlElement *node, OrientedBoundingBox & obb, bool isroot);

	// Load a set of instance-rendering flags, including a set of meta-flags which indicate which flags were actually loaded (and which are defaulted)
	InstanceFlags LoadInstanceFlags(TiXmlElement *node);

	// Load a set of collision spatial data.  Returns a flag indicating whether the data could be loaded
	CollisionSpatialDataF LoadCollisionSpatialData(TiXmlElement *node);

	// Post-processing methods for loaded game data, where data is dependent on other data (of the same or different class) that must be loaded first
	Result PostProcessResources(void);					// Resources must be interlinked with their dependencies/ingredients, and also productioncost initialisation
	Result PostProcessComplexShipTileData(void);		// Tiles must have their productioncost data initialised, with e.g. links to component resources/tile dependencies

	// Load bounding box data into the supplied bounding object
	void   LoadBoundingObjectData(TiXmlElement *node, BoundingObject *bounds, int boundscapacity);

	// Look ahead within a node definition to locate the given string property
	std::string LookaheadNodeTextField(TiXmlElement *node, const std::string & field);

	Result LoadNoiseResource(TiXmlElement *node);
	Result LoadFireEffect(TiXmlElement *node);
	Result LoadParticleEmitter(TiXmlElement *node);
	Result LoadSystem(TiXmlElement *node);

	Result LoadFont(TiXmlElement *node);
	Result LoadUILayout(TiXmlElement *node);
	Result LoadUIComponentGroup(TiXmlElement *node, Render2DGroup *group);
	Result LoadUIManagedControlDefinition(TiXmlElement *node);

	//Result LoadComplexShipSectionElementsIntoShip(ComplexShipDetails *ship, ComplexShipSection *section);
	Result LoadComplexShipElement(TiXmlElement *node, iSpaceObjectEnvironment *parent);
	
	Result LoadComplexShipTileClass(TiXmlElement *node);
	Result LoadComplexShipTileDefinition(TiXmlElement *node);
	Result LoadComplexShipTile(TiXmlElement *node, ComplexShipTile **pOutShipTile);
	Result LoadComplexShipTileCompoundModel(TiXmlElement *node, ComplexShipTileDefinition *tiledef);
	Result LoadDynamicTileSet(TiXmlElement *node);
	Result LoadDynamicTileSetOption(TiXmlElement *node, DynamicTileSet *pOutDTS);
	Result LoadElementStateDefinition(TiXmlElement *node, ElementStateDefinition & outStateDefinition);
	Result LoadElementStateDefinition(TiXmlElement *node, const INTVECTOR3 & element_size, ElementStateDefinition & outStateDefinition);

	Result LoadTerrainDefinition(TiXmlElement *node);
	Terrain *LoadTerrain(TiXmlElement *node);

	Result LoadDynamicTerrainDefinition(TiXmlElement *node);
	Result LoadDynamicTerrainStateDefinition(TiXmlElement *node, DynamicTerrainState & outStateDefinition);
	bool LoadUsableObjectData(TiXmlElement *node, HashVal key, UsableObject *object);
	DynamicTerrain *LoadDynamicTerrain(TiXmlElement *node);
	bool LoadDynamicTerrainInstanceData(TiXmlElement *node, HashVal hash, DynamicTerrain *terrain);

	ViewPortal LoadViewPortal(TiXmlElement *node);

	Result LoadResource(TiXmlElement *node);
	ProductionCost * LoadProductionCostData(TiXmlElement *node);

	Result LoadSkinnedModel(TiXmlElement *node);
	Result LoadActorAttributeGenerationData(TiXmlElement *node);
	Result LoadActor(TiXmlElement *node);

	Result LoadAndApplyTileConnectionState(TiXmlElement *node, TileConnections *pOutConnections);
	
	Result LoadModifier(TiXmlElement *node);

	Result LoadDamage(TiXmlElement *node, Damage & outDamage);
	Result LoadDamageSet(TiXmlElement *node, DamageSet & outDamageSet);
	Result LoadDamageResistance(TiXmlElement *node, DamageResistance & outDR);
	Result LoadDamageResistanceSet(TiXmlElement *node, DamageResistanceSet & outDRSet);

	// Load audio-related definitions
	Result LoadAudioItem(TiXmlElement *node);
	AudioParameters LoadAudioParameters(TiXmlElement *node);

	// Temporary buffer used to store non-standard sections during initialisation of the complex ship itself.  Cleared after each CS is loaded
	extern std::vector<ComplexShipSection*> __TemporaryCSSLoadingBuffer;
	ComplexShipSection *FindInTemporaryCSSBuffer(const std::string & code);

	// Data input logic will always maintain a record of the file currently being processed
	extern std::string FileBeingProcessed;
	CMPINLINE std::string GetFileCurrentlyBeingProcessed(void) { return FileBeingProcessed; }
	CMPINLINE void SetFileCurrentlyBeingProcessed(const std::string & file) { FileBeingProcessed = file; }

	// We have the option of specifying an entity type and code; during reload of resources, we will ONLY load
	// the item matching this combination and will ignore all others
	extern bool AllowReloadOfExistingEntities;
	extern HashVal ReloadOnlyType;
	extern std::string ReloadOnlyCode;
	CMPINLINE bool ReloadOfExistingResourcesIsPermitted(void) { return AllowReloadOfExistingEntities; }

	// Attempt to reload the single entity with given type/code, from the specified file.  All other definitions
	// in data file will be ignored.  File indices will not be followed
	Result ReloadEntityData(const std::string & filename, HashVal type, const std::string & code);


}}



#endif