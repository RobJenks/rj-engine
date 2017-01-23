#pragma once

#ifndef __DataInputH__
#define __DataInputH__

#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include "DX11_Core.h"
#include "XML\\tinyxml.h"
#include "ErrorCodes.h"
#include "GameDataExtern.h"
#include "HashFunctions.h"

using namespace std;
using namespace std::tr1;

class iStaticObject;
class iActiveObject;
class iSpaceObject;
class iEnvironmentObject;
class Model;
class ArticulatedModel;
class Hardpoint;
class OrientedBoundingBox;
class SimpleShip;
class ComplexShip;
class ComplexShipSection;
class ComplexShipElement;
class iSpaceObjectEnvironment;
class BoundingObject;
class SimpleShipLoadout;
class CompoundLoadoutMap;
class Render2DGroup;
class ProductionCost;
class StaticTerrain;
class EffectBase;
class FireEffect;
class TileConnections;

// This file contains no objects with special alignment requirements
namespace IO { namespace Data {

	Result LoadModelData(TiXmlElement *node);

	TiXmlDocument *LoadXMLDocument(const string &filename);
	Result LoadGameDataFile(const string &filename, bool follow_indices);
	Result LoadGameDataFile(const string &filename);
	Result LoadXMLFileIndex(TiXmlElement *node);
	Result LoadConfigFile(const string &filename);

	
	// Common methods to load intermediate class data
	bool LoadObjectData(TiXmlElement *node, HashVal hash, iObject *object);									// Loads iObject class data
	bool LoadDamageableEntityData(TiXmlElement *node, HashVal hash, iObject *object);						// Loads iTakesDamage class data
	bool LoadActiveObjectData(TiXmlElement *node, HashVal hash, iActiveObject *object);						// Loads iActiveObject class data
	bool LoadStaticObjectData(TiXmlElement *node, HashVal hash, iStaticObject *object);						// Loads iStaticObject class data
	bool LoadSpaceObjectData(TiXmlElement *node, HashVal hash, iSpaceObject *object);						// Loads iSpaceObject class data
	bool LoadEnvironmentObjectData(TiXmlElement *node, HashVal hash, iEnvironmentObject *object);			// Loads iEnvironmentObject class data
	bool LoadSpaceObjectEnvironmentData(TiXmlElement *node, HashVal hash, iSpaceObjectEnvironment *object);	// Loads iSpaceObjectEnvironment data
	bool LoadShipData(TiXmlElement *node, HashVal hash, Ship *object);										// Loads base ship class data

	// Load final class data, for non-inherited classes
	Result LoadSimpleShip(TiXmlElement *root);																// Loads simple ship data
	Result LoadComplexShip(TiXmlElement *root);																// Loads complex ship data
	Result LoadComplexShipSection(TiXmlElement *root);														// Loads complex ship section data


	// Loads an instance of a CS section (which should already be loaded) and adds it to the complex ship, assuming it is valid
	Result LoadComplexShipSectionInstance(TiXmlElement *root, ComplexShip *object);

	Result LoadFaction(TiXmlElement *node);
	
	Result LoadTurret(TiXmlElement *node);
	Result LoadTurretLaunchers(TiXmlElement *node, SpaceTurret *turret);
	Result LoadProjectileLauncher(TiXmlElement *node);
	Result LoadBasicProjectileDefinition(TiXmlElement *node);
	Result LoadSpaceProjectileDefinition(TiXmlElement *node);

	Hardpoint *LoadHardpoint(TiXmlElement *node);
	Result LoadSimpleShipLoadout(TiXmlElement *node);
	CompoundLoadoutMap *LoadCompoundLoadoutMap(TiXmlElement *node, SimpleShipLoadout *L, SimpleShip *targetshiptype);
	Result LoadEngine(TiXmlElement *node);

	// Load an articulated model
	Result LoadArticulatedModel(TiXmlElement *node);

	// Load an element in an OBB hierarchy; proceeds recursively until all data is read, or until the maximum depth is reached
	// 'obb' is the node to be updated
	void LoadCollisionOBB(iObject *object, TiXmlElement *node, OrientedBoundingBox & obb, bool isroot);

	Result LoadAllModelGeometry(void);								// Iterates through all model collections and loads models one-by-one
	Result LoadModelGeometry(Model *model);							// Method to load geometry for a specific model
	Result PostProcessAllModelGeometry(void);						// Runs post-load manipulation of model geometry as necessary

	// Post-processing methods for loaded game data, where data is dependent on other data (of the same or different class) that must be loaded first
	Result PostProcessResources(void);					// Resources must be interlinked with their dependencies/ingredients, and also productioncost initialisation
	Result PostProcessComplexShipTileData(void);		// Tiles must have their productioncost data initialised, with e.g. links to component resources/tile dependencies

	// Load bounding box data into the supplied bounding object
	void   LoadBoundingObjectData(TiXmlElement *node, BoundingObject *bounds, int boundscapacity);

	Result LoadFireEffect(TiXmlElement *node);
	Result LoadParticleEmitter(TiXmlElement *node);
	Result LoadSystem(TiXmlElement *node);

	Result LoadUILayout(TiXmlElement *node);
	Result LoadImage2DGroup(TiXmlElement *node, Render2DGroup *group);
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

	Result LoadStaticTerrainDefinition(TiXmlElement *node);
	StaticTerrain *LoadStaticTerrain(TiXmlElement *node);

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

	// Temporary buffer used to store non-standard sections during initialisation of the complex ship itself.  Cleared after each CS is loaded
	extern std::vector<ComplexShipSection*> __TemporaryCSSLoadingBuffer;
	ComplexShipSection *FindInTemporaryCSSBuffer(const std::string & code);
}}



#endif