#include "stdio.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <Windows.h>

#include "GameDataExtern.h"
#include "CoreEngine.h"
#include "Utility.h"
#include "HashFunctions.h"
#include "Logging.h"
#include "FileSystem.h"
#include "FileInput.h"
#include "Attachment.h"
#include "Model.h"
#include "ArticulatedModel.h"
#include "Modifiers.h"
#include "ModifierDetails.h"
#include "DynamicTerrainInteractionType.h"

#include "Hardpoint.h"
#include "Hardpoints.h"
#include "Hp.h"

#include "iObject.h"
#include "iActiveObject.h"
#include "iStaticObject.h"
#include "iSpaceObject.h"
#include "iEnvironmentObject.h"

#include "Equip.h"
#include "LoadoutMap.h"
#include "CompoundLoadoutMap.h"
#include "BoundingObject.h"
#include "SimpleShip.h"
#include "SimpleShipLoadout.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "ComplexShipElement.h"
#include "iSpaceObjectEnvironment.h"
#include "ComplexShipTileClass.h"
#include "ComplexShipTileDefinition.h"
#include "ComplexShipObjectClass.h"
#include "ComplexShipInfrastructure.h"
#include "ComplexShipTile.h"
#include "CSCorridorTile.h"
#include "DynamicTileSet.h"
#include "Terrain.h"
#include "TerrainDefinition.h"
#include "DynamicTerrain.h"
#include "DynamicTerrainDefinition.h"
#include "DynamicTerrainClass.h"
#include "ElementStateDefinition.h"
#include "CollisionSpatialDataF.h"

#include "SpaceTurret.h"
#include "ProjectileLauncher.h"
#include "BasicProjectileDefinition.h"
#include "SpaceProjectileDefinition.h"

#include "Resource.h"
#include "ResourceAmount.h"
#include "ProductionProgress.h"
#include "ProductionCost.h"
#include "CrewClassRequirement.h"
#include "ShipTileRequirement.h"

#include "ActorAttributes.h"
#include "ActorBase.h"
#include "Actor.h"

#include "Engine.h"
#include "GameUniverse.h"
#include "SpaceSystem.h"
#include "Faction.h"
#include "FactionManagerObject.h"
#include "EffectManager.h"
#include "FireEffect.h"
#include "ParticleEngine.h"
#include "ParticleEmitter.h"

#include "UserInterface.h"
#include "AudioManager.h"
#include "Render2DManager.h"
#include "Render2DGroup.h"
#include "RenderMouseEvent.h"
#include "Image2DRenderGroup.h"
#include "Image2DRenderGroupInstance.h"
#include "iUIComponent.h"
#include "UIComponentGroup.h"
#include "UIManagedControlDefinition.h"
#include "TextBlock.h"
#include "MultiLineTextBlock.h"
#include "UIButton.h"
#include "UITextBox.h"
#include "UIComboBox.h"
#include "Fonts.h"

#include "DataInput.h"


std::vector<ComplexShipSection*> IO::Data::__TemporaryCSSLoadingBuffer;


TiXmlDocument *IO::Data::LoadXMLDocument(const std::string &filename)
{
	// Initialise and open the document
	TiXmlDocument *doc = new TiXmlDocument(filename.c_str());
	if (!doc) return NULL;

	// Attempt to load the file to this document object
	if (doc->LoadFile())
		return doc;
	else {
		delete doc; return NULL;
	}
}

Result IO::Data::LoadGameDataFile(const std::string &filename) { return LoadGameDataFile(filename, true); }

Result IO::Data::LoadGameDataFile(const std::string &file, bool follow_indices)
{
	// Record the time taken to process this file; store the start time before beginning
	unsigned int processtime = (unsigned int)timeGetTime();

	// Build full filename
	if (file == NullString) return ErrorCodes::NullFilenamePointer;
	std::string & filename = BuildStrFilename(D::DATA, file);

	// Locate the file system object and make sure it is valid
	FileSystem::FileSystemObjectType fso_type = FileSystem::GetFileSystemObjectType(filename.c_str());
	if (fso_type == FileSystem::FileSystemObjectType::FSO_None)
	{
		Game::Log << LOG_WARN << "Specified data file/directory does not exist (" << file << ")\n";
		return ErrorCodes::FileDoesNotExist;
	}

	// If we have been passed a directory, process each file within it in turn.  We do NOT recurse 
	// into subdirectories for safety
	if (fso_type == FileSystem::FileSystemObjectType::FSO_Directory)
	{
		std::vector<std::string> files;
		FileSystem::GetFileSystemObjects(filename, true, false, files);

		Game::Log << LOG_INFO << "Processing directory \"" << file << "\" (" << files.size() << " files)\n";
		for (std::string f : files)
		{
			// We need to test for & propogate any circular file indices to ensure they don't cause an issue
			if (LoadGameDataFile(concat(file)("\\")(f).str(), follow_indices) == ErrorCodes::ForceTerminatedInfiniteCircularFileIndices)
				return ErrorCodes::ForceTerminatedInfiniteCircularFileIndices;
		}
	}
	else
	{
		// This is a file.  Attempt to load the XML data file
		Result res = ErrorCodes::NoError;
		TiXmlDocument *doc = IO::Data::LoadXMLDocument(filename);
		if (doc == NULL) return ErrorCodes::CannotLoadXMLDocument;

		// The first (and only) root note should be a "GameData" node; if not then stop
		TiXmlElement *root = doc->FirstChildElement();
		if (root == NULL) { delete doc; return ErrorCodes::CannotFindXMLRoot; }

		// Make sure the root name is valid
		std::string rname = root->Value(); StrLowerC(rname);
		if (!(rname == D::NODE_GameData)) { delete doc; return ErrorCodes::InvalidXMLRootNode; }

		// Now iterate through each child element in turn; these elements at level one should denote the type of object
		std::string name = "";
		TiXmlElement *child = root->FirstChildElement();
		for (child; child; child = child->NextSiblingElement())
		{
			// Test the type of this node
			// TODO: Add error handling if a function returns =/= 0
			name = child->Value(); StrLowerC(name);

			if (name == D::NODE_FileIndex) {
				res = IO::Data::LoadXMLFileIndex(child);

				// If we caught and terminated an infinite file loop we need to propogate the error backwards to stop it simply repeating
				if (res == ErrorCodes::ForceTerminatedInfiniteCircularFileIndices)
					return ErrorCodes::ForceTerminatedInfiniteCircularFileIndices;
			}
			else if (name == D::NODE_SimpleShip) {
				res = IO::Data::LoadSimpleShip(child);
			} else if (name == D::NODE_SimpleShipLoadout) {
				res = IO::Data::LoadSimpleShipLoadout(child);
			} else if (name == D::NODE_ComplexShip) {
				res = IO::Data::LoadComplexShip(child);
			} else if (name == D::NODE_ComplexShipSection) {
				res = IO::Data::LoadComplexShipSection(child);
			} else if (name == D::NODE_Engine) {
				res = IO::Data::LoadEngine(child);
			} else if (name == D::NODE_System) {
				res = IO::Data::LoadSystem(child);
			} else if (name == D::NODE_FireEffect) {
				res = IO::Data::LoadFireEffect(child);
			} else if (name == D::NODE_ParticleEmitter) {
				res = IO::Data::LoadParticleEmitter(child);
			} else if (name == D::NODE_UILayout) {
				res = IO::Data::LoadUILayout(child);
			} else if (name == D::NODE_Model) {
				res = IO::Data::LoadModelData(child);
			} else if (name == D::NODE_ArticulatedModel) {
				res = IO::Data::LoadArticulatedModel(child);
			} else if (name == D::NODE_UIManagedControlDefinition) {
				res = IO::Data::LoadUIManagedControlDefinition(child);
			} else if (name == D::NODE_ComplexShipTileClass) {
				res = IO::Data::LoadComplexShipTileClass(child);
			} else if (name == D::NODE_ComplexShipTileDefinition) {
				res = IO::Data::LoadComplexShipTileDefinition(child);
			} else if (name == D::NODE_Resource) {
				res = IO::Data::LoadResource(child);
			} else if (name == D::NODE_SkinnedModel) {
				res = IO::Data::LoadSkinnedModel(child);
			} else if (name == D::NODE_ActorAttributeGeneration) {
				res = IO::Data::LoadActorAttributeGenerationData(child);
			} else if (name == D::NODE_ActorBase) {
				res = IO::Data::LoadActor(child);
			} else if (name == D::NODE_TerrainDefinition) {
				res = IO::Data::LoadTerrainDefinition(child);
			} else if (name == D::NODE_DynamicTerrainDefinition) { 
				res = IO::Data::LoadDynamicTerrainDefinition(child);
			} else if (name == D::NODE_Faction) {
				res = IO::Data::LoadFaction(child);
			} else if (name == D::NODE_Turret) {
				res = IO::Data::LoadTurret(child);
			} else if (name == D::NODE_ProjectileLauncher) {
				res = IO::Data::LoadProjectileLauncher(child);
			} else if (name == D::NODE_BasicProjectileDefinition) {
				res = IO::Data::LoadBasicProjectileDefinition(child);
			} else if (name == D::NODE_SpaceProjectileDefinition) {
				res = IO::Data::LoadSpaceProjectileDefinition(child);
			} else if (name == D::NODE_DynamicTileSet) {
				res = IO::Data::LoadDynamicTileSet(child);
			} else if (name == D::NODE_ModifierDetails) {
				res = IO::Data::LoadModifier(child);
			} else if (name == D::NODE_Audio) { 
				res = IO::Data::LoadAudioItem(child);
			} else {
				// Unknown level one node type
				res = ErrorCodes::UnknownDataNodeType;
			}

			// If we encountered any errors loading this node then log them and continue
			if (res != ErrorCodes::NoError)
			{
				Game::Log << LOG_ERROR << "Error " << res << " loading \"" << name << "\" from game data file \"" << file << "\"\n";
				res = ErrorCodes::NoError;
			}
		}

		// Release any resources used to parse the XML data
		if (doc) SafeDelete(doc);
	}

	// Calculate the total time taken to process this file and log it
	processtime = ((unsigned int)timeGetTime() - processtime);
	Game::Log << LOG_INFO << "Game data " << StrLower(FileSystem::FileSystemObjectName(fso_type)) << " \"" << file << "\" processed [" << processtime << "ms]\n";

	// Return success
	return ErrorCodes::NoError;
}

Result IO::Data::LoadXMLFileIndex(TiXmlElement *node) 
{
	// Maintain an invocation counter to catch infinite circular links between data files
	static int _INVOKE_COUNT = 0;
	std::string name; Result res;

	// If we have hit the invocation limit we are most likely in an infinite circular loop
	if (++_INVOKE_COUNT > Game::C_DATA_LOAD_RECURSION_LIMIT) 
		return ErrorCodes::ForceTerminatedInfiniteCircularFileIndices;

	// Iterate through the attributes of this node looking for a file path
	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = attr->Name(); StrLowerC(name);

		// If this is a file path we want to follow the link
		if (name == "file") {
			// Retrieve the filename and attempt to parse the target file
			res = IO::Data::LoadGameDataFile(attr->Value(), true);			// follow_indices must have been set to true for us to actually be here

			// If we caught and terminated an infinite file loop we need to propogate the error backwards to stop it simply repeating
			if (res == ErrorCodes::ForceTerminatedInfiniteCircularFileIndices)
				return ErrorCodes::ForceTerminatedInfiniteCircularFileIndices;
			
		}
		attr = attr->Next();	
	}

	// If we reach this point we have processed the file index correctly and are not over the invocation limit.
	// Reduce the count by one to indicate that we have dropped a level of recursion.
	--_INVOKE_COUNT;

	// Return success
	return ErrorCodes::NoError;
}

// Load a configuration file
Result IO::Data::LoadConfigFile(const std::string &filename)
{
	// Maintain a recursion counter to prevent infinite loops when loading configuration
	static int RECURSION_DEPTH = 0;

	// Record the time taken to process this file; store the start time before beginning
	unsigned int processtime = (unsigned int)timeGetTime();

	// Attempt to load the XML config file
	Result res = ErrorCodes::NoError;
	TiXmlDocument *doc = IO::Data::LoadXMLDocument(Game::ExePath + "\\" + filename);
	if (doc == NULL) return ErrorCodes::CannotLoadXMLDocument;

	// The first (and only) root node should be a "Config" node; if not then stop
	TiXmlElement *root = doc->FirstChildElement();
	if (root == NULL) { delete doc; return ErrorCodes::CannotFindXMLRoot; }

	// Make sure the root name is valid
	std::string rname = root->Value(); StrLowerC(rname);
	if (!(rname == D::NODE_Config)) { delete doc; return ErrorCodes::InvalidXMLRootNode; }

	// The file is valid; increment the recursion counter to track the processing, and make sure we aren't in an infinite loop
	if (++RECURSION_DEPTH > Game::C_CONFIG_LOAD_RECURSION_LIMIT)
	{
		delete doc;
		return ErrorCodes::ForceTerminatedInfiniteCircularFileIndices;
	}

	// Now iterate through each child element in turn and pull the relevant configuration
	std::string name = "";
	TiXmlElement *child = root->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Test the type of this node
		name = child->Value(); StrLowerC(name);

		if (name == D::NODE_FileIndex) {
			const char *file = child->Attribute("file"); if (!file) continue;
			std::string sfile = file;
			res = IO::Data::LoadConfigFile(sfile);

			// If we caught and terminated an infinite file loop we need to propogate the error backwards to stop it simply repeating
			if (res == ErrorCodes::ForceTerminatedInfiniteCircularFileIndices)
				return ErrorCodes::ForceTerminatedInfiniteCircularFileIndices;
		}
		else if (name == "data") {
			const char *path = child->Attribute("path");
			if (path && FileSystem::DirectoryExists(path))
			{
				// Store the data path in string form; other dependent fields will be initialised in a post-processing step
				D::DATA_S = std::string(path);
			}
		}
		else if (name == "screenresolution") {
			int x = 1024, y = 768, hz = 0;
			child->Attribute("x", &x);
			child->Attribute("y", &y);
			child->Attribute("hz", &hz);
			Game::ScreenWidth = x;
			Game::ScreenHeight = y;
			if (hz > 0) Game::ScreenRefresh = hz;
		}
		else if (name == "softwarerasterizeroverride")
		{
			const char *enabled = child->Attribute("enable");
			if (enabled && strcmp(enabled, "true") == 0)
				Game::ForceWARPRenderDevice = true;
		}
	}

	// Decrement the recursion counter now that this file has been fully processed
	--RECURSION_DEPTH;

	// Calculate the total time taken to process this file and log it
	processtime = ((unsigned int)timeGetTime() - processtime);
	Game::Log << LOG_INFO << "Game config file \"" << filename << "\" processed [" << processtime << "ms]\n";

	// Dispose of memory no longer required and return success
	if (doc) delete doc;
	return ErrorCodes::NoError;
}

Result IO::Data::LoadModelData(TiXmlElement *node)
{
	Model *model;
	Model::ModelClass mclass;
	std::string key, code, type, fname, tex, val;
	XMFLOAT3 acteffsize, effsize;
	INTVECTOR3 elsize; bool no_centre = false;
	std::vector<CollisionSpatialDataF> collision;
	HashVal hash;

	// Set defaults before loading the model
	code = type = fname = tex = "";
	acteffsize = effsize = XMFLOAT3(0.0f, 0.0f, 0.0f);
	elsize = NULL_INTVECTOR3;

	// Look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Retrieve the xml node key and hash it for more efficient comparison
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		// Test the hash against each expected field
		if (hash == HashedStrings::H_Code) {								/* Uniquely-identifying string code */
			code = child->GetText();
			StrLowerC(code);
		}
		else if (hash == HashedStrings::H_Type) {							/* The model class for this object */
			type = child->GetText();
			StrLowerC(type);
		}
		else if (hash == HashedStrings::H_Filename) {						/* Filename of the ship model file */
			fname = child->GetText();
			StrLowerC(fname);
		}
		else if (hash == HashedStrings::H_Texture) {						/* Filename of the associated model texture */
			tex = child->GetText();
			StrLowerC(tex);
		}
		else if (hash == HashedStrings::H_NoModelCentering) {				/* Flag that overrides the default behaviour of centring all models about the origin */
			val = child->GetText(); StrLowerC(val);
			no_centre = (val == "true");
		}
		else if (hash == HashedStrings::H_EffectiveSize) {					/* Effective size (i.e. not just vertex max-min) for the model */
			effsize = IO::GetFloat3FromAttr(child);
		}
		else if (hash == HashedStrings::H_ActualEffectiveSize) {			/* Actual (i.e. in-game) effective size.  Determines scaling if set */
			acteffsize = IO::GetFloat3FromAttr(child);
		}
		else if (hash == HashedStrings::H_ElementSize) {					/* Mapping to element dimensions; optional, and used to scale to fit elements by load post-processing */
			elsize = IO::GetInt3CoordinatesFromAttr(child);	
		}
		else if (hash == HashedStrings::H_Collision) {						/* List of collision objects attached to this model */
			collision.push_back(IO::Data::LoadCollisionSpatialData(child));
		}
	}

	// Make sure we have all mandatory fields 
	if (code == NullString || type == NullString || fname == NullString || tex == NullString) return ErrorCodes::InsufficientDataToLoadModel;

	// We need to take action depending on the type of model, so identify the model class here
	mclass = Model::DetermineModelClass(type);

	// Check whether we have an existing model; if we do, return an error since we will not load the same model twice
	model = Model::GetModel(code);
	if (model != NULL) return ErrorCodes::CannotLoadModelWhereDuplicateAlreadyExists;
	
	// Construct full filenames from the info specified
	std::string filename = BuildStrFilename(D::DATA, fname);
	std::string texture = BuildStrFilename(D::IMAGE_DATA, tex);

	// Otherwise create a new model here
	model = new Model();
	model->SetCode(code);
	model->SetModelClass(mclass);
	model->SetFilename(filename);
	model->SetTextureFilename(texture);
	model->SetEffectiveModelSize(effsize);
	model->SetElementSize(elsize);
	model->SetCentredAboutOrigin(!no_centre);
	model->SetCollisionData(std::move(collision));

	// Mark as a 'standard' model, i.e. one that will be shared as a template
	// between multiple entities for performance reasons.  The entity only acquires an individual model
	// if its geometry is changed / deformed in some way.  Setting this flag means that when an object
	// with the model is deallocated it will not attempt to deallocate the model data, preserving it for
	// other entities
	model->SetStandardModel(true);
	
	// If an effective model size is not specified, take the actual extent calculated from vertex data as a default
	if (effsize.x <= Game::C_EPSILON || effsize.y <= Game::C_EPSILON || effsize.z <= Game::C_EPSILON)
		model->SetEffectiveModelSize(model->GetModelSize());
	
	// Attempt to set the actual model size; method will handle missing/incomplete parameters itself, so pass whatever we have (incl 0,0,0 as default)
	model->SetActualModelSize(acteffsize);

	// Load the model geometry immediately
	Result result = LoadModelGeometry(model);
	if (result != ErrorCodes::NoError)
	{
		Game::Log << LOG_WARN << "Could not load geometry for model \"" << model->GetCode() << "\" [" << result << "]\n";
	}

	// Test whether this model has an element size specified; if so, override all size data from the xml data or geometry
	elsize = model->GetElementSize();
	if (elsize.x > 0 && elsize.y > 0 && elsize.z > 0)
	{
		// An element size has been specified, so scale the mesh to be mapped onto the specified number of standard-sized game elements
		model->SetActualModelSize(Game::ElementLocationToPhysicalPositionF(elsize));
	}

	// Add this model to the relevant static collection and return success
	Model::AddModel(model);
	return ErrorCodes::NoError;
}

// Loads iObject class data, returning true if the value was loaded
bool IO::Data::LoadObjectData(TiXmlElement *node, HashVal hash, iObject *object)
{
	// Compare the hash against all iObject-related fields
	if		(hash == HashedStrings::H_Code)							object->SetCode(GetLCString(node));
	else if (hash == HashedStrings::H_Name)							object->SetName(node->GetText());
	else if (hash == HashedStrings::H_StandardObject)				object->SetIsStandardObject(GetBoolValue(node));
	else if (hash == HashedStrings::H_Position)						object->SetPosition(IO::GetVector3FromAttr(node));
	else if (hash == HashedStrings::H_Orientation)					object->SetOrientation(IO::GetQuaternionFromAttr(node));
	else if (hash == HashedStrings::H_Model)
	{
		___tmp_loading_string = node->GetText(); StrLowerC(___tmp_loading_string);
		object->SetModel(Model::GetModel(___tmp_loading_string));
	}
	else if (hash == HashedStrings::H_Visible)						object->SetIsVisible(GetBoolValue(node));
	//else if (hash == HashedStrings::H_VisibilityTestingMode)		object->SetVisibilityTestingMode(TranslateVisibilityModeFromString(node->GetText()));
	else if (hash == HashedStrings::H_SimulationState)				object->SetSimulationState(iObject::TranslateSimulationStateFromString(node->GetText()));	// Takes immediate effect
	else if (hash == HashedStrings::H_CollisionMode)				object->SetCollisionMode(Game::TranslateCollisionModeFromString(node->GetText()));
	else if (hash == HashedStrings::H_CollisionOBB)					LoadCollisionOBB(object, node, object->CollisionOBB, true);
	else if (hash == HashedStrings::H_AmbientAudio)					object->SetAmbientAudio(LoadAudioParameters(node));


	// Otherwise check against any superclasses
	else if (LoadDamageableEntityData(node, hash, object))			return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the fields.  So return true here.
	return true;
}

// Loads iTakesDamage class data, returning true if the value was loaded
bool IO::Data::LoadDamageableEntityData(TiXmlElement *node, HashVal hash, iObject *object)
{
	// Compare the hash against all iTakesDamage-related fields
	if (hash == HashedStrings::H_MaxHealth)
	{
		float health = GetFloatValue(node);
		object->SetMaxHealth(health);
		object->SetHealth(health);
	}
	else if (hash == HashedStrings::H_Health)						object->SetHealth(GetFloatValue(node));		// Note: should be set AFTER max health if Health > Old_MaxHealth
	else if (hash == HashedStrings::H_IsInvulnerable)				object->SetInvulnerabilityFlag(GetBoolValue(node));
	else if (hash == HashedStrings::H_DamageResistanceSet)			LoadDamageResistanceSet(node, object->DamageResistanceData());

	// Otherwise check against any superclasses
	/* No superclasses */

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the fields.  So return true here.
	return true;
}


// Loads iActiveObject class data, returning true if the value was loaded
bool IO::Data::LoadActiveObjectData(TiXmlElement *node, HashVal hash, iActiveObject *object)
{	
	// Compare the hash against all iActiveObject-related fields
	if (hash == HashedStrings::H_Mass)								object->SetMass(GetFloatValue(node));

	/* Now pass to each direct superclass if we didn't match any field in this class */
	else if (LoadObjectData(node, hash, object))					return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the direct class fields.  So return true here.
	return true;
	
}

// Loads iStaticObject class data, returning true if the value was loaded
bool IO::Data::LoadStaticObjectData(TiXmlElement *node, HashVal hash, iStaticObject *object)
{
	// Compare the hash against all iStaticObject-related fields
	// (No iStaticObject-specific fields at this point)

	/* Now pass to each direct superclass if we didn't match any field in this class */
	if (LoadObjectData(node, hash, object))							return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;
	
	// If we didn't hit the "else" clause, and return false, we must have matched one of the direct class fields.  So return true here.
	return true;
}

bool IO::Data::LoadSpaceObjectData(TiXmlElement *node, HashVal hash, iSpaceObject *object)
{
	// Compare the hash against all iSpaceObject-related fields
	// (No iSpaceObject-specific fields at this point)

	/* Now pass to each direct superclass if we didn't match any field in this class */
	if (LoadActiveObjectData(node, hash, object))					return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the direct class fields.  So return true here.
	return true;
}

bool IO::Data::LoadEnvironmentObjectData(TiXmlElement *node, HashVal hash, iEnvironmentObject *object)
{
	// Compare the hash against all iEnvironmentObject-related fields
	// (No iEnvironmentObject-specific fields at this point)

	/* Now pass to each direct superclass if we didn't match any field in this class */
	if (LoadActiveObjectData(node, hash, object))					return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the direct class fields.  So return true here.
	return true;
}

bool IO::Data::LoadSpaceObjectEnvironmentData(TiXmlElement *node, HashVal hash, iSpaceObjectEnvironment *object)
{
	// Handle each direct superclass first
	// (No direct superclasses)

	// Compare the hash against all iContainsComplexShipElements-related fields
	if (hash == HashedStrings::H_ElementSize)
	{
		// Initialise all elements when the element size is set.  This means element size MUST be set before elements are defined
		object->InitialiseElements(IO::GetInt3CoordinatesFromAttr(node), false);
	}
	else if (hash == HashedStrings::H_ComplexShipElement)			LoadComplexShipElement(node, object);
	else if (hash == HashedStrings::H_Terrain)
	{
		Terrain *t = LoadTerrain(node);
		if (t) object->AddTerrainObject(t);
	}
	else if (hash == HashedStrings::H_DynamicTerrain)
	{
		DynamicTerrain *t = LoadDynamicTerrain(node);
		if (t) object->AddTerrainObject(t);
	}

	/* Now pass to each direct superclass if we didn't match any field in this class */
	// (No direct superclasses for the element container class)

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the direct class fields.  So return true here.
	return true;
	
}

bool IO::Data::LoadShipData(TiXmlElement *node, HashVal hash, Ship *object)
{
	// Compare the hash against all ship-related fields
	if		(hash == HashedStrings::H_VelocityLimit)				object->VelocityLimit.BaseValue = GetFloatValue(node);
	else if (hash == HashedStrings::H_AngularVelocityLimit)			object->AngularVelocityLimit.BaseValue = GetFloatValue(node);
	else if (hash == HashedStrings::H_BrakeFactor)					object->BrakeFactor.BaseValue = GetFloatValue(node);
	else if (hash == HashedStrings::H_TurnAngle)					object->TurnAngle.BaseValue = GetFloatValue(node);
	else if (hash == HashedStrings::H_TurnRate)						object->TurnRate.BaseValue = GetFloatValue(node);
	else if (hash == HashedStrings::H_BankRate)						object->BankRate.BaseValue = GetFloatValue(node);
	else if (hash == HashedStrings::H_BankExtent)					object->SetBankExtent(IO::GetVector3FromAttr(node));
	else if (hash == HashedStrings::H_DefaultLoadout)				object->SetDefaultLoadout(GetLCString(node));
	else if (hash == HashedStrings::H_Mass)							object->SetBaseMass(GetFloatValue(node));	// Overrides the iActiveObject behaviour, since ships have base & overall mass

	/* Now pass to each direct superclass if we didn't match any field in this class */
	else if (LoadSpaceObjectData(node, hash, object))				return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the fields.  So return true here.
	return true;

}

// Loads data for an object implementing iContainsTurrets
bool IO::Data::LoadHardpointContainerData(TiXmlElement *node, HashVal hash, iContainsHardpoints *object)
{
	return false;
}

Result IO::Data::LoadSimpleShip(TiXmlElement *root)
{
	// Create a new SimpleShip instance to hold the data
	SimpleShip *object = new SimpleShip();
	object->SetShipClass(Ship::ShipClass::Simple);

	// Suspend updates while loading the data
	object->GetHardpoints().SuspendUpdates();

	// Now look at each child element in turn
	std::string key; HashVal hash;
	TiXmlElement *node = root->FirstChildElement();
	for (node; node; node = node->NextSiblingElement())
	{
		// Hash the value for more efficient lookups
		key = node->Value(); StrLowerC(key);
		hash = HashString(key);

		// Handle each direct superclass first
		if (LoadShipData(node, hash, object))
			;

		// Test for fields directly within this class
		else
		{
			// Compare the hash against all SimpleShip-related fields
			if		(hash == HashedStrings::H_CameraPosition)				object->CameraPosition = IO::GetVector3FromAttr(node);
			else if (hash == HashedStrings::H_CameraRotation)				object->CameraRotation = (IO::GetVector3FromAttr(node) * PIBY180);	// Convert degrees > radians
			else if (hash == HashedStrings::H_CameraElasticity)				object->CameraElasticity = GetFloatValue(node);
			else if (hash == HashedStrings::H_Hardpoint)
			{
				Hardpoint *h = LoadHardpoint(node);
				if (h) object->GetHardpoints().AddHardpoint(h);
			}
		}
	}

	// Validation; make sure key mandatory fields are supplied, and the code is not already in use, otherwise we will not create the ship
	if (object->GetCode() == NullString || D::SimpleShips.Exists(object->GetCode()))
	{
		SafeDelete(object);
		return ErrorCodes::CannotLoadSimpleShipDetailsWithDuplicateCode;
	}

	// Otherwise, resume updates, recalculate the ship data and store in the central collection
	object->GetHardpoints().ResumeUpdates();
	object->RecalculateAllShipData();
	D::SimpleShips.Store(object);

	// Return success
	return ErrorCodes::NoError;
}


Result IO::Data::LoadComplexShip(TiXmlElement *root)
{
	// Create a new ComplexShip instance to hold the data
	ComplexShip *object = new ComplexShip();
	object->SetShipClass(Ship::ShipClass::Complex);

	// Suspend updates while data is loaded
	object->SuspendUpdates();
	object->GetHardpoints().SuspendUpdates();

	// Now look at each node element in turn
	std::string key; HashVal hash; Result result; ComplexShipTile *tile;
	TiXmlElement *node = root->FirstChildElement();
	for (node; node; node = node->NextSiblingElement())
	{
		// Hash the value for more efficient lookups
		key = node->Value(); StrLowerC(key);
		hash = HashString(key);

		// Handle each direct superclass first
		if (LoadShipData(node, hash, object))
			;
		else if (LoadSpaceObjectEnvironmentData(node, hash, object))
			;

		// Test for fields directly within this class
		else
		{
			// Compare the hash against all ComplexShip-related fields
			if (hash == HashedStrings::H_ComplexShipSectionInstance)	result = LoadComplexShipSectionInstance(node, object);
			else if (hash == HashedStrings::H_ComplexShipTile)
			{
				result = LoadComplexShipTile(node, &tile);
				if (tile && (result == ErrorCodes::NoError))
				{
					object->AddTile(&tile);
				}
			}
		}
	}

	// Validation; make sure key mandatory fields are supplied, and the code is not already in use, otherwise we will not create the ship
	if (object->GetCode() == NullString || D::ComplexShips.Exists(object->GetCode()))
	{
		SafeDelete(object);
		__TemporaryCSSLoadingBuffer.clear();
		return ErrorCodes::CannotLoadComplexShipDetailsWithDuplicateCode;
	}

	// Otherwise, resume updates, which will calculate the ship data, and store in the central collection
	object->ResumeUpdates();
	object->GetHardpoints().ResumeUpdates();
	D::ComplexShips.Store(object);

	// Clear the temporary CS section loading buffer; sections are only persisted for loading into the next ship
	__TemporaryCSSLoadingBuffer.clear();

	// Return success
	return ErrorCodes::NoError;
}

// Loads a ship section from file, storing it in the central collection
Result IO::Data::LoadComplexShipSection(TiXmlElement *root)
{
	// Create a new ComplexShipSection instance to hold the data
	ComplexShipSection *object = new ComplexShipSection();

	// Suspend updates while the object is being loaded
	object->SuspendUpdates();

	// Now look at each node element in turn
	std::string key; HashVal hash;
	TiXmlElement *node = root->FirstChildElement();
	for (node; node; node = node->NextSiblingElement())
	{
		// Hash the value for more efficient lookups
		key = node->Value(); StrLowerC(key);
		hash = HashString(key);

		// Handle each direct superclass first
		if (LoadSpaceObjectData(node, hash, object))
			;

		// Test for fields directly within this class
		else
		{
			// Compare the hash against all ComplexShipSection-related fields
			if (hash == HashedStrings::H_ElementSize)						object->ResizeSection(IO::GetInt3CoordinatesFromAttr(node));
			else if (hash == HashedStrings::H_VelocityLimit)				object->SetVelocityLimit(GetFloatValue(node));
			else if (hash == HashedStrings::H_AngularVelocityLimit)			object->SetAngularVelocityLimit(GetFloatValue(node));
			else if (hash == HashedStrings::H_BrakeFactor)					object->SetBrakeFactor(GetFloatValue(node));
			else if (hash == HashedStrings::H_TurnAngle)					object->SetTurnAngle(GetFloatValue(node));
			else if (hash == HashedStrings::H_TurnRate)						object->SetTurnRate(GetFloatValue(node));
			else if (hash == HashedStrings::H_BankRate)						object->SetBankRate(GetFloatValue(node));
			else if (hash == HashedStrings::H_BankExtent)					object->SetBankExtents(IO::GetVector3FromAttr(node) * PIBY180);	// Convert degrees > radians
			else if (hash == HashedStrings::H_PreviewImage)					object->SetPreviewImage(node->GetText());
			else if (hash == HashedStrings::H_Hardpoint)
			{
				Hardpoint *h = LoadHardpoint(node);
				if (h) object->AddHardpoint(h);
			}
			else if (hash == HashedStrings::H_ElementStateDefinition) {
				// NOTE: Definition should be set AFTER the tile element size has been defined, since it is passed
				// into the method below.  Or if not, area size can be specified within the state definition, but 
				// this would then require definition in two places
				LoadElementStateDefinition(node, object->GetElementSize(), object->DefaultElementState);
			}
		}
	}

	// Validation; make sure key mandatory fields are supplied, and the code is not already in use, otherwise we will not create the ship
	if (object->GetCode() == NullString || D::ComplexShipSections.Exists(object->GetCode()))
	{
		SafeDelete(object);
		return ErrorCodes::CannotLoadCSSectionDetailsWithDuplicateCode;
	}

	// Otherwise, resume all updates and recalculate the section data 
	object->ResumeUpdates();
	object->RecalculateShipDataFromCurrentState();

	// Store in either the central collection or the temporary loading buffer, depending on whether this is a standard section
	if (object->IsStandardObject())
	{
		// If this is a standard section, simply add to the central data collection
		D::ComplexShipSections.Store(object);
	}
	else
	{
		// If this is non-standard it is only valid for the current ship.  Store in the temporary buffer for the duration of the next CS load
		__TemporaryCSSLoadingBuffer.push_back(object);
	}

	// Return success
	return ErrorCodes::NoError;
}

// Loads an instance of a CS section (which should already be loaded) and adds it to the complex ship, assuming it is valid
Result IO::Data::LoadComplexShipSectionInstance(TiXmlElement *root, ComplexShip *object)
{
	// Look at each node element in turn, storing required values for now
	std::string key; HashVal hash; 
	std::string a_code = ""; INTVECTOR3 a_elpos = NULL_INTVECTOR3; 
	Rotation90Degree a_rot = Rotation90Degree::Rotate0;
	TiXmlElement *node = root->FirstChildElement();
	for (node; node; node = node->NextSiblingElement())
	{
		// Hash the value for more efficient lookups
		key = node->Value(); StrLowerC(key);
		hash = HashString(key);

		// Test for each required field
		if		(hash == HashedStrings::H_Code)				a_code = node->GetText();
		else if (hash == HashedStrings::H_ElementLocation)	a_elpos = IO::GetInt3CoordinatesFromAttr(node);
		else if (hash == HashedStrings::H_Rotation)			a_rot = TranslateRotation90Degree(node->GetText());
	}

	// Make sure we have mandatory fields
	if (a_code == NullString) return ErrorCodes::CannotLoadInstanceOfNullCSSection;

	// (Check for validity, e.g. make sure no overlap with existing section)

	// Check whether a section exists with this code
	StrLowerC(a_code);
	ComplexShipSection *sec = NULL;
	if (D::ComplexShipSections.Exists(a_code))
	{
		// The section exists in the central collection, so create an instance from it now
		sec = ComplexShipSection::Create(a_code);
	}
	else
	{
		// The section does not exist in the central collection; check whether it is being maintained for temporary-loading
		ComplexShipSection *temp_sec = FindInTemporaryCSSBuffer(a_code);
		if (temp_sec)
		{
			// We do have a matching section held in the temporary buffer, so use this
			sec = ComplexShipSection::Create(temp_sec);
		}
		else
		{
			// No matching section; return error here
			return ErrorCodes::CannotLoadCCSectionInstanceWithInvalidCode;
		}
	}

	// Double-check that the section was successfully created
	if (!sec) return ErrorCodes::UnknownErrorInstantiatingCSSection;

	// This is a a valid section, so set the details now 
	sec->SetElementLocation(a_elpos);
	sec->RotateSection(a_rot);

	// Add this section to the ship and return the result
	Result result = object->AddShipSection(sec);
	return result;
}



void IO::Data::LoadBoundingObjectData(TiXmlElement *node, BoundingObject *bounds, int boundscapacity)
{
	// Make sure this is a valid XML node
	if (!node || !bounds) return;

	// Validate that the ID of this bounding object is accomodated in the allocated memory
	int id = -1;
	node->Attribute("id", &id);
	if (id < 0 || id >= boundscapacity) return;		// Since the ID is outside the range of allocated memory

	// Get a pointer to the new bounding object
	BoundingObject *obj = &(bounds[id]);

	// Get the type of bounding object that this node defines
	const char *c_type = node->Attribute("type");
	if (!c_type) return;
	std::string type = c_type;

	// Based on the type, load the other parameters and create the object
	if (type == "point")
	{
		// Point type has no parameters
		obj->CreatePointBound();
	}
	else if (type == "cube")
	{
		// Cube objects require a radius to be specified
		double rad = 0.0f;
		node->Attribute("radius", &rad);
		obj->CreateCubeBound((float)rad);
	}
	else if (type == "sphere")
	{
		// Sphere objects also only require a radius
		double rad = 0.0f;
		node->Attribute("radius", &rad);
		obj->CreateSphereBound((float)rad);
	}
	else if (type == "cuboid")
	{
		// Cuboids require the (symmetric) size in each dimension
		double sx = 0.0f, sy = 0.0f, sz = 0.0f;
		node->Attribute("sizex", &sx);
		node->Attribute("sizey", &sy);
		node->Attribute("sizez", &sz);
		obj->CreateCuboidBound((float)sx, (float)sy, (float)sz);
	}
	else { delete obj; }	// If not a valid bounding object type then deallocate the memory

}


Result IO::Data::LoadComplexShipElement(TiXmlElement *node, iSpaceObjectEnvironment *parent)
{
	const char *cval;
	std::string key, val;
	HashVal hash;
	
	// Make sure we have a valid section of document to work on
	if (!node || !parent) return ErrorCodes::NullComplexShipElementNodeProvided;
	
	// Get the element ID from this top-level node and validate it
	const char *c_id = node->Attribute("id"); 
	if (!c_id) return ErrorCodes::CannotLoadComplexShipElementWithInvalidID;
	
	// Make sure this element fits within the environment bounds
	int id = atoi(c_id); 
	if (!parent->IsValidElementID(id)) return ErrorCodes::CannotLoadComplexShipElementWithInvalidID;

	// Get a reference to this element within the parent
	ComplexShipElement & el = parent->GetElements()[id];

	// Parse the contents of this node to populate the element details
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key); 
		hash = HashString(key);

		if (hash == HashedStrings::H_ElementLocation) {
			el.SetLocation(IO::GetInt3CoordinatesFromAttr(child));
		}
		/*else if (hash == HashedStrings::H_Properties) {
			el.SetProperties(IO::GetIntValue(child));
		}*/
		else if (hash == HashedStrings::H_Health) {
			el.SetHealth(IO::GetFloatValue(child));
		}
		else if (hash == HashedStrings::H_Strength) {
			el.SetStrength(IO::GetFloatValue(child));
		}
		else if (hash == HashedStrings::H_Connections) {
			el.SetConnectionState(IO::GetIntValue(child));
		}
		else if (hash == HashedStrings::H_AttachPoint) 
		{
			// Make sure the attach point type is valid
			int type = IO::GetIntegerAttribute(child, "type");
			if (type < ComplexShipElement::AttachType::_AttachTypeCount && type >= 0)
			{
				el.SetAttachmentState((ComplexShipElement::AttachType)type, IO::GetIntValue(child));
			}
		}
		else if (hash == HashedStrings::H_NavNodePositionCount) { 
			// Allocate space for nav point data (deal with deallocation if necessary)
			cval = child->GetText();
			int n = atoi(cval);
			el.AllocateNavPointPositionData(n);
		}
		else if (hash == HashedStrings::H_NavNodeConnectionCount) { 
			// Allocate space for nav point connection data (deal with deallocation if necessary)
			cval = child->GetText();
			int n = atoi(cval);
			el.AllocateNavPointConnectionData(n);
		}
		else if (hash == HashedStrings::H_NavNodePosition) {			
			// Get the node index and make sure it is valid
			const char *cindex = child->Attribute("Index"); if (!cindex) continue;
			int index = atoi(cindex); if (index < 0 || index >= el.GetNavPointPositionCount()) continue;

			// Pull other required parameters
			el.GetNavPointPositionData()[index].Position = IO::GetInt3CoordinatesFromAttr(child);
			const char *ccostmod = child->Attribute("CostModifier"); if (!ccostmod) continue;
			float costmod = (float)atof(ccostmod); if (costmod > 0.0f) el.GetNavPointPositionData()[index].CostModifier = costmod;

			// Record the number of connections to other nav nodes, or outside of the current element
			const char *cnumconns = child->Attribute("NumConnections"); if (!cnumconns) continue;
			int numconns = atoi(cnumconns); 
			if (numconns > 0) el.GetNavPointPositionData()[index].NumConnections = numconns;
		}
		else if (hash == HashedStrings::H_NavNodeConnection) { 
			// Retrieve the basic properties of this connection
			const char *cid = child->Attribute("ID"); if (!cid) continue;
			const char *csrc = child->Attribute("Source"); if (!csrc) continue;
			const char *ctgt = child->Attribute("Target"); if (!ctgt) continue;
			const char *ccost = child->Attribute("Cost");  // Not mandatory

			// Make sure the source node is valid
			int navcount = el.GetNavPointPositionCount();
			int src = atoi(csrc); 
			if (src < 0 || src >= navcount) continue;

			// Get the target node; first, see if this is a direction out of the element (rather than a specific node)
			bool isdir = false; int tgt;
			Direction dir = DirectionFromString(ctgt);
			if (dir != Direction::_Count)
				isdir = true;
			else
			{
				isdir = false;
				tgt = atoi(ctgt);
				if (tgt < 0 || tgt >= navcount || tgt == src) continue;
			}

			// Make sure the connection ID is valid
			int conn_id = atoi(cid); 
			if (conn_id < 0 || conn_id >= el.GetNavPointConnectionCount()) continue;

			// Attempt to retrieve the connection cost, if it has been specified
			int cost = 0;
			if (ccost) cost = atoi(ccost); 

			// If no cost is specified, or if it is <=0, then derive it instead from the distance between src & tgt
			if (cost <= 0) 
			{
				INTVECTOR3 psrc, ptgt;
				psrc = el.GetNavPointPositionData()[src].Position;
				if (isdir)	ptgt = ComplexShipElement::GetAdjacentElementCentrePosition(dir);
				else		ptgt = el.GetNavPointPositionData()[tgt].Position;
				INTVECTOR3 diff = psrc - ptgt;
				cost = (int)floorf(sqrtf((float)(diff.x * diff.x) + (float)(diff.y * diff.y) + (float)(diff.z * diff.z)));
			}

			// Make sure we now have a valid cost
			if (cost <= 0) continue;

			// We have all the data we need so store the connection
			el.GetNavPointConnectionData()[conn_id].Source = src;
			el.GetNavPointConnectionData()[conn_id].IsDirection = isdir;
			el.GetNavPointConnectionData()[conn_id].Cost = cost;
			if (isdir)
				el.GetNavPointConnectionData()[conn_id].Target = (int)dir;
			else
				el.GetNavPointConnectionData()[conn_id].Target = tgt;
		}
	}
	
	// Return success
	return ErrorCodes::NoError;
}

Result IO::Data::LoadComplexShipTileClass(TiXmlElement *node)
{
	std::string key, val;
	const char *c_attr = NULL;
	HashVal hash;

	// Parameter check
	if (!node) return ErrorCodes::CannotLoadComplexShipTileClassWithNullData;

	// Create a new tile class object to hold this data
	ComplexShipTileClass *cls = new ComplexShipTileClass();

	// Parse the contents of this node to populate the element details
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (hash == HashedStrings::H_Code) {
			// Set the code
			val = child->GetText(); StrLowerC(val);
			cls->SetCode(val);

			// Validate the class type mapped to this code; if it could not be resolved to a valid type then we can quit here
			if (cls->GetClassType() == D::TileClass::Unknown)
			{
				delete cls; cls = NULL;
				return ErrorCodes::CouldNotLoadTileClassWithInvalidClassType;
			}
		}
		else if (hash == HashedStrings::H_Name) {
			cls->SetName(child->GetText());
		}
		else if (hash == HashedStrings::H_PrimaryTile) {
			val = child->GetText(); StrLowerC(val);
			cls->SetPrimaryTile( (val == "true") );
		}
		else if (hash == HashedStrings::H_MinimumSize) {
			cls->SizeRequirements.MinimumSize = IO::GetInt3CoordinatesFromAttr(child);
		}
		else if (hash == HashedStrings::H_MaximumSize) {
			cls->SizeRequirements.MaximumSize = IO::GetInt3CoordinatesFromAttr(child);
		}
		else if (hash == HashedStrings::H_InterchangeableXYSize) {
			val = child->GetText(); StrLowerC(val);
			cls->SizeRequirements.InterchangeableXY = (val == "true");
		}
		else if (hash == HashedStrings::H_ObjectRequirement) {
			// Retrieve attribute data
			const char *aobj = child->Attribute("class");
			const char *amin = child->Attribute("min");
			const char *amax = child->Attribute("max");

			// Make sure we have the required attributes
			if (!aobj || (!amin && !amax)) continue;

			// Make sure we have a valid class
			ComplexShipObjectClass *obj = D::ComplexShipObjectClasses.Get(aobj);
			if (!obj) continue;

			// Create a new object requirement
			ComplexShipTileClass::ObjectRequirement req = ComplexShipTileClass::ObjectRequirement(obj, 0, 0);

			// Update min and max if applicable
			if (amin) req.Minimum = atoi(amin);
			if (amax) req.Maximum = atoi(amax);

			// Add the requirement to this class
			cls->ObjectRequirements.push_back(req);
		}
		else if (hash == HashedStrings::H_InfrastructureRequirement) {
			// Retrieve the class attribute
			const char *atype = child->Attribute("class");
			if (!atype) continue;

			// Make sure we have a valid infrastructure class
			ComplexShipInfrastructure::InfrastructureClass typ = ComplexShipInfrastructure::TranslateStringToType(atype);
			if (typ == ComplexShipInfrastructure::InfrastructureClass::Unknown) continue;

			// Add a new infrastructure requirement
			cls->InfrastructureRequirements.push_back(ComplexShipTileClass::InfrastructureRequirement(typ));
		}
	}

	// Make sure we have loaded all required data
	if (cls->GetCode() == NullString || cls->GetClassType() == D::TileClass::Unknown)
	{
		delete cls; cls = NULL; 
		return ErrorCodes::CouldNotLoadAllRequiredTileClassData;	
	}

	// Make sure we don't have a duplicate class already defined
	if (D::ComplexShipTileClasses.Exists(cls->GetCode()))
	{
		delete cls; cls = NULL; 
		return ErrorCodes::CouldNotLoadTileClassWithDuplicateCode;
	}
	
	// Add the class to the global collection and return success
	D::ComplexShipTileClasses.Store(cls);
	return ErrorCodes::NoError;
}

Result IO::Data::LoadComplexShipTileDefinition(TiXmlElement *node)
{
	std::string key, val;
	HashVal hash;
	ComplexShipTileDefinition *tiledef;
	bool modeldataloaded = false;
	bool havetemplatedata = false;

	// Make sure we have a valid section of document to work on
	if (!node) return ErrorCodes::NullComplexShipTileDefinitionNodeProvided;

	// Set the class of this tile definition from the parent node attributes
	val = node->Attribute("class"); StrLowerC(val);
	if (val == NullString) return ErrorCodes::NullComplexShipTileDefinitionClass;

	// Attempt to resolve the class type
	ComplexShipTileClass *tc = D::ComplexShipTileClasses.Get(val);
	if (!tc) return ErrorCodes::InvalidComplexShipTileDefinitionClass;

	// Create a new tile definition to hold this data, based on the desired subclass type
	tiledef = ComplexShipTileDefinition::Create(tc->GetClassType());
	if (!tiledef)
	{
		return ErrorCodes::CouldNotCreateNewCSTileDefinition;
	}

	// Attempt to set the tile class object
	if (!tiledef->SetClass(tc))
	{
		return ErrorCodes::CouldNotSetComplexShipTileDefinitionClass;
	}

	// Parse the contents of this node to populate the tile definition details
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (hash == HashedStrings::H_Code) {
			val = child->GetText(); StrLowerC(val);
			tiledef->SetCode(val);
		}
		else if (hash == HashedStrings::H_Name) {
			val = child->GetText();
			tiledef->SetName(val);
		}
		else if (hash == HashedStrings::H_Size) {
			tiledef->SetElementSize( IO::GetInt3CoordinatesFromAttr(child) );
		}
		else if (hash == HashedStrings::H_Level) {
			val = child->GetText();
			if (val == NullString)				tiledef->SetTileLevel(1);
			else								tiledef->SetTileLevel(atoi(val.c_str()));
			if (tiledef->GetTileLevel() < 1)	tiledef->SetTileLevel(1);
		}
		else if (hash == HashedStrings::H_Mass) {
			tiledef->SetMass(IO::GetFloatValue(child, 1.0f));
		}
		else if (hash == HashedStrings::H_Hardness) {
			tiledef->SetHardness(IO::GetFloatValue(child, 1.0f));
		}
		else if (hash == HashedStrings::H_Hardpoint) {
			Hardpoint *h = LoadHardpoint(child);
			if (h) tiledef->AddHardpoint(h);
		}
		else if (hash == HashedStrings::H_Portal) {
			tiledef->AddPortal(std::move(LoadViewPortal(child)));
		}
		else if (hash == HashedStrings::H_Model && !modeldataloaded) {
			// Link this tile to its standard model, which should have been loaded prior to loading dependent objects
			val = child->GetText(); StrLowerC(val);
			tiledef->SetModel( Model::GetModel(val) );

			// This will be a single-model tile
			tiledef->SetHasCompoundModel(false);

			// Load no further model data
			modeldataloaded = true;
		}
		else if (hash == HashedStrings::H_CompoundTileModelData && !modeldataloaded) {
			// Load the compound model data associated with this tile
			LoadComplexShipTileCompoundModel(child, tiledef);

			// This will be a compound-model tile
			tiledef->SetHasCompoundModel(true);
			
			// Load no further model data
			modeldataloaded = true;
		}
		else if (hash == HashedStrings::H_Terrain) 
		{
			// An entry for a terrain object which is tied to this tile.  Attempt to load and add to the tile
			Terrain *t = LoadTerrain(child);
			if (t) tiledef->AddTerrainObject(t);

		}
		else if (hash == HashedStrings::H_DynamicTerrain) 
		{
			// An entry for a dynamic terrain object which is tied to this tile.  Attempt to load and add to the tile
			DynamicTerrain *t = LoadDynamicTerrain(child);
			if (t) tiledef->AddTerrainObject(t);
		}
		else if (hash == HashedStrings::H_ElementStateDefinition) {
			// NOTE: Definition should be set AFTER the tile element size has been defined, since it is passed
			// into the method below.  Or if not, area size can be specified within the state definition, but 
			// this would then require definition in two places
			LoadElementStateDefinition(child, tiledef->GetElementSize(), tiledef->DefaultElementState);
		}
		else if (hash == HashedStrings::H_CanConnect) {
			LoadAndApplyTileConnectionState(child, &(tiledef->Connectivity));
		}
		else if (hash == HashedStrings::H_PowerRequirement) {
			tiledef->SetPowerRequirement(IO::GetIntValue(child));
		}
		else if (hash == HashedStrings::H_DynamicTileSet) {
			val = child->GetText();
			tiledef->AddToDynamicTileSet(val);
		}
		else if (hash == HashedStrings::H_ProductionCost) {
			// We supply one special attribute for tile production; whether these are per-element or overall requirements.  Store it now
			bool perelement = true;										// We assume production cost is per-element unless specified
			const char *ctype = child->Attribute("type");
			if (ctype) { std::string stype = ctype; StrLowerC(stype); perelement = !(stype == "total"); }
			
			// Load the per-element production cost data from file
			ProductionCost *pcost = IO::Data::LoadProductionCostData(child);
			if (!pcost) continue;

			// Check the tile-specific flag; if we were actually loading an overall production cost, make sure that the tile has a fixed
			// size.  If it does, scale the production cost down acordingly to make sure it is stored as per-element data
			if (!perelement)
			{
				// Make sure the tile has a valid, fixed size
				INTVECTOR3 size = tiledef->GetElementSize();
				if (size.x <= 0 || size.y <= 0 || size.z <= 0) continue;

				// Create a clone of the loaded data, but scaled down to represent the per-element cost
				ProductionCost *pcnew = pcost->CreateClone( (1.0f / (size.x * size.y * size.z)) );
				if (!pcnew) { delete pcost; continue; }

				// Deallocate the old production cost and replace it with this one
				delete pcost; 
				pcost = pcnew;
			}

			// Store this new production cost data in the tile definition
			tiledef->SetProductionCost(pcost);
		}
		else if (hash == HashedStrings::H_ClassSpecificDefinition) {
			// Pass control to the class-specific XML method 
			tiledef->ReadClassSpecificXMLData(child);
		}
	}

	// Make sure we were able to load model data, either simple or compound
	if (!modeldataloaded)
	{
		tiledef->Shutdown(); delete tiledef; tiledef = NULL;
		return ErrorCodes::CouldNotLoadModelDataForTileDefinition;
	}

	// Make sure we don't already have a tile definition with this code; if not, add to the central collection
	if (D::ComplexShipTiles.Exists(tiledef->GetCode()))
	{
		tiledef->Shutdown(); delete tiledef; tiledef = NULL;
	}
	else
	{
		D::ComplexShipTiles.Store(tiledef);
	}

	// Return success
	return ErrorCodes::NoError;
}

Result IO::Data::LoadComplexShipTile(TiXmlElement *node, ComplexShipTile **pOutShipTile)
{
	Result compilation_result;
	std::string key, val;
	ComplexShipTile *tile = NULL; 
	
	// Make sure we have a valid section of document to work on
	if (!node || !pOutShipTile) return ErrorCodes::CannotLoadTileWithInvalidParameters;
	
	// The node must also have a valid class attribute, otherwise we cannot load it
	const char *ctcode = node->Attribute("code");
	if (!ctcode) return ErrorCodes::CannotLoadTileWithoutCodeSpecified;

	// Attempt to get the tile definition corresponding to this code
	std::string stcode = ctcode; StrLowerC(stcode);
	ComplexShipTileDefinition *def = D::ComplexShipTiles.Get(stcode);
	if (!def) return ErrorCodes::CannotLoadTileWithInvalidDefinitionCode;

	// Create a new tile from this definition
	tile = def->CreateTile();
	if (!tile) return ErrorCodes::UnknownErrorInCreatingTileFromDefinition;
	
	// Read the XML data in to populate the properties of this tile
	ComplexShipTile::ReadBaseClassXML(node, tile);

	// Now attempt to compile and validate the tile based on the data that was loaded from XML
	compilation_result = def->CompileAndValidateTile(tile);
	if (compilation_result != ErrorCodes::NoError)
	{
		// In case of any error, delete the partially-constructed tile and return the error code
		Game::Log << LOG_WARN << "Tile " << tile->GetID() << " of type \"" << def->GetCode() << "\" failed load-time validation and will not be instantiated (" << compilation_result << ")\n";
		SafeDelete(tile);
		return compilation_result;
	}
	
	// Set a pointer to the new tile and return success to indicate that the tile was created successfully
	(*pOutShipTile) = tile;
	return compilation_result;
}

Result IO::Data::LoadComplexShipTileCompoundModel(TiXmlElement *node, ComplexShipTileDefinition *tiledef)
{
	const char *c_attr;
	std::string key, mtype, mcode;
	float prob = 0.0f;

	// Parameter check
	if (!node || !tiledef) return ErrorCodes::InvalidParametersForLoadingCompoundTileModel;

	// Parse the contents of this node to populate the tile definition details
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);

		if (key == "model") 
		{
			// Attempt to retrieve the tile model type 
			c_attr = child->Attribute("type");
			if (!c_attr) continue;
			mtype = c_attr;

			// Attempt to retrieve the model name for this entry
			c_attr = child->Attribute("code");
			if (!c_attr) continue;
			mcode = c_attr;

			// We have a model name, so also now retrieve the probability
			c_attr = child->Attribute("prob");
			if (!c_attr) continue;
			prob = (float)atof(c_attr);

			// Make sure this is a valid probability
			if (prob < Game::C_EPSILON) continue;

			// Add a new entry to the probability-weighted model collection
			tiledef->AddItemToCompoundModelCollection(mtype, mcode, prob);
		}
	}

	// We have finished loading the compound modelset, so return success
	return ErrorCodes::NoError;
}

// Loads a dynamic tileset and registers it with the central collection
Result IO::Data::LoadDynamicTileSet(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadDynamicTileSetWithInvalidReference;

	// Create a new dynamic tile set to hold the data
	DynamicTileSet *dts = new DynamicTileSet();

	// Parse the contents of this node one element at a time
	std::string key, val;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);

		if (key == "code")
		{
			val = child->GetText();
			dts->SetCode(val);
		}
		else if (key == "defaultoption")
		{
			val = child->GetText(); StrLowerC(val);
			ComplexShipTileDefinition *def = D::ComplexShipTiles.Get(val);
			dts->SetDefault(def);
		}
		else if (key == "option")
		{
			LoadDynamicTileSetOption(child, dts);
		}
	}

	// Only store this dynamic tileset if it contains all mandatory data
	if (dts->GetCode() == NullString)
	{
		return ErrorCodes::CouldNotAddDynamicTileSetWithoutMandatoryData;
	}

	// Add to the central register and return success
	D::DynamicTileSets.Store(dts);
	return ErrorCodes::NoError;
}

// Load one option for a dynamic tileset and apply it to the DTS provided
Result IO::Data::LoadDynamicTileSetOption(TiXmlElement *node, DynamicTileSet *pOutDTS)
{
	// Parameter check
	if (!node || !pOutDTS) return ErrorCodes::CouldNotLoadDynamicTileSetEntryWithNullData;

	// Create an object to hold the data
	DynamicTileSet::DynamicTileRequirements option;

	// Parse the contents of this node one element at a time
	std::string key;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);

		if (key == "tile")
		{
			const char *ccode = child->Attribute("code");
			const char *crot = child->Attribute("rotation");
			std::string code = ccode;
			int rot = atoi(crot);

			option.TileDefinition = D::ComplexShipTiles.Get(code);
			option.Rotation = (Rotation90Degree)rot;

			// Use the tile definition to allocate space for connection requirements.  This adds
			// a hard dependency; the "Tile" element MUST be included before any "ConnectionState"
			// elements, otherwise the connection state data will be ignored 
			if (option.TileDefinition)
			{
				option.Connections.Initialise(option.TileDefinition->GetElementSize());
			}
		}
		else if (key == "connectionstate")
		{
			// Load the connection state data.  Note that, as per the above, this data will only
			// be loaded if it follows the "Tile" element, since that element will also allocate
			// the space required for this connection state data
			LoadAndApplyTileConnectionState(child, &(option.Connections));
		}
	}

	// Add to the dynamic tileset and return success
	pOutDTS->AddEntry(option);
	return ErrorCodes::NoError;
}


// Load the state of a tile connection and set that state in the TileConnections object passed in as a parameter
Result IO::Data::LoadAndApplyTileConnectionState(TiXmlElement *node, TileConnections *pOutConnections)
{
	// Parameter check
	if (!node || !pOutConnections) return ErrorCodes::CannotLoadTileConnectionStateWithNullData;

	// Read data out of the relevant attributes
	INTVECTOR3 loc = INTVECTOR3(); int type, iState; 
	const char *loc_override = NULL;
	node->Attribute(HashedStrings::H_Type.CStr(), &type);
	node->Attribute(HashedStrings::H_X.CStr(), &loc.x);
	node->Attribute(HashedStrings::H_Y.CStr(), &loc.y);
	node->Attribute(HashedStrings::H_Z.CStr(), &loc.z);
	node->Attribute(HashedStrings::H_State.CStr(), &iState);  
	loc_override = node->Attribute(HashedStrings::H_Loc.CStr());

	// Take special action in case of a location override
	if (loc_override)
	{
		std::string setloc = loc_override; StrLowerC(setloc);
		if (setloc == "all")
		{
			// We want to apply this state to ALL elements in the connection set
			if (pOutConnections->ValidateConnectionState((TileConnections::TileConnectionType)type, NULL_INTVECTOR3, (bitstring)iState) == false) 
				return ErrorCodes::TileConnectionStateIsInvalid;;

			pOutConnections->SetConnectionState((TileConnections::TileConnectionType)type, NULL_INTVECTOR3, (bitstring)iState);
			pOutConnections->ReplicateConnectionState(0);
			return ErrorCodes::NoError;
		}
	}

	// No valid override has been specified, so apply the connection state as normal
	if (pOutConnections->ValidateConnectionState((TileConnections::TileConnectionType)type, loc, (bitstring)iState))
	{
		pOutConnections->SetConnectionState((TileConnections::TileConnectionType)type, loc, (bitstring)iState);
		return ErrorCodes::NoError;
	}
	else
	{
		return ErrorCodes::TileConnectionStateIsInvalid;
	}
}

// Loads a static terrain definition and stores it in the global collection
Result IO::Data::LoadTerrainDefinition(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadTerrainDefinitionWithInvalidParams;

	// Create a new terrain definition object to store the data
	TerrainDefinition *def = new TerrainDefinition();

	// Parse the contents of this node to populate the tile definition details
	std::string key, val;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);

		if (key == "code")
		{
			val = child->GetText();
			def->SetCode(val);
		}
		else if (key == "model")
		{
			val = child->GetText();
			def->SetModel(Model::GetModel(val));
		}
		else if (key == "defaultextent")
		{
			def->SetDefaultExtent(IO::GetFloat3FromAttr(child));
		}
		else if (key == "destructible")
		{
			val = child->GetText(); StrLowerC(val);
			def->SetDestructible(val == "true");
		}
		else if (key == "maxhealth")
		{
			const char *cstr = child->GetText();
			def->SetMaxHealth((float)atof(cstr));
		}
	}

	// Make sure we have all mandatory data
	if (def->GetCode() == NullString)
	{
		SafeDelete(def);
		return ErrorCodes::CouldNotLoadTerrainDefWithoutRequiredData;
	}

	// Add to the central collection, assuming no terrain object exists with the same code already
	if (D::TerrainDefinitions.Exists(def->GetCode()))
	{
		SafeDelete(def);
		return ErrorCodes::CouldNotLoadDuplicateTerrainDefinition;
	}
	else
	{
		// Add to the collection and return success
		D::TerrainDefinitions.Store(def);
		return ErrorCodes::NoError;
	}
}

// Load an instance of static terrain
Terrain *IO::Data::LoadTerrain(TiXmlElement *node)
{
	std::string name, val;
	TerrainDefinition *def;
	XMFLOAT3 vpos, vextent;
	XMFLOAT4 qorient;

	// Parameter check
	if (!node) return NULL;

	// Create a new terrain object to hold the data, and default placeholders to hold the data
	Terrain *obj = new Terrain();
	def = NULL;
	vpos = vextent = NULL_FLOAT3; 
	qorient = ID_QUATERNIONF;

	// The terrain details are fully contained within the attributes of this one elemnet
	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = attr->Name(); StrLowerC(name);

		// Take different action depending on the attribute name
		if (name == "code") 
		{
			// Attempt to get the terrain definition with this code
			val = attr->Value(); StrLowerC(val);
			def = D::TerrainDefinitions.Get(val);								// This can be null, for e.g. pure non-renderable collision volumes
		}
		else if (name == "px") vpos.x = (float)atof(attr->Value());
		else if (name == "py") vpos.y = (float)atof(attr->Value());
		else if (name == "pz") vpos.z = (float)atof(attr->Value());
		else if (name == "ox") qorient.x = (float)atof(attr->Value());
		else if (name == "oy") qorient.y = (float)atof(attr->Value());
		else if (name == "oz") qorient.z = (float)atof(attr->Value());
		else if (name == "ow") qorient.w = (float)atof(attr->Value());
		else if (name == "ex") vextent.x = (float)atof(attr->Value());
		else if (name == "ey") vextent.y = (float)atof(attr->Value());
		else if (name == "ez") vextent.z = (float)atof(attr->Value());

		attr = attr->Next();
	}

	// Convert to vectorised form
	XMVECTOR pos = XMLoadFloat3(&vpos);
	XMVECTOR orient = XMLoadFloat4(&qorient);
	XMVECTOR extent = XMLoadFloat3(&vextent);

	// Apply the placeholder data, if it was correctly & fully set from the node attributes
	if (!IsZeroVector3(pos)) obj->SetPosition(pos);
	if (!IsIDQuaternion(orient)) obj->SetOrientation(orient);
	if (!IsZeroVector3(extent)) obj->SetExtent(extent);

	// Assign the terrain definition last, if it was provided, since this will default some values (e.g. extent) if they have
	// not already been set.  Can be NULL, in the case of non-renderable collision volumes
	obj->SetDefinition(def);

	// Return a reference to the new terrain object
	return obj;
}

// Loads a dynamic terrain instance and return it, or null if the object cannot be loaded
DynamicTerrain * IO::Data::LoadDynamicTerrain(TiXmlElement *node)
{
	if (!node) return NULL;

	// Attempt to get any parameters of the top-level node
	const char *c_def = node->Attribute("code");		
	if (!c_def) return NULL; 

	// Attempt to instantiate a new dynamic terrain instance based on this definition code
	DynamicTerrain *terrain = DynamicTerrain::Create(std::string(c_def));
	if (!terrain) return NULL;

	// Process all child nodes of this instance definition
	std::string key, val; HashVal hash;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		// Simply pass to the instance data processing method
		LoadDynamicTerrainInstanceData(child, hash, terrain);
	}
	
	// Return the new dynamic terrain object
	return terrain;

}

// Attempts to load data for a dynamic terrain instance, and returns a flag indicating whether the 
// given data items was applicable & loaded for the object
bool IO::Data::LoadDynamicTerrainInstanceData(TiXmlElement *node, HashVal hash, DynamicTerrain *terrain)
{
	// Compare the hash against all DynamicTerrain-related fields
	if (hash == HashedStrings::H_Position)
	{
		terrain->SetPosition(IO::GetVector3FromAttr(node));
	}
	else if (hash == HashedStrings::H_Orientation)
	{
		terrain->SetOrientation(IO::GetVector4FromAttr(node));
	}
	else if (hash == HashedStrings::H_Extent)
	{
		terrain->SetExtent(IO::GetVector3FromAttr(node));
	}
	else if (hash == HashedStrings::H_State)
	{
		terrain->SetState(node->GetText());
	}
	else if (hash == HashedStrings::H_Property)
	{
		const char *key = node->Attribute("key");
		const char *value = node->Attribute("value");
		if (key && value) terrain->SetProperty(std::string(key), std::string(value));
	}

	// Now pass to each direct superclass if we didn't match any field in this class 
	else if (LoadUsableObjectData(node, hash, static_cast<UsableObject*>(terrain))) return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the direct class fields.  So return true here.
	return true;
}

// Loads a dynamic terrain definition and stores it in the global collection
Result IO::Data::LoadDynamicTerrainDefinition(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadDynamicTerrainDefWithInvalidParams;

	// Attempt to get any parameters of the top-level node
	const char *c_code = node->Attribute("code");		// Mandatory
	const char *c_class = node->Attribute("class");		// Mandatory
	const char *c_def = node->Attribute("def");			// Optional; default NULL
	if (!c_code) return ErrorCodes::CannotLoadDynamicTerrainWithoutUniqueCode;
	if (!c_class) return ErrorCodes::CannotLoadDynamicTerrainWithoutClassName;

	// Code must be unique
	std::string code = std::string(c_code);
	if (D::DynamicTerrainDefinitions.Exists(code)) return ErrorCodes::CannotLoadDuplicateDynamicTerrainDefinition;

	// Resolve the provided terrain definition, if applicable
	const TerrainDefinition *static_def = NULL;
	if (c_def != NULL)
	{
		static_def = D::TerrainDefinitions.Get(std::string(c_def));
		if (static_def == NULL)
		{
			Game::Log << LOG_WARN << "Could not assign terrain-def \"" << c_def << "\" to dynamic terrain definition object \"" << code << "\"; terrain-def is not valid\n";
		}
	}

	// Attempt to instantiate an instance of the given class by its string code; this will become the prototype
	DynamicTerrain *terrain = DynamicTerrainClass::Create(c_class);
	if (!terrain)
	{
		Game::Log << LOG_ERROR << "Cannot instantiate dynamic terrain definition class \"" << c_class << "\"; not a valid class name\n";
		return ErrorCodes::CannotInstantiateDynamicTerrainClass;
	}

	// Create the new definition wrapper object and assign all required data
	DynamicTerrainDefinition *def = new DynamicTerrainDefinition();
	def->SetCode(code);
	def->SetPrototype(terrain);

	// Assign a pointer from the prototype back to its definitions, which will be reflected in all new instances
	terrain->InitialiseNewTerrain(static_def);
	terrain->SetDynamicTerrainDefinition(def);

	// Prototype has been successfully instantiated; process all remaining data in the 
	// node and update the prototype accordingly
	std::string key, val; HashVal hash;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		/* Check each primary proprty of the dynamic terrain definition class */
		if (hash == HashedStrings::H_StateDefinition)
		{
			DynamicTerrainState state;
			Result result = LoadDynamicTerrainStateDefinition(child, state);
			if (result == ErrorCodes::NoError)
			{
				result = def->AssignStateDefinition(state);
				if (result != ErrorCodes::NoError) Game::Log << LOG_WARN << "Failed to assign dynamic terrain state definition \"" << state.GetStateCode() << "\" [" << result << "]\n";
			}
			else {
				Game::Log << LOG_WARN << "Failed to load dynamic terrain state definition \"" << state.GetStateCode() << "\" [" << result << "]\n";
			}
		}
		else if (hash == HashedStrings::H_DefaultState)
		{
			def->SetDefaultState(child->GetText());
		}
		else if (hash == HashedStrings::H_DefaultStateTransition)
		{
			const char *state = child->Attribute("state");
			const char *next_state = child->Attribute("next_state");
			if (state && next_state) def->AddDefaultStateTransition(std::string(state), std::string(next_state));
		}
		else if (hash == HashedStrings::H_PermittedInteractionType)
		{
			def->SetPermittedInteractionType(TranslateDynamicTerrainInteractionTypeFromString(child->GetText()));
		}

		/* Also check whether this is a property of the prototype instance, and assign it here instead if so */
		else if (LoadDynamicTerrainInstanceData(child, hash, terrain)) continue;

	}


	// Add this dynamic terrain definition to the global collection and return success
	D::DynamicTerrainDefinitions.Store(def);
	return ErrorCodes::NoError;
}

// Load properties of a usable object
bool IO::Data::LoadUsableObjectData(TiXmlElement *node, HashVal key, UsableObject *object)
{
	if (!node || !object) return false;

	if (key == HashedStrings::H_DefaultSuccessfulInteractionAudio)	object->SetSuccessfulInteractionAudio(LoadAudioParameters(node));
	else if (key == HashedStrings::H_DefaultFailedInteractionAudio)	object->SetFailedInteractionAudio(LoadAudioParameters(node));

	else
		return false;

	// If we did not hit the 'else' clause we must have matched one of the other conditions, so return success
	return true;
}

Result IO::Data::LoadDynamicTerrainStateDefinition(TiXmlElement *node, DynamicTerrainState & outStateDefinition)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadDynamicTerrainStateDefWithNullData;

	// Attempt to get any parameters of the top-level node
	const char *c_code = node->Attribute("code");
	if (!c_code) return ErrorCodes::CannotLoadDynamicTerrainStateDefWithoutKeyData;

	// Assign top-level data
	outStateDefinition.SetStateCode(std::string(c_code));

	// Now process all remaining data in the node and update the state definition accordingly
	std::string key, val;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);

		/* Check each primary proprty of the dynamic terrain class */
		if (key == "id")
		{
			int id = atoi(child->GetText());
			outStateDefinition.SetStateID(id);
		}
		else if (key == "staticterrain")
		{
			val = child->GetText();
			outStateDefinition.AssignStateStaticTerrain(val);
		}
	}

	// Return success if we have loaded all data
	return ErrorCodes::NoError;
}

// Load a single view portal definition and return it
ViewPortal IO::Data::LoadViewPortal(TiXmlElement *node)
{
	// Portals should be provided as a set of four vertices
	AXMVECTOR vertices[4]; int current_vertex = 0;

	// Process the xml data
	std::string key;
	for (TiXmlElement *child = node->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		HashVal hash = HashString(key);
		
		if (hash == HashedStrings::H_Vertex && current_vertex < 4)	vertices[current_vertex++] = IO::GetVector3FromAttr(child);
	}

	// Any unspecified vertices will be defaulted to null
	for (int i = current_vertex; i < 4; ++i) vertices[i] = NULL_VECTOR;

	// Create the portal from all supplied vertices
	return ViewPortal(vertices);
}

// Load an element state definition from external XML.  Accepts an element_size parameter for initialisation of the
// definition object before loading data.  However size can be overridden at load-time using an "ElementSize" node
Result IO::Data::LoadElementStateDefinition(TiXmlElement *node, const INTVECTOR3 & element_size, ElementStateDefinition & outStateDefinition)
{
	if (!node) return ErrorCodes::CannotLoadElementStateDefinitionFromNullData;

	// If we have passed an element size in via parameters (all components are >0), initialise 
	// the state object before loading data
	if (element_size > NULL_INTVECTOR3) outStateDefinition.Initialise(element_size);

	// Process the xml data
	std::string key, val; HashVal hash;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (hash == HashedStrings::H_ElementSize)
		{
			// Initialises the area based on the given size, overwriting any existing data.  This is optional, in case size is 
			// not already passed in via parameter to this method
			INTVECTOR3 size = IO::GetInt3CoordinatesFromAttr(child);
			outStateDefinition.Initialise(size);
		}
		else if (hash == HashedStrings::H_DefaultState)
		{
			// NOTE: while element state is only the element properties bitstring, we load as an attribute "state" from the node
			// This may need to be extended in future if ElementState begin containing additional data
			val = std::string(child->Attribute("state"));
			bitstring state = ComplexShipElement::ParsePropertyString(val);

			outStateDefinition.ApplyDefaultElementState(ElementStateDefinition::ElementState(state));
		
		}
		else if (hash == HashedStrings::H_State)
		{
			INTVECTOR3 loc = IO::GetInt3CoordinatesFromAttr(child);
			val = std::string(child->Attribute("state"));
			bitstring state = ComplexShipElement::ParsePropertyString(val);
			
			outStateDefinition.SetElementState(ElementStateDefinition::ElementState(state), loc);
		}
		else if (hash == HashedStrings::H_StateFilter)
		{
			val = std::string(child->Attribute("state"));
			bitstring state_filter = ComplexShipElement::ParsePropertyString(val);

			outStateDefinition.ChangeStateFilter(state_filter);
		}
	}

	return ErrorCodes::NoError;
}

// Load an element state definition from external XML.  No target size is specified, so this must either be
// pre-initialised or loaded as part of the xml definition
Result IO::Data::LoadElementStateDefinition(TiXmlElement *node, ElementStateDefinition & outStateDefinition)
{
	return LoadElementStateDefinition(node, NULL_INTVECTOR3, outStateDefinition);
}

// Load turret object data from external XML
Result IO::Data::LoadTurret(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadTurretDataWithInvalidParams;

	// Create a new turret object to store the data
	SpaceTurret *turret = new SpaceTurret();

	// Parse the contents of this node to populate the tile definition details
	std::string key, val; HashVal hash;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (hash == HashedStrings::H_Code)
		{
			val = child->GetText(); StrLowerC(val);
			turret->SetCode(val);
		}
		else if (hash == HashedStrings::H_Name)
		{
			val = child->GetText();
			turret->SetName(val);
		}
		else if (hash == HashedStrings::H_ArticulatedModel)
		{
			val = child->GetText(); StrLowerC(val);
			ArticulatedModel *model = ArticulatedModel::GetModel(val);
			if (model) turret->SetArticulatedModel(model->Copy());
		}
		else if (hash == HashedStrings::H_Yaw)
		{
			// Attempt to pull yaw limit attributes
			const char *cmin = child->Attribute("min");
			const char *cmax = child->Attribute("max");
			if (cmin && cmax)
			{
				// We want to limit yaw on this turret (by supplying both required parameters)
				turret->SetYawLimitFlag(true);

				// Apply the specific values, or keep defaults if not valid
				float ymin = (float)atof(cmin); float ymax = (float)atof(cmax);
				if (ymin < ymax)
				{
					turret->SetYawLimits(ymin, ymax);
				}
			}
			else
			{
				// We do not want to limit yaw on this turret
				turret->SetYawLimitFlag(false);
			}

			// We need a yaw rate attribute, or stick with default if not found
			const char *crate = child->Attribute("rate");
			if (crate)
			{
				float yrate = (float)atof(crate);
				yrate = min(max(Game::C_EPSILON, yrate), 100.0f);
				turret->SetYawRate(yrate);
			}
		}
		else if (hash == HashedStrings::H_Pitch)
		{
			// Attempt to pull pitch limit attributes; if not specified or not valid we keep defaults (pitch limit is mandatory)
			const char *cmin = child->Attribute("min");
			const char *cmax = child->Attribute("max");
			if (cmin && cmax)
			{
				// Apply the specific values, or keep defaults if not valid
				float pmin = (float)atof(cmin); float pmax = (float)atof(cmax);
				if (pmin < pmax)
				{
					turret->SetPitchLimits(pmin, pmax);
				}
			}

			// We need a pitch rate attribute, or stick with default if not found
			const char *crate = child->Attribute("rate");
			if (crate)
			{
				float prate = (float)atof(crate);
				prate = min(max(Game::C_EPSILON, prate), 100.0f);
				turret->SetPitchRate(prate);
			}
		}
		else if (hash == HashedStrings::H_TurretFireDelay)
		{
			const char *cms = child->Attribute("ms"); if (!cms) continue;
			int ms = atoi(cms); ms = max(ms, 1);
			turret->SetTurretFireDelay((unsigned int)ms);
		}
		else if (hash == HashedStrings::H_CreateLaunchers)
		{
			Result result = LoadTurretLaunchers(child, turret);		// Not currently testing return code for failure
		}
	}

	// Make sure we have any mandatory parameters
	if (turret->GetCode() == NullString)
	{
		SafeDelete(turret);
		return ErrorCodes::CannotLoadTurretObjectWithoutAllRequiredData;
	}

	// Recalculate all turret statistics to make it ready for use
	turret->RecalculateTurretStatistics();

	// Add to the central collection of standard turret objects
	D::Turrets.Store(turret);
	return ErrorCodes::NoError;
}

// Load the launcher block for a space turret
Result IO::Data::LoadTurretLaunchers(TiXmlElement *node, SpaceTurret *turret)
{
	// Parameter check
	if (!node || !turret) return ErrorCodes::CannotLoadTurretLauncherBlock;

	// Launcher count should be specified as an attribute at the top level
	const char *ccount = node->Attribute("count"); if (!ccount) return ErrorCodes::CannotLoadTurretLauncherBlock;
	int icount = atoi(ccount); if (icount <= 0) return ErrorCodes::CannotLoadTurretLauncherBlock;
	icount = min(icount, Game::C_MAX_TURRET_LAUNCHERS);

	// Initialise the turret with the specified number of launchers
	turret->InitialiseLaunchers(icount);

	// Process each launcher definition inside the block
	Result result, overallresult = ErrorCodes::NoError;
	std::string bkey, key, val; HashVal hash;
	TiXmlElement *lblock = node->FirstChildElement();
	for (lblock; lblock; lblock = lblock->NextSiblingElement())
	{
		bkey = lblock->Value(); StrLowerC(bkey);
		if (bkey == "projectilelauncher")
		{
			// Index should be specified as an attribute on the block
			int index = 0;
			const char *cindex = lblock->Attribute("index"); if (!cindex) continue;
			index = atoi(cindex); if (index < 0 || index >= icount) continue;

			// Block references a valid launcher index; pull data for it
			TiXmlElement *child = lblock->FirstChildElement();
			for (child; child; child = child->NextSiblingElement())
			{
				key = child->Value(); StrLowerC(key);
				hash = HashString(key);
				if (hash == HashedStrings::H_Code)
				{
					// Copy all data from the specified launcher object
					val = child->GetText(); StrLowerC(val);
					result = turret->SetLauncher(index, D::ProjectileLaunchers.Get(val));
					if (result != ErrorCodes::NoError) overallresult = result;
				}
				else if (hash == HashedStrings::H_RelativePosition)
				{
					XMVECTOR pos = NULL_VECTOR;
					pos = IO::GetVector3FromAttr(child);
					turret->GetLauncher(index)->SetRelativePosition(pos);
				}
				else if (hash == HashedStrings::H_RelativeOrientation)
				{
					XMVECTOR orient = ID_QUATERNION;
					orient = XMQuaternionNormalize(IO::GetQuaternionFromAttr(child));
					turret->GetLauncher(index)->SetRelativeOrientation(orient);
				}
			}
		}
	}

	// Return overall status after loading the entire block
	return overallresult;
}

// Load projectile launcher data from external XML
Result IO::Data::LoadProjectileLauncher(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadProjectileLauncherWithInvalidParams;

	// Create a new object to store the data
	ProjectileLauncher *launcher = new ProjectileLauncher();

	// Parse the contents of this node to populate the tile definition details
	std::string key, val; HashVal hash;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (hash == HashedStrings::H_Code)
		{
			val = child->GetText(); StrLowerC(val);
			launcher->SetCode(val);
		}
		else if (hash == HashedStrings::H_Name)
		{
			val = child->GetText();
			launcher->SetName(val);
		}
		else if (hash == HashedStrings::H_Projectile)
		{
			val = child->GetText(); StrLowerC(val);

			// Determine whether this is a basic or space projectile and update the launcher accordingly
			SpaceProjectileDefinition *space_proj = D::SpaceProjectiles.Get(val);
			if (space_proj)
			{
				launcher->SetProjectileDefinition(space_proj);						// Takes precedence if we have both projectile types with the same identifier
			}
			else
			{
				launcher->SetProjectileDefinition(D::BasicProjectiles.Get(val));	// Will assign null if not found
			}
		}
		else if (hash == HashedStrings::H_LaunchInterval)
		{
			const char *cms = child->Attribute("ms"); if (!cms) continue;
			int ms = atoi(cms); ms = clamp(ms, 1, 1000000);
			launcher->SetLaunchInterval((unsigned int)ms);
		}
		else if (hash == HashedStrings::H_LaunchIntervalVariance)
		{
			const char *cpc = child->Attribute("pc"); if (!cpc) continue;
			float pc = (float)atof(cpc);
			launcher->SetLaunchIntervalVariance(pc);
		}
		else if (hash == HashedStrings::H_Launch)
		{
			const char *ctype = child->Attribute("type");
			const char *cimp = child->Attribute("impulse");
			if (!ctype || !cimp) continue;

			val = ctype;
			float fimp = (float)atof(cimp); fimp = max(1.0f, fimp);
			launcher->SetLaunchMethod(ProjectileLauncher::TranslateLaunchMethodFromString(val));
			launcher->SetLaunchImpulse(fimp);
		}
		else if (hash == HashedStrings::H_Spread)
		{
			const char *cspread = child->Attribute("radians"); if (!cspread) continue;
			float fspread = (float)atof(cspread); fspread = clamp(fspread, 0.0f, PIOVER2 * 0.5f);	// Clamp between 0 and PI/4 rads (0 to 90 deg)
			launcher->SetProjectileSpread(fspread);
		}
		else if (hash == HashedStrings::H_LaunchAngularVelocity)
		{
			launcher->SetLaunchAngularVelocity(IO::GetVector3FromAttr(child));
		}
		else if (hash == HashedStrings::H_DegradeLinearVelocity)
		{
			const char *crate = child->Attribute("rate");
			if (!crate) continue;
			float frate = (float)atof(crate); frate = max(frate, 0.0f);
			
			launcher->SetLinearVelocityDegradeState(true);
			launcher->SetLinearVelocityDegradeRate(frate);
		}
		else if (hash == HashedStrings::H_DegradeAngularVelocity)
		{
			const char *crate = child->Attribute("rate");
			if (!crate) continue;
			float frate = (float)atof(crate); frate = max(frate, 0.0f);

			launcher->SetAngularVelocityDegradeState(true);
			launcher->SetAngularVelocityDegradeRate(frate);
		}
		else if (hash == HashedStrings::H_AddOrientationDrift)
		{
			launcher->SetProjectileOrientationChange(IO::GetQuaternionFromAttr(child));
		}
	}

	// Make sure we have all required fields
	if (launcher->GetCode() == NullString)
	{
		SafeDelete(launcher);
		return ErrorCodes::CannotLoadProjectileLauncherWithoutRequiredData;
	}

	// Add to the central collection and return success
	D::ProjectileLaunchers.Store(launcher);
	return ErrorCodes::NoError;
}

// Load basic projectile definition data from external XML
Result IO::Data::LoadBasicProjectileDefinition(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadBasicProjectileDefWithInvalidParams;

	// Create a new basic projectile object to store the data
	BasicProjectileDefinition *proj = new BasicProjectileDefinition();

	// Parse the contents of this node to populate the projectile definition details
	std::string key, val; HashVal hash;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (hash == HashedStrings::H_Code)
		{
			val = child->GetText(); StrLowerC(val);
			proj->SetCode(val);
		}
		else if (hash == HashedStrings::H_Name)
		{
			val = child->GetText();
			proj->SetName(val);
		}
		else if (hash == HashedStrings::H_Speed)
		{
			const char *cval = child->GetText();
			float fval = (float)atof(cval); fval = clamp(fval, 1.0f, 100000.0f);
			proj->SetProjectileSpeed(fval);
		}
		else if (hash == HashedStrings::H_ProjectileBeamLength)
		{
			const char *cval = child->GetText();
			float fval = (float)atof(cval); fval = clamp(fval, 1.0f, 10000.0f);
			proj->SetProjectileBeamLength(fval);
		}
		else if (hash == HashedStrings::H_ProjectileBeamRadius)
		{
			const char *cval = child->GetText();
			float fval = (float)atof(cval); fval = clamp(fval, 0.0f, 1000.0f);
			proj->SetProjectileBeamRadius(fval);
		}
		else if (hash == HashedStrings::H_Lifetime)
		{
			const char *cval = child->GetText();
			int ival = (int)atoi(cval); ival = clamp(ival, 0, 10000);
			proj->SetProjectileLifetime((unsigned int)ival);
		}
		else if (hash == HashedStrings::H_Colour)
		{
			XMFLOAT4 vec = IO::GetColourFloatFromAttr(child);
			proj->SetProjectileColour(vec);
		}
		else if (hash == HashedStrings::H_Texture)
		{
			val = child->GetText();
			if (val != NullString)
			{
				// Note: not testing return code of this method at the moment
				proj->SetTexture(BuildStrFilename(D::IMAGE_DATA_S, val));
			}
		}
		else if (hash == HashedStrings::H_DamageSet)
		{
			LoadDamageSet(child, proj->ProjectileDamageSet());
		}
		else if (hash == HashedStrings::H_LaunchAudio)
		{
			proj->SetLaunchAudio(LoadAudioParameters(child));
		}
	}

	// Make sure we have all required fields
	if (proj->GetCode() == NullString)
	{
		SafeDelete(proj);
		return ErrorCodes::CannotLoadBasicProjectileDefWithoutRequiredData;
	}

	// Initialise the projectile rendering data based on these loaded properties
	proj->GenerateProjectileRenderingData();

	// Add to the central collection and return success
	D::BasicProjectiles.Store(proj);
	return ErrorCodes::NoError;
}

// Load space projectile definition data from external XML
Result IO::Data::LoadSpaceProjectileDefinition(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadProjectileDefWithInvalidParams;
	 
	// Create a new space projectile object to store the data
	SpaceProjectileDefinition *proj = new SpaceProjectileDefinition();

	// Parse the contents of this node to populate the tile definition details
	std::string key, val; HashVal hash;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (hash == HashedStrings::H_Code)
		{
			val = child->GetText(); StrLowerC(val);
			proj->SetCode(val);
		}
		else if (hash == HashedStrings::H_Name)
		{
			val = child->GetText();
			proj->SetName(val);
		}
		else if (hash == HashedStrings::H_Model)
		{
			val = child->GetText(); StrLowerC(val);
			proj->SetModel(Model::GetModel(val));
		}
		else if (hash == HashedStrings::H_Mass)
		{
			const char *cmass = child->GetText();
			float fmass = (float)atof(cmass); fmass = clamp(fmass, 1.0f, 10000000.0f);
			proj->SetMass(fmass);
		}
		else if (hash == HashedStrings::H_ProjectileType)
		{
			val = child->GetText();
			proj->SetProjectileType(SpaceProjectileDefinition::TranslateProjectileTypeFromString(val));
		}
		else if (hash == HashedStrings::H_DefaultLifetime)
		{
			const char *clife = child->GetText(); if (!clife) continue;
			float flife = (float)atof(clife); flife = max(flife, 0.01f);
			proj->SetDefaultLifetime(flife);
		}
		else if (hash == HashedStrings::H_LifetimeEndAction)
		{
			val = child->GetText();
			proj->SetLifetimeEndAction(SpaceProjectileDefinition::TranslateLifetimeEndActionFromString(val));
		}
		else if (hash == HashedStrings::H_LaunchAudio)
		{
			proj->SetLaunchAudio(LoadAudioParameters(child));
		}
	}

	// Make sure we have all required fields
	if (proj->GetCode() == NullString)
	{
		SafeDelete(proj);
		return ErrorCodes::CannotLoadProjectileDefWithoutRequiredData;
	}

	// Add to the central collection and return success
	D::SpaceProjectiles.Store(proj);
	return ErrorCodes::NoError;
}




ProductionCost *IO::Data::LoadProductionCostData(TiXmlElement *node)
{
	// Parameter check
	if (!node) return NULL;

	// Create a new object to store this production cost data
	ProductionCost *pcost = new ProductionCost();

	// Parse the contents of this node to populate the tile definition details
	const char *cstr = NULL;
	std::string key, val1, val2, val3, val4;
	HashVal hash;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (key == "timerequirement") 
		{
			cstr = child->Attribute("secs"); if (!cstr) continue;
			pcost->SetTimeRequirement( (float)atof(cstr) );
		}
		else if (key == "resourcerequirement")
		{
			val1 = val2 = "";
			cstr = child->Attribute("resource"); if (!cstr) continue; val1 = cstr;
			cstr = child->Attribute("amount"); if (!cstr) continue; val2 = cstr;

			pcost->AddResourceRequirement(val1, (float)atof(val2.c_str()));
		}
		else if (key == "crewclassrequirement")
		{
			val1 = val2 = "";
			cstr = child->Attribute("class"); if (cstr) val1 = cstr;
			cstr = child->Attribute("level"); if (cstr) val2 = cstr; else val2 = "0";

			pcost->AddCrewClassRequirement(val1, atoi(val2.c_str()));
		}
		else if (key == "shiptilerequirement")
		{
			val1 = val2 = val3 = val4 = "";
			cstr = child->Attribute("class"); if (cstr) val1 = cstr;
			cstr = child->Attribute("definition"); if (cstr) val2 = cstr;
			cstr = child->Attribute("level"); if (cstr) val3 = cstr; else val3 = "0";
			cstr = child->Attribute("count"); if (cstr) val4 = cstr; else val4 = "1";

			pcost->AddShipTileRequirement(val1, val2, atoi(val3.c_str()), atoi(val4.c_str()));
		}
		else if (key == "constructedby")
		{
			val1 = "";
			cstr = child->Attribute("class"); if (cstr) val1 = cstr;

			pcost->AddConstructionOwner(val1);
		}
	}

	// Return the new production cost object
	return pcost;
}

Result IO::Data::LoadResource(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadResourceFromNullData;

	// Create a new resource object to hold this data
	Resource *res = new Resource();

	// Process each child node in turn
	std::string key, val;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);
		if (key == "code") {	
			val = child->GetText(); StrLowerC(val);
			res->SetCode(val);
		}
		else if (key == "name") {
			val = child->GetText();
			res->SetName(val);
		}
		else if (key == "value") {
			val = child->GetText();
			if (val != NullString) res->SetValue((float)atof(val.c_str()));
		}
		else if (key == "asteroidresource") { 
			val = child->GetText(); StrLowerC(val);
			res->SetIsAsteroidResource( (val == "true") );
		}
		else if (key == "planetresource") { 
			val = child->GetText(); StrLowerC(val);
			res->SetIsPlanetResource( (val == "true") );
		}
		else if (key == "productioncost") {
			ProductionCost *pcost = LoadProductionCostData(child);
			if (pcost) res->SetProductionCost(pcost);
		}
	}

	// Make sure the resource is valid
	if (res->GetCode() == NullString) return ErrorCodes::CannotLoadResourceWithoutRequiredParameters;

	// Make sure we don't already have a resource definition with this code; if not, add to the central collection
	if (D::Resources.Exists(res->GetCode()))
	{
		delete res; res = NULL;
	}
	else
	{
		D::Resources.Store(res);
	}

	// Return success
	return ErrorCodes::NoError;
}

// Load a new faction and add it to the central collection
Result IO::Data::LoadFaction(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadFactionFromNullData;
	std::string key, val; HashVal hash;

	// Create a new faction object to hold this data
	Faction *f = new Faction();

	// Parse the contents of this node to populate the faction details
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (hash == HashedStrings::H_Code) {
			val = child->GetText(); StrLowerC(val);
			f->SetCode(val);
		}
		else if (hash == HashedStrings::H_Name) {
			val = child->GetText();
			f->SetName(val);
		}
	}

	// Make sure we have loaded all mandatory parameters
	if (f->GetCode() == NullString /* || ... || ... */)
	{
		SafeDelete(f);
		return ErrorCodes::CouldNotLoadFactionWithoutAllRequiredData;
	}

	// Add this faction to the central faction manager collection, & make sure it was successfully added
	if (Game::FactionManager.AddFaction(f) < 0)
	{
		SafeDelete(f);
		return ErrorCodes::CouldNotAddNewLoadedFaction;
	}

	// Return success
	return ErrorCodes::NoError;
}

Hardpoint *IO::Data::LoadHardpoint(TiXmlElement *node)
{
	if (!node) return NULL;

	// Get type and create appropriate item
	const char *c_type = node->Attribute("type");
	if (!c_type) return NULL;
	std::string type = c_type;

	// Compare and determine the correct type of hardpoint
	Equip::Class t = Hp::GetType(type);
	
	// Now create the hardpoint
	Hardpoint *hp = Hp::Create(t);
	if (hp == NULL) return NULL;

	// Get unique code of the hardpoint as appropriate
	const char *c_code = node->Attribute("code");
	if (!c_code) return NULL;
	hp->Code = c_code;

	// * If we have got this far then we know that the required code + type attributes exist, so process child nodes
	std::string key; HashVal hash; Result result;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		result = hp->ReadFromXML(child, hash);
		if (result != ErrorCodes::NoError)
		{
			Game::Log << LOG_WARN << "Error encountered when loading hardpoint \"" << c_code << "\"";
			if (result == ErrorCodes::CouldNotLoadUnrecognisedHardpointProperty) Game::Log << "; ignoring unrecognised property \"" << child->Value() << "\"";
			Game::Log << " (" << result << ")\n";
		}
	}

	// Return the completed hardpoint
	return hp;
}

Result IO::Data::LoadSimpleShipLoadout(TiXmlElement *node)
{
	// Create a new simple ship loadout object to hold the data
	SimpleShipLoadout *L = new SimpleShipLoadout();

	// Now look at each child element in turn and pull data from them
	std::string key; SimpleShip *ship = NULL;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);
		if (key == "code") {					/* The loadout code */
			L->Code = child->GetText();
			StrLowerC(L->Code);
		}
		else if (key == "ship" && !ship) {				// Don't allow a change in ship definition if we have already linked to one
			const char *val = child->GetText();
			if (!val) return ErrorCodes::NoShipSpecifiedForLoadout;
			std::string cval = val; StrLowerC(cval);			// Make lowercase, non-const std::string copy
			
			// Attempt to locate this ship; if it cannot be found then return an error now
			ship = D::SimpleShips.Get(cval);
			if (!ship) return ErrorCodes::InvalidShipSpecifiedForLoadout;

			// Otherwise store a reference to the ship and continue parsing
			L->Ship = cval;
		}
		else if (key == "map" && ship)					// We have to have a ship specified by this point to validate the hardpoint map
		{				
			// Get the hardpoint name and attempt to locate it on the ship
			const char *cc_hp = child->Attribute("hp");
			if (cc_hp) 
			{
				// Validate the ship has a hardpoint with this code, otherwise don't add the map
				std::string c_hp = cc_hp;
				if (c_hp != NullString) 
				{
					Hardpoint *hp = ship->GetHardpoints().Get(c_hp);
					if (hp) 
					{
						// Retrieve the equipment name and validate it
						const char *cc_eq = child->Attribute("equip");
						if (cc_eq)
						{
							std::string c_eq = cc_eq;
							if (c_eq != NullString)
							{
								// Add a map to this loadout;  no (equip!=NULL) check since NULL is valid, to set an empty HP
								Equipment *e = D::Equipment.Get(c_eq);
								L->AddMap(c_hp, e);
							}
						}
					}
				}
			}
		}
		else if (key == "compoundmap" && ship) {		// We have to have a ship specified by this point to validate the hardpoint map
			// Pass control to a function that loads all compound map data
			CompoundLoadoutMap *map = IO::Data::LoadCompoundLoadoutMap(child, L, ship);
			
			// Method will return NULL if the compound map data is invalid; test here and only add it if not NULL
			if (map) L->AddCompoundMap(map->HP, map);
		}
	}

	// Add this loadout to the collection, assuming there isn't already one with the same code
	if (L->Code == NullString || D::SSLoadouts.Exists(L->Code))
		return ErrorCodes::CannotLoadSimpleShipLoadoutWithDuplicateCode;
	else
	{
		// Add the loadout and return success
		D::SSLoadouts.Store(L);
		return ErrorCodes::NoError;
	}
}

CompoundLoadoutMap *IO::Data::LoadCompoundLoadoutMap(TiXmlElement *node, SimpleShipLoadout *L, SimpleShip *targetshiptype)
{
	// Parameter check
	if (!node || !L || !targetshiptype) return NULL;

	// The hardpoint name should be specified as an attribute of this top-level element
	const char *cc_hp = node->Attribute("hp"); if (!cc_hp) return NULL;
	std::string c_hp = cc_hp;

	// Attempt to match the hardpoint to one on this ship; if it does not exist, go no further
	Hardpoint *hp = targetshiptype->GetHardpoints().Get(c_hp);
	if (!hp) return NULL;

	// Create a new compound loadout object to hold the data
	CompoundLoadoutMap *map = new CompoundLoadoutMap();
	map->HP = c_hp;

	// Now look at each child element in turn and pull data from them
	std::string key, val;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);
		if (key == "option") {								/* One option in the probability-weighted compound map */
			const char *cc_prb = child->Attribute("prob");
			const char *cc_eqp = child->Attribute("equip");
			
			// Validate that we have both elements of the probability/equipment combination
			if (cc_prb != NULL && cc_eqp != NULL) 
			{
				// Convert to required data types
				float prob = (float)atof(cc_prb);
				std::string c_eqp = cc_eqp; StrLowerC(c_eqp);

				// Make sure we have equipment specified, and a prob of >~0 (else no point)
				if (c_eqp != NullString && prob > Game::C_EPSILON)
				{
					// Attempt to locate this item of equipment and add an entry to the map
					// No (!= NULL) check since NULL is valid, to specify the chance of an empty HP in the loadout
					Equipment *e = D::Equipment.Get(c_eqp);
					map->AddItem(e, prob);
				}

			}
		}
	}

	return map;
}

Result IO::Data::LoadEngine(TiXmlElement *node)
{
	// Create a new engine object to hold the data
	Engine *e = new Engine();
	
	// Local variables for extracting data
	std::string key;
	const char *c_val = NULL;
	double dval = 0.0f;

	// Now look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);

		if (key == "name") {									/* The engine equipment name */
			e->Name = child->GetText();
		}
		else if (key == "code") {
			e->Code = child->GetText();
		}
		else if (key == "maxhealth") {
			c_val = child->GetText();
			e->SetMaxHealth((Game::HitPoints)atof(c_val));				// Defaults to 0.0f if no conversion possible
		}
		else if (key == "maxthrust") {
			c_val = child->GetText();
			e->SetBaseMaxThrust((float)atof(c_val));					// Defaults to 0.0f if no conversion possible
		}
		else if (key == "minthrust") {
			c_val = child->GetText();
			e->SetBaseMinThrust((float)atof(c_val));					// Defaults to 0.0f if no conversion possible
		}
		else if (key == "acceleration") {
			c_val = child->GetText();
			e->SetBaseAcceleration((float)atof(c_val));					// Defaults to 0.0f if no conversion possible
		}
		else if (key == "emitterclass") {
			e->EmitterClass = child->GetText();
		}
	}

	// Add this engine to the global collection, IF it has at least an internal code to identify it
	if (e->Code != NullString) {
		D::Equipment.Store(e);
		return ErrorCodes::NoError;
	}
	else {
		return ErrorCodes::ObjectHasNoInternalCode;
	}
	
	// If we reached this point we have loaded the item successfully
	return ErrorCodes::NoError;
}

// Load an element in an OBB hierarchy; proceeds recursively until all data is read, or until the maximum depth is reached
void IO::Data::LoadCollisionOBB(iObject *object, TiXmlElement *node, OrientedBoundingBox & obb, bool isroot)
{
	std::string key; HashVal hash;

	// Parameter check
	if (!object || !node) return;

	// Fields to store data as it is being loaded
	XMFLOAT3 posf = NULL_FLOAT3;
	XMFLOAT3 extentf = NULL_FLOAT3;
	XMFLOAT4 orientf = NULL_FLOAT4;
	int numchildren = 0; bool skip = false;

	// Get a reference to the core OBB data.  This will also perform an invalidation
	// check which should do nothing since there is nothing to invalidate yet
	OrientedBoundingBox::CoreOBBData & obb_data = obb.Data();

	// Get required data from the node attributes
	for (TiXmlAttribute *attr = node->FirstAttribute(); attr; attr = attr->Next())
	{
		// Get and hash the attribute key
		key = attr->Name(); StrLowerC(key); 
		hash = HashString(key);

		// Set values based on this hash
		if (hash == HashedStrings::H_Px)					posf.x = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Py)				posf.y = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Pz)				posf.z = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ox)				orientf.x = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Oy)				orientf.y = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Oz)				orientf.z = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ow)				orientf.w = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ex)				extentf.x = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ey)				extentf.y = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ez)				extentf.z = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_NumChildren)		numchildren = atoi(attr->Value());
		else if (hash == HashedStrings::H_Skip)
		{
			std::string val = attr->Value(); StrLowerC(val);
			skip = (val == "true");
		}
	}

	// Convert to vectorised form
	XMVECTOR pos = XMLoadFloat3(&posf);
	XMVECTOR orient = XMLoadFloat4(&orientf);
	XMVECTOR extent = XMLoadFloat3(&extentf);

	// Only update the data for this node if the 'skip' flag was not set.  Usually used to skip the (auto-calculated) root obb
	if (!skip)
	{
		// We do not want to autocalculate this OBB if we have explicit data for it
		obb.SetAutoFitMode(false);

		// Build a transform matrix to determine the obb basis vectors (world = rot * trans)
		XMMATRIX mworld = XMMatrixMultiply(XMMatrixRotationQuaternion(orient), XMMatrixTranslationFromVector(pos));

		// Take different action depending on whether this is the root node
		if (isroot)
		{
			// Set the obb centre, extent and axes based upon this loaded data
			// obb.Data.Axis[0] = D3DXVECTOR3(mworld._11, mworld._12, mworld._13); D3DXVec3Normalize(&obb.Data.Axis[0], &obb.Data.Axis[0]);
			obb_data.Centre = object->GetPosition();
			obb_data.Axis[0].value = XMVector3Normalize(mworld.r[0]);
			obb_data.Axis[1].value = XMVector3Normalize(mworld.r[1]);
			obb_data.Axis[2].value = XMVector3Normalize(mworld.r[2]);
			obb.SetOffsetFlag(false);

			// Update the OBB extents, which will also trigger a full recalculation of the OBB data based on all the above parameters
			obb.UpdateExtent(extent);
		}
		else
		{
			// This is not the root, so instead of setting axes we will set the offset matrix
			obb_data.Centre = object->GetPosition();
			obb.SetOffsetFlag(true);
			obb.Offset = mworld;

			// Update the OBB extents, which will also trigger a full recalculation of the OBB data based on all the above parameters
			obb.UpdateExtent(extent);
		}
	}
	else
	{
		// Otherwise, if we want to skip this node, set it to auto-calculate based on the object size
		obb.SetAutoFitMode(true);
	}

	// Test whether we had any children to be allocated
	if (numchildren > 0)
	{
		obb.AllocateChildren(numchildren);
	}

	// Loop through any child elements of this node, each of which should represent a child obb below this one
	int childindex = 0;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// Make sure we don't exceed the allocated child count
		if (childindex >= numchildren) break;

		// Move recursively into this node as long as it is valid
		key = child->Value(); StrLowerC(key);
		if (key == "collisionobb")
		{
			LoadCollisionOBB(object, child, obb.Children[childindex], false);
			++childindex;
		}
	}

	// If we are the root, we have now set offset matrices all the way down the hierarchy and recursed
	// back up to the root level.  In this case, call the update method to populate all other data for
	// this node and all children below it
	if (isroot) obb.UpdateFromObject(*object);
}

// Load a set of collision spatial data.  Returns a flag indicating whether the data could be loaded
CollisionSpatialDataF IO::Data::LoadCollisionSpatialData(TiXmlElement *node)
{
	CollisionSpatialDataF data;
	if (!node) return data;

	// All details are fully-contained within the attributes of this one elemnet
	std::string name; HashVal hash;
	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) 
	{
		name = attr->Name(); StrLowerC(name);
		hash = HashString(name);

		// Take different action depending on the attribute name
		if (hash == HashedStrings::H_Px.Hash)	   data.Position.x = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Py.Hash) data.Position.y = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Pz.Hash) data.Position.z = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ox.Hash) data.Orientation.x = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Oy.Hash) data.Orientation.y = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Oz.Hash) data.Orientation.z = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ow.Hash) data.Orientation.w = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ex.Hash) data.Extent.x = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ey.Hash) data.Extent.y = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ez.Hash) data.Extent.z = (float)atof(attr->Value());

		attr = attr->Next();
	}

	return data;
}

// Loads the geometry for the specified model
Result IO::Data::LoadModelGeometry(Model *model)
{
	// Exit immediately if the geometry data has already been loaded
	if (model->IsGeometryLoaded()) return ErrorCodes::NoError;

	// Load the model geometry and textures
	Result r = model->Initialise(model->GetFilename().c_str(), model->GetTextureFilename().c_str());

	// Test the return code to see if the model could be successfully loaded
	if (r == ErrorCodes::NoError)	model->SetGeometryLoaded(true); 
	else							model->SetGeometryLoaded(false);

	// If we did succeed in loading the model, scale the model to its target actual size now.  Actual size may already be 
	// set, but this will actually initiate the scaling of model vertices since they were not previously loaded
	if (model->IsGeometryLoaded())	
	{
		// If no effective model size was specified in the xml data, calculate a default now by exactly fitting bounds around the mesh data	
		XMFLOAT3 size = model->GetEffectiveModelSize();
		if (size.x < Game::C_EPSILON || size.y < Game::C_EPSILON || size.z < Game::C_EPSILON)
			model->SetEffectiveModelSize(model->GetModelSize());

		// Set the actual size now.  Method will take care of deriving missing parameters if required, so no validation needed here
		model->SetActualModelSize(model->GetActualModelSize());
	}

	// Return the result of the model loading operation
	return r;
}

Result IO::Data::PostProcessResources(void)
{
	Result result;
	Resource *res;
	ProductionCost *pcost;

	// Create a temporary vector of resource pointers for linear processing reasons
	std::vector<Resource*> resources;
	VectorFromUnorderedMap<std::string, Resource*>(D::Resources.Data, resources);

	// First, we need to build the set of dependencies between resources based on their respective production costs
	std::vector<Resource*>::size_type n = resources.size();
	for (std::vector<Resource*>::size_type i = 0; i < n; ++i)
	{
		// Have the production cost object associated with this resource perform post-processing.  This will resolve
		// all the links out to other resources that are ingredients for this one
		res = resources[i];								if (!res) continue;							// Skip if the resource doesn't exist
		pcost = res->GetProductionCost();				if (!pcost) pcost = new ProductionCost();	// Create a default if doesn't exist (due to some error)
		pcost->PerformPostLoadInitialisation();

		// Also initialise the compound value of each resource to a default (-ve) value.  Values must be > 0 in reality so anything <=0 is treated as uninitialised
		res->SetCompoundValue(-999999.0f);
	}

	// We will now pass through the collection once more.  This time we will follow dependencies (recursively as necessary) to determine
	// the compound value of each resource
	for (std::vector<Resource*>::size_type i = 0; i < n; i++)
	{
		// Get a reference to this resource
		res = resources[i];							
		if (!res) continue;	
	
		// Check the compound value; if it is positive then the value has already been calculated and so we can skip this item
		if (res->GetCompoundValue() > 0.0f) continue;

		// Otherwise, we need to pass control to the recursive method that will calculate the compound value
		result = res->DetermineCompoundValue();
		if (result != ErrorCodes::NoError) return result;
	}

	// Final safety pass through the resource collection to make sure all resources have a compound value > 0.  If not, we return an error
	for (std::vector<Resource*>::size_type i = 0; i < n; i++)
	{
		if (resources[i]->GetCompoundValue() <= 0.0f) return ErrorCodes::ResourceCompoundValuesCouldNotAllBeDetermined;
	}

	// Return success
	return ErrorCodes::NoError;
}
	

Result IO::Data::PostProcessComplexShipTileData(void)
{
	ProductionCost *pcost;
	ComplexShipTileDefinition *tile;
	
	// Create a temporary vector of tile definition pointers for linear processing reasons
	std::vector<ComplexShipTileDefinition*> tiles;
	VectorFromUnorderedMap<std::string, ComplexShipTileDefinition*>(D::ComplexShipTiles.Data, tiles);

	// We need to run post-processing on the tile production requirements, to link to resources/other tile dependencies
	std::vector<ComplexShipTileDefinition*>::size_type n = tiles.size();
	for (std::vector<ComplexShipTileDefinition*>::size_type i = 0; i < n; ++i)
	{
		// Run post-processing on the production cost data
		tile = tiles[i];								if (!tile) continue;						// Skip this if the tile doesn't exist
		pcost = tile->GetProductionCost();				if (!pcost) pcost = new ProductionCost();	// Create a default if doesn't exist (due to some error)
		pcost->PerformPostLoadInitialisation();
	}

	// Return success once all data is initialisated
	return ErrorCodes::NoError;
}

Result IO::Data::LoadSystem(TiXmlElement *node)
{
	// Create a new system object to hold the data
	SpaceSystem *s = new SpaceSystem();
	
	// Local variables for extracting data
	std::string key, val;

	// Now look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);

		if (key == "name") {					/* The system name */
			val = child->GetText();
			s->SetName(val);
		}
		else if (key == "code") {				/* The internal system code */
			val = child->GetText();
			s->SetCode(val);
		}
		else if (key == "size") {				/* Size of the system; system will extend from -X/2 to +X/2 for each dimension of size X */
			XMVECTOR size = XMVectorSetW(															// System size will always be a 
							XMVectorMax(IO::GetVector3FromAttr(child), XMVectorReplicate(1000.0f)), // minimum of 1000 units in each 
							0.0f);																	// dimension, with W cleared to zero
			s->SetSize(size);
		}
		else if (key == "spacebackdrop") {		/* The location of the space backdrop texture */
			s->SetBackdropLocation(child->GetText());
		}
	}
			
	// Make sure the system has all required parameters
	if (s->GetCode() == NullString)
	{
		return ErrorCodes::CannotInitialiseSystemWithoutRequiredParams;
	}

	// Note: we do not need to run s->InitialiseSystem() here; this is handled by the Universe manager later in the load process

	// Finally, add this system to the universe and return success
	Game::Universe->AddSystem(s);
	return ErrorCodes::NoError;
}

Result IO::Data::LoadFireEffect(TiXmlElement *node)
{
	// Create a new effect object to hold the data
	FireEffect *e = new FireEffect();
	
	// Local variables for extracting data
	std::string key, val;

	// Now look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);

		if (key == "code") {												// The uniquely-identifying effect code
			e->SetCode(child->GetText());
		}
		else if (key == "effectmodel") {
			e->SetEffectModel(child->GetText());							// The underlying effect model used for rendering
		}
		else if (key == "firetexture") {
			std::string filename = BuildStrFilename(D::IMAGE_DATA, child->GetText());	// Attempt to load the fire texture used for this effect (no error handling)
			e->SetFireTexture(filename.c_str());				
		}
		else if (key ==	"noisetexture") { 
			std::string filename = BuildStrFilename(D::IMAGE_DATA, child->GetText());	// Attempt to load the noise texture used for this effect (no error handling)
			e->SetNoiseTexture(filename.c_str());				
		}
		else if (key == "alphatexture") {
			std::string filename = BuildStrFilename(D::IMAGE_DATA, child->GetText());	// Attempt to load the alpha texture used for this effect (no error handling)
			e->SetAlphaTexture(filename.c_str());				
		}
		else if (key == "noisescrollspeed") {
			e->SetScrollSpeeds(IO::GetFloat3FromAttr(child));				// 3x scroll speeds for the texture translation
		}
		else if (key == "noisescaling") {									// 3x scaling factors for the texture sampling
			e->SetScaling(IO::GetFloat3FromAttr(child));
		}
		else if (key == "noisedistortion1") {
			e->SetDistortionParameters1(IO::GetFloat2FromAttr(child));	// Noise distortion for sample #1
		}
		else if (key == "noisedistortion2") {
			e->SetDistortionParameters2(IO::GetFloat2FromAttr(child));	// Noise distortion for sample #2
		}
		else if (key == "noisedistortion3") {
			e->SetDistortionParameters3(IO::GetFloat2FromAttr(child));	// Noise distortion for sample #3
		}
		else if (key == "noisedistortionscale") {
			val = child->GetText();
			e->SetDistortionScale((float)atof(val.c_str()));						// Noise distortion scaling factor
		}
		else if (key == "noisedistortionbias") {							// Noise distortion scaling bias
			val = child->GetText();
			e->SetDistortionBias((float)atof(val.c_str()));
		}
	}

	// Add this system to the effect manager collection, IF it has at least an internal code to identify it
	if (e->GetCode() != NullString) {
		Game::Engine->GetEffectManager()->AddFireEffectType(e);
		return ErrorCodes::NoError;
	}
	else {
		return ErrorCodes::ObjectHasNoInternalCode;
	}
}


Result IO::Data::LoadParticleEmitter(TiXmlElement *node)
{
	// Create a new particle emitter prototype to hold the data
	ParticleEmitter *e = new ParticleEmitter();
	
	// Local variables for extracting data
	std::string key, val;

	// Now look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);

		if (key == "code") {												// The uniquely-identifying emitter type code
			e->SetTypeCode(child->GetText());
		}
		else if (key == "particlelimit") {
			e->SetParticleLimit(atoi(child->GetText()));					
		}
		else if (key == "particletexture") {
			e->LoadTexture(Game::Engine->GetDevice(), BuildStrFilename(D::IMAGE_DATA, child->GetText()).c_str());
		}
		else if (key == "emissionfrequency") {
			const char *mn = child->Attribute("min");
			const char *mx = child->Attribute("max");
			if (mn) e->SetParticleEmissionFrequency(ParticleEmitter::Prop::MinValue, (float)atof(mn));
			if (mx) e->SetParticleEmissionFrequency(ParticleEmitter::Prop::MaxValue, (float)atof(mx));
		}
		else if (key == "initiallifetime") {
			const char *mn = child->Attribute("min");
			const char *mx = child->Attribute("max");
			if (mn) e->SetInitialParticleLifetime(ParticleEmitter::Prop::MinValue, (float)atof(mn));
			if (mx) e->SetInitialParticleLifetime(ParticleEmitter::Prop::MaxValue, (float)atof(mx));
		}
		else if (key == "initialposition.min") {
			e->SetInitialParticleLocation(ParticleEmitter::Prop::MinValue, (IO::GetFloat3FromAttr(child)));
		}
		else if (key == "initialposition.max") {
			e->SetInitialParticleLocation(ParticleEmitter::Prop::MaxValue, (IO::GetFloat3FromAttr(child)));
		}
		else if (key == "initialcolour.min") {
			e->SetInitialParticleColour(ParticleEmitter::Prop::MinValue, (IO::GetFloat4FromAttr(child)));
		}
		else if (key == "initialcolour.max") {
			e->SetInitialParticleColour(ParticleEmitter::Prop::MaxValue, (IO::GetFloat4FromAttr(child)));
		}
		else if (key == "initialsize") {
			const char *mn = child->Attribute("min");
			const char *mx = child->Attribute("max");
			if (mn) e->SetInitialParticleSize(ParticleEmitter::Prop::MinValue, (float)atof(mn));
			if (mx) e->SetInitialParticleSize(ParticleEmitter::Prop::MaxValue, (float)atof(mx));
		}
		else if (key == "initialvelocity.min") {
			e->SetInitialParticleVelocity(ParticleEmitter::Prop::MinValue, (IO::GetFloat3FromAttr(child)));
		}
		else if (key == "initialvelocity.max") {
			e->SetInitialParticleVelocity(ParticleEmitter::Prop::MaxValue, (IO::GetFloat3FromAttr(child)));
		}
		else if (key == "updatecolour.min") {
			e->SetParticleColourUpdate(ParticleEmitter::Prop::MinValue, (IO::GetFloat4FromAttr(child)));
		}
		else if (key == "updatecolour.max") {
			e->SetParticleColourUpdate(ParticleEmitter::Prop::MaxValue, (IO::GetFloat4FromAttr(child)));
		}
		else if (key == "updatesize") {
			const char *mn = child->Attribute("min");
			const char *mx = child->Attribute("max");
			if (mn) e->SetParticleSizeUpdate(ParticleEmitter::Prop::MinValue, (float)atof(mn));
			if (mx) e->SetParticleSizeUpdate(ParticleEmitter::Prop::MaxValue, (float)atof(mx));
		}
		else if (key == "updatevelocity.min") {
			e->SetParticleVelocityUpdate(ParticleEmitter::Prop::MinValue, (IO::GetFloat3FromAttr(child)));
		}
		else if (key == "updatevelocity.max") {
			e->SetParticleVelocityUpdate(ParticleEmitter::Prop::MaxValue, (IO::GetFloat3FromAttr(child)));
		}
	}

	// Add this emitter prototype to the particle engine collection, IF it has at least an internal typecode to identify it
	if (e->GetTypeCode() != NullString) {
		Game::Engine->GetParticleEngine()->AddEmitterPrototype(e->GetTypeCode(), e);
		return ErrorCodes::NoError;
	}
	else {
		return ErrorCodes::ObjectHasNoInternalCode;
	}
}


Result IO::Data::LoadUILayout(TiXmlElement *node)
{
	// Local variables for extracting data
	Result result;
	Render2DGroup *group;
	std::string key, val;

	// Render group should start as NULL and be created when the relevant code is provided
	group = NULL;

	// Now look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Extract data from the node
		key = child->Value(); StrLowerC(key);

		// Only pull data if we have created the object
		if (!group && key != "code") continue;

		if (key == "code") {												
			// If this is the layout code then we can create the new layout group and register with the render manager
			group = Game::Engine->Get2DRenderManager()->CreateRenderGroup(child->GetText());
			if (!group) return ErrorCodes::CouldNotCreateRenderGroupFromGameData;
		}
		else if (key == "description") {
			group->SetDescription(child->GetText());
		}
		else if (key == "image2d") 
		{
			// Attempt to retrieve all the required data for an Image2D component
			std::string code = child->Attribute("code");
			std::string texture = child->Attribute("texture");
			XMFLOAT3 pos = IO::GetFloat3FromAttr(child);
			const char *cwidth = child->Attribute("width");
			const char *cheight = child->Attribute("height");
			std::string brender = child->Attribute("render");

			// Make sure we were able to retrieve all required information
			if (code == NullString || texture == NullString || !cwidth || !cheight) continue;

			// Process relevant fields to get the correct format
			StrLowerC(code); 
			std::string texfile = BuildStrFilename(D::IMAGE_DATA, texture);
			int width = (int)floor(atof(cwidth));
			int height = (int)floor(atof(cheight));

			// Initialise a new component using this data
			Image2D *item = D::UI->NewComponent(code, texfile.c_str(), (int)floor(pos.x), (int)floor(pos.y), pos.z, width, height);
			if (!item) continue;
			
			// Only process the render flag if it has been set, otherwise use the Image2D object default
			if (brender != NullString) 
			{
				StrLowerC(brender);
				item->SetRenderActive(brender == "true");
			}

			// Add this new component to the render group
			group->Components.Image2D.AddItem(code, item);
			group->RegisterRenderableComponent(item);
		}
		else if (key == D::NODE_Image2DGroup) {
			result = IO::Data::LoadImage2DGroup(child, group);
		}
		else if (key == "constant")
		{
			// Pull the required fields for a rendering constant
			std::string code = child->Attribute("code");
			std::string value = child->Attribute("value");

			// Add the constant if we have the required info
			if (code != NullString && value != NullString)
			{
				group->Components.RenderConstants.AddItem(code, value);
			}
		}
		else if (key == "textblock")
		{
			// Pull the required fields for a UI text block
			const char *code = child->Attribute("code");
			const char *text = child->Attribute("text");
			const char *cfont = child->Attribute("font");
			const char *x = child->Attribute("x");
			const char *y = child->Attribute("y");
			const char *csize = child->Attribute("size");
			const char *r = child->Attribute("r");
			const char *g = child->Attribute("g");
			const char *b = child->Attribute("b");
			const char *a = child->Attribute("a");
			const char *crender = child->Attribute("render");
			const char *capacity = child->Attribute("capacity");

			// Check that we have those which are mandatory
			if (!code) continue;
			
			// Set default values for anything not specified
			int maxlength;
			std::string srender; bool render;
			XMFLOAT4 col;
			
			// Default position
			int ix, iy;
			if (x) ix = atoi(x); else ix = 0;
			if (y) iy = atoi(y); else iy = 0;

			// Default text
			std::string stext = (text ? text : "");			
			const char *textbuffer = stext.c_str();

			// Default font
			int font;
			if (cfont) font = atoi(cfont); else font = Game::Fonts::FONT_BASIC1;

			// Default size
			float size;
			if (csize) size = (float)atof(csize); else size = 1.0f;

			// Default colour
			if (r && g && b && a) 
				col = XMFLOAT4((FLOAT)atof(r), (FLOAT)atof(g), (FLOAT)atof(b), (FLOAT)atof(a));
			else
				col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

			// Default capacity
			if (capacity) maxlength = atoi(capacity); else maxlength = 256;

			// Default render flag
			if (!crender) render = true;
			else {
				srender = (crender ? crender : "");
				render = (srender == "true") ;
			}
			
			// Now initialise a new sentence object, via the main UI interface
			TextBlock *textblock = D::UI->CreateTextBlock(code, textbuffer, maxlength, font, INTVECTOR2(ix, iy), 
					 									  size, col, render);
			
			// If successful then add this text block to the render group collection
			if (textblock) group->Components.TextBlocks.AddItem(code, textblock);			
		}
		else if (key == "multilinetextblock")
		{
			// Pull the required fields for a UI multi-line text block
			const char *code = child->Attribute("code");
			const char *mode = child->Attribute("mode");
			const char *x = child->Attribute("x");
			const char *y = child->Attribute("y");
			const char *z = child->Attribute("z");
			const char *w = child->Attribute("w");
			const char *h = child->Attribute("h");
			const char *clinecount = child->Attribute("linecount");
			const char *clinelength = child->Attribute("maxlinelength");
			const char *cfont = child->Attribute("font");
			const char *csize = child->Attribute("size");
			const char *r = child->Attribute("r");
			const char *g = child->Attribute("g");
			const char *b = child->Attribute("b");
			const char *a = child->Attribute("a");
			const char *crender = child->Attribute("render");

			// Check that we have those which are mandatory
			if (!code) continue;

			// Default position
			int ix, iy; float fz;
			if (x) ix = atoi(x); else ix = 0;
			if (y) iy = atoi(y); else iy = 0;
			if (z) fz = (float)atof(z); else fz = 0.0f;

			// Default size; each dimension will be independently auto-calculated upon initialisation if not set here
			INTVECTOR2 size = NULL_INTVECTOR2;
			if (w) size.x = atoi(w); else size.x = 0;
			if (h) size.y = atoi(h); else size.y = 0;

			// Default op mode
			MultiLineTextBlock::OperationMode opmode = MultiLineTextBlock::OperationMode::IndividualLines;
			if (mode) opmode = MultiLineTextBlock::TranslateOperationModeFromString(mode);

			// Default font
			int font = Game::Fonts::FONT_BASIC1;
			if (cfont) font = atoi(cfont);

			// Default size
			float fontsize = 1.0f;
			if (csize) fontsize = (float)atof(csize); 

			// Default colour
			XMFLOAT4 col;
			if (r && g && b && a)
				col = XMFLOAT4((FLOAT)atof(r), (FLOAT)atof(g), (FLOAT)atof(b), (FLOAT)atof(a));
			else
				col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

			// Default max line length
			int linelength = 256;
			if (clinelength) linelength = atoi(clinelength);

			// Default line count	
			int linecount = 1;
			if (clinecount) linecount = atoi(clinecount);

			// Default render flag
			bool render = true;
			if (crender) render = !(strcmp(crender, "false") == 0);

			// Now create a new multi-line text block component and initialise it with these values
			MultiLineTextBlock *tb = new MultiLineTextBlock(); if (!tb) continue;
			Result res = tb->Initialise(group, code, opmode, INTVECTOR2(ix, iy), fz, size, linecount, linelength, font, fontsize, col, render);

			// If successful then add this text block to the render group collection
			if (res == ErrorCodes::NoError) group->Components.MultiLineTextBlocks.AddItem(code, tb);
		}
		else if (key == "mouseevent")
		{
			// Attempt to pull required fields for a mouse event
			const char *code = child->Attribute("code");
			const char *sx = child->Attribute("x");
			const char *sy = child->Attribute("y");
			const char *swidth = child->Attribute("width");
			const char *sheight = child->Attribute("height");
	
			// Make sure all mandatory fields exist
			if (!code || !sx || !sy || !swidth || !sheight) continue;

			// Convert mandatory fields as required
			int x = atoi(sx); int y = atoi(sy);
			int width = atoi(swidth); 
			int height = atoi(sheight);

			// Create the new mouse event
			RenderMouseEvent *ev = new RenderMouseEvent(code);
			ev->SetBounds(x, y, width, height);

			// Now look for the optional fields for each event condition
			std::string edefault = child->Attribute("default");
			std::string ehover = child->Attribute("hover");
			std::string edown = child->Attribute("down");

			// Assign event codes based on these parameters
			if (edefault != NullString) ev->SetMouseDefault(edefault, NULL);
			if (ehover != NullString) ev->SetMouseHover(ehover, NULL);
			if (edown != NullString) ev->SetMouseDown(edown, NULL);

			// Add this event to the UI render layout
			group->Components.MouseEvents.AddItem(code, ev);
		}
		else if (key == "button")
		{
			// Pull required fields for a button control
			const char *code = child->Attribute("code");
			const char *upcomp = child->Attribute("up");
			const char *upkey = child->Attribute("up_key");
			const char *downcomp = child->Attribute("down");
			const char *downkey = child->Attribute("down_key");
			const char *textcomp = child->Attribute("text");
			const char *crender = child->Attribute("render");

			// Make sure we have all required fields
			if (!code || !upcomp || !upkey || !downcomp || !downkey) continue;

			// Convert data to the format we require
			bool render = true;
			if (crender && strcmp(crender, "false") == 0) render = false;

			// Attempt to get a reference to each component group
			Image2DRenderGroup *upgroup = group->Components.Image2DGroup.GetItem(upcomp);
			Image2DRenderGroup *downgroup = group->Components.Image2DGroup.GetItem(downcomp);
			if (!upgroup || !downgroup) continue;

			// Now make sure the key is valid, and if so get references to the specific instances
			Image2DRenderGroup::InstanceReference upinst = upgroup->GetInstanceReferenceByCode(upkey);
			Image2DRenderGroup::InstanceReference downinst = downgroup->GetInstanceReferenceByCode(downkey);
			if (!upinst.instance || !downinst.instance) continue;

			// Also get a reference to the button text component, if one has been specified
			TextBlock *textinst = NULL;
			if (textcomp) textinst = group->Components.TextBlocks.GetItem(textcomp);

			// Create a new button component and add links to these components.  Note derived properties are taken from the up componnet
			UIButton *button = new UIButton(code, upinst, downinst, textinst, upinst.instance->position, upinst.instance->size, render);
			group->Components.Buttons.AddItem(code, button);

			// Also add links from the individual components back to this button, for more efficient event handling
			upinst.instance->control = button;
			downinst.instance->control = button;
		}
		else if (key == "textbox")
		{
			// Pull required fields for a button control
			const char *code = child->Attribute("code");
			const char *framecomp = child->Attribute("frame");
			const char *framekey = child->Attribute("frame_key");
			const char *framefocuscomp = child->Attribute("frame_focus");
			const char *framefocuskey = child->Attribute("frame_focus_key");
			const char *textcomp = child->Attribute("text");
			const char *crender = child->Attribute("render");

			// Make sure we have all required fields
			if (!code || !framecomp || !framekey || !textcomp) continue;

			// If we don't have a distinct 'focus' appearance then copy the normal appearance
			if (!framefocuscomp) { framefocuscomp = framecomp; framefocuskey = framekey; }

			// Convert data to the format we require
			bool render = true;
			if (crender && strcmp(crender, "false") == 0) render = false;

			// Attempt to get a reference to each component group
			Image2DRenderGroup *frgroup = group->Components.Image2DGroup.GetItem(framecomp);
			Image2DRenderGroup *frfocusgroup = group->Components.Image2DGroup.GetItem(framefocuscomp);
			if (!frgroup || !frfocusgroup) continue;

			// Now make sure the key is valid, and if so get references to the specific instances
			Image2DRenderGroup::InstanceReference frinst = frgroup->GetInstanceReferenceByCode(framekey);
			Image2DRenderGroup::InstanceReference frfocusinst = frfocusgroup->GetInstanceReferenceByCode(framefocuskey);
			if (!frinst.instance || !frfocusinst.instance) continue;

			// Also get a reference to the button text component, if one has been specified
			TextBlock *textinst = NULL;
			if (textcomp) textinst = group->Components.TextBlocks.GetItem(textcomp);

			// Create a new textbox component and add links to these components.  Derived properties are taken from the frame component
			UITextBox *tbox = new UITextBox(code, frinst, frfocusinst, textinst, frinst.instance->position, frinst.instance->size, render);
			group->Components.TextBoxes.AddItem(code, tbox);

			// Also add links from the individual components back to this button, for more efficient event handling
			frinst.instance->control = tbox;
			frfocusinst.instance->control = tbox;
		}
		else if (key == "combobox")
		{
			// Load from a pre-defined control setup, plus also load parameters for positioning etc.
			const char *cdef = child->Attribute("definition");
			const char *code = child->Attribute("code");
			const char *sx = child->Attribute("x");
			const char *sy = child->Attribute("y");
			const char *sz = child->Attribute("z");
			const char *swidth = child->Attribute("width");
			const char *sheight = child->Attribute("height");
			const char *sexpandsize = child->Attribute("expandsize");
			const char *crender = child->Attribute("render");
	
			// Make sure all mandatory fields exist
			if (!cdef || !code || !sx || !sy || !sz || !swidth || !sheight) continue;

			// Convert mandatory fields as required
			int x = atoi(sx); int y = atoi(sy);
			float z = (float)atof(sz);
			int width = atoi(swidth); 
			int height = atoi(sheight);

			// Convert optional fields or default as necessary
			bool render = true; if (crender && strcmp(crender, "false") == 0) render = false;
			int expandsize = UIComboBox::DEFAULT_EXPAND_SIZE; if (sexpandsize) expandsize = atoi(sexpandsize);

			// Make sure this is a valid definition 
			std::string sdef = cdef;
			if (!D::UI->HaveManagedControlDefinition(sdef)) continue;

			// Retrieve the control definition
			UIManagedControlDefinition *def = D::UI->GetManagedControlDefinition(sdef);
			if (!def) continue;

			// Create a new combobox and initialise it based on this definition
			UIComboBox *control = new UIComboBox();
			control->Initialise(def, code, expandsize, x, y, z, width, height, render);

			// Register this new control with the render group
			control->RegisterWithRenderGroup(group);			
		}
		else if (key == "componentgroup")
		{
			// Load this component group
			result = IO::Data::LoadUIComponentGroup(child, group);
		}
	}

	// Post-processing step: attempt to resolve all event components to the UI components themselves
	if (group)
	{
		Render2DGroup::MouseEventCollection::const_iterator it_end = group->Components.MouseEvents.Items()->end();
		for (Render2DGroup::MouseEventCollection::const_iterator it = group->Components.MouseEvents.Items()->begin(); it != it_end; ++it)
			it->second->ResolveAllComponents(group->Components.Image2D.Items());

		// Final post-processing step; disable rendering of the group by default, which also updates all components to a consistent initial state
		group->SetRenderActive(false);
	}

	// Return success
	return ErrorCodes::NoError;
}

// Loads a 2D image group and, if successful, registers with the specified render group
Result IO::Data::LoadImage2DGroup(TiXmlElement *node, Render2DGroup *group)
{
	std::string key;
	Result result;

	// Parameter check 
	if (!node || !group) return ErrorCodes::CannotLoadImage2DGroupWithNullParameters;

	// This top-level node should contain all the info required to instantiate the render group
	const char *code = node->Attribute("code");
	const char *texture = node->Attribute("texture");
	const char *tmode = node->Attribute("texturemode");
	const char *brender = node->Attribute("render");
	const char *smouse = node->Attribute("acceptsmouse");
	const char *czorder = node->Attribute("z");

	// Make sure we were able to retrieve all required information
	if (!code || !texture || !tmode) return ErrorCodes::InsufficientDataToConstructImage2DGroup;

	// Process the parameters and convert as required
	std::string stexfile = BuildStrFilename(D::IMAGE_DATA, texture);
	const char *texfile = stexfile.c_str();
	std::string grouprender = (brender ? brender : ""); StrLowerC(grouprender);
	std::string acceptsmouse = (smouse ? smouse : ""); StrLowerC(acceptsmouse);

	// Create the new 2D image group object and attempt to initialise it
	Image2DRenderGroup *igroup = new Image2DRenderGroup();
	result = igroup->Initialize( Game::Engine->GetDevice(), Game::ScreenWidth, Game::ScreenHeight, 
								 texfile, Texture::TranslateTextureMode(tmode) );
	if (result != ErrorCodes::NoError) return result;

	// Set other properties at the group level
	igroup->SetCode(code);
	igroup->SetRenderActive( (grouprender == "true") );
	igroup->SetAcceptsMouseInput( (acceptsmouse == "true") );
	
	// Attempt to convert the z value for this group
	float z = 0.0f; if (czorder) z = (float)atof(czorder);
	igroup->SetZOrder(z);

	// Now look at each child element in turn (each should correspond to an instance) and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Extract data from the node
		key = child->Value(); StrLowerC(key);

		// If this is a new instance
		if (key == "instance")
		{
			// Pull data on this instance
			INTVECTOR2 pos = IO::GetInt2CoordinatesFromAttr(child);
			const char *cz = child->Attribute("z");
			const char *cwidth = child->Attribute("width");
			const char *cheight = child->Attribute("height");
			const char *instrender = child->Attribute("render");
			const char *rotate = child->Attribute("rotation");
			const char *instcode = child->Attribute("code");
			
			// Make sure we have all required data
			if (!cwidth || !cheight) continue;

			// Convert data if required
			std::string srender = (instrender ? instrender : "");
			std::string srotate = (rotate ? rotate : "");
			INTVECTOR2 size = INTVECTOR2( atoi(cwidth), atoi(cheight) );
			float zorder = (cz ? (float)atof(cz) : 0.0f);
			Rotation90Degree rot = TranslateRotation90Degree(srotate);
			StrLowerC(srender);

			// Create a new instance with this data
			Image2DRenderGroup::Instance *i = igroup->AddInstance(INTVECTOR2(pos.x, pos.y), zorder, size, (srender == "true"), rot );			

			// Add any additional properties
			i->code = (instcode ? instcode : "");
		}
	}

	// Add this image group to the render group now that we have created it successfully
	group->Components.Image2DGroup.AddItem(code, igroup);
	group->RegisterRenderableComponent(igroup);

	// Return success
	return ErrorCodes::NoError;
}

// Loads a component grouping into a UI layout
Result IO::Data::LoadUIComponentGroup(TiXmlElement *node, Render2DGroup *group)
{
	std::string key;
	const char *citemcode, *citemkey;
	std::string itemcode, itemkey;

	// Parameter check 
	if (!node || !group) return ErrorCodes::CannotLoadUIComponentGroupWithNullParameters;

	// Retrieve top-level information on the group itself
	const char *code = node->Attribute("code");
	if (!code) return ErrorCodes::CannotLoadUIComponentGroupWithNullParameters;

	// Create a new component group 
	UIComponentGroup *cg = new UIComponentGroup();
	cg->SetCode(StrLower(code));

	// Now look at each child element in turn (each should correspond to an instance) and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Extract data from the node
		key = child->Value(); StrLowerC(key);

		// If this is a member of the group then pull information now
		if (key == "include")
		{
			// Pull data on this item
			citemcode = child->Attribute("code");
			citemkey = child->Attribute("key");

			// Make sure the required parameters exist
			if (!citemcode) continue; else itemcode = citemcode;
			if (!citemkey) itemkey = ""; else itemkey = citemkey;

			// Attempt to locate the item in the render group
			iUIComponent *component = group->FindUIComponent(itemcode, itemkey);
			if (component) 
			{
				// If we have found a matching component then add it now
				cg->AddItem(component);
			} 
		}
	}

	// Add this component group to the render group and return success
	group->Components.ComponentGroups.AddItem(cg->GetCode(), cg);
	return ErrorCodes::NoError;
}

// Loads a managed control definition, that specifies all the components within a particular type of managed control
Result IO::Data::LoadUIManagedControlDefinition(TiXmlElement *node)
{
	std::string key;
	const char *citemkey, *citemval;
	std::string itemkey, itemval;

	// Parameter check 
	if (!node) return ErrorCodes::CannotLoadManagedUIControlDefWithoutParameters;

	// Create a new definition object
	UIManagedControlDefinition *def = new UIManagedControlDefinition();

	// Now look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Extract data from the node
		key = child->Value(); StrLowerC(key);

		if (key == "code") {
			def->SetCode(child->GetText());
		}
		else if (key == "class") {
			std::string sclass = child->GetText();
			def->SetClass(iUIControl::DeriveType(sclass));
		}
		else if (key == "component")
		{
			// Pull data on this item
			citemkey = child->Attribute("key");
			citemval = child->Attribute("file");

			// Make sure the required parameters exist
			if (!citemkey) continue; else itemkey = citemkey;
			if (!citemval) continue; else itemval = citemval;

			// Build a complete filename from the relative path supplied
			std::string filename = BuildStrFilename(D::DATA, itemval);

			// Add this mapping to the collection
			def->AddComponent(itemkey, filename);
		}
	}

	// Run validation of the definition before accepting it
	if (!def->ValidateDefinition()) {
		def->Shutdown();
		delete def;
		return ErrorCodes::UIControlDefinitionFailedValidationOnLoad;
	}

	// Add this component group to the user interface and return success
	D::UI->AddManagedControlDefinition(def->GetCode(), def);
	return ErrorCodes::NoError;
}

Result IO::Data::LoadSkinnedModel(TiXmlElement *node)
{
	std::string key = "", code = "", fname = "", texdir = "", val = "";

	// Parameter check
	if (!node) return ErrorCodes::CannotLoadSkinnedModelWithNullParameters;

	// Skinned model will be created during the load sequence once all required parameters are loaded
	SkinnedModel *sm = NULL;
	bool modelcreated = false, trycreatingmodel = false;

	// Look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Extract data from the node
		key = child->Value(); StrLowerC(key);

		// These fields are all loaded before the model can be created
		if (!modelcreated)
		{
			if (key == "code") {
				code = child->GetText(); StrLowerC(code);
				trycreatingmodel = true;
			}
			else if (key == "model") {
				fname = child->GetText(); StrLowerC(fname);
				trycreatingmodel = true;
			}
			else if (key == "textures") {
				texdir = child->GetText(); StrLowerC(texdir);
				trycreatingmodel = true;
			}
		}

		// Test whether we now have the main required parameters for creating the model, before which we
		// cannot set any other parameters since the model will not yet exist
		if (trycreatingmodel && !modelcreated) {
			if (code != NullString && fname != NullString && texdir != NullString)
			{
				// Attempt to create the skinned model based on this data
				// TODO: Error handling in case of error
				sm = new SkinnedModel(Game::Engine->GetDevice(), code, fname, texdir);

				// Set the flag so that we know we have now loaded the model
				modelcreated = true; 
			}

			// Reset the flag so that we try creation again once the next key field is loaded (or not at all if we just succeeded)
			trycreatingmodel = false;
		}

		// Now handle the remainder of parameters, which can only be loaded once the model itself is successfully created
		if (modelcreated && sm)
		{
			if (key == "defaultanimation") {
				val = child->GetText(); StrLowerC(val);
				sm->SetDefaultAnimation(val);
			}
			else if (key == "modelsize") {							// Sets the ultimate model size, which the base mesh will be scaled to at render-time
				sm->SetModelSize(IO::GetFloat3FromAttr(child));
			}
			else if (key == "viewoffset") {
				sm->SetViewOffsetPercentage(IO::GetFloat3FromAttr(child));
			}
		}
	}

	// Make sure the model could be created
	if (!sm || !modelcreated) return ErrorCodes::SkinnedModelCreationFailedDuringLoad; 

	// Make sure a model doesn't already exist with this code
	if (D::SkinnedModels.Exists(code)) return ErrorCodes::CouldNotLoadDuplicateSkinnedModel;

	// Store the newly-created model in the central collection
	D::SkinnedModels.Store(sm);

	// Return success
	return ErrorCodes::NoError;
}

Result IO::Data::LoadActorAttributeGenerationData(TiXmlElement *node)
{
	// Pass control directly to the method within the ActorAttributes domain
	return ActorAttributeGeneration::LoadAttributeGenerationData(node);
}

Result IO::Data::LoadActor(TiXmlElement *node)
{
	std::string key, val;

	// Parameter check
	if (!node) return ErrorCodes::CannotLoadActorWithNullParameters;

	// Create a new actor object to hold this data
	ActorBase *a = new ActorBase();

	// Look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Extract data from the node
		key = child->Value(); StrLowerC(key);

		if (key == "code") {
			val = child->GetText(); StrLowerC(val);
			a->SetCode(val);
		}
		else if (key == "name") {
			val = child->GetText(); StrLowerC(val);
			a->SetName(val);
		}
		else if (key == "model") {
			val = child->GetText(); StrLowerC(val);
			
			// Attempt to link to the specified model
			a->SetModel(val);
			if (a->GetModel() == NULL) { delete a; a = NULL; return ErrorCodes::CouldNotLinkBaseActorToSkinnedModel; }
		}
		else if (key == "attribute") 
		{
			// Pull xml attrs for this attribute
			const char *ctype = child->Attribute("type");					// The attribute being loaded
			const char *cmin = child->Attribute("min");						// Minimum possible value for this attribute
			const char *cmax = child->Attribute("max");						// Maximum possible value for this attribute
			
			const char *cderive, *cgen_min, *cgen_max;
			const char *cbase = child->Attribute("value");					// Either base value is directly specified, or 
																			// derivation parameters are provided
			if (cbase == NULL)
			{
				cderive = child->Attribute("derivation");					// Derivation method used to generate the attribute; 
				cgen_min = child->Attribute("generate_min");				// Minimum bound for the generated value
				cgen_max = child->Attribute("generate_max");				// Maximum bound for the generated value
			}
			
			// Check for required parameters.  Use defaults if not specified and if not mandatory
			ActorBaseAttributeData attr;
			if (!ctype)			continue;
			if (cmin)			attr.MinBound = (float)atof(cmin);
			if (cmax)			attr.MaxBound = (float)atof(cmax);
			if (cbase)
			{
				attr.DerivationType = AttributeDerivationType::Fixed;
				attr.FixedBaseValue = (float)atof(cbase);
			}
			else
			{
				if (!cderive || !cgen_min || !cgen_max) continue;
				std::string sderive = cderive; 
				attr.DerivationType = TranslateAttributeDerivationTypeFromString(sderive);
				attr.GenerateMin = (float)atof(cgen_min);
				attr.GenerateMax = (float)atof(cgen_max);
			}
			
			// Now assign these values to the relevant attribute in the collection
			a->Attributes.SetData(ctype, attr);
		}
		else if (key == "mass") {
			a->SetMass((float)atof(child->GetText()));
		}
	}

	// Make sure we have all key parameters
	if (a->GetCode() == NullString || a->GetModel() == NULL) return ErrorCodes::CannotLoadActorBaseWithInsufficientData;

	// Make sure this isn't a duplicate actor
	if (D::Actors.Exists(a->GetCode())) return ErrorCodes::CouldNotLoadDuplicateActorBaseData;

	// Store the new actor base data in the central collection and return success
	D::Actors.Store(a);
	return ErrorCodes::NoError;
}

// Load an articulated model
Result IO::Data::LoadArticulatedModel(TiXmlElement *node)
{
	Result result;

	// Parameter check
	if (!node) return ErrorCodes::CannotLoadArticulatedModelWithNullParameters;

	// Pull the model component count from this node, and use to create the model itself
	const char *ccount = node->Attribute("components");
	if (!ccount) return ErrorCodes::ArticulatedModelHasInvalidComponentCount;
	int count = atoi(ccount);

	// Attempt to create the model; if parameters are incorrect, model will not be initialised
	ArticulatedModel *model = new ArticulatedModel(count);
	if (!model || model->GetComponentCount() != count)
	{
		if (model) SafeDelete(model);
		return ErrorCodes::CouldNotLoadNewArticulatedModel;
	}

	// Track the number of attachments that have been created
	int attachcount = 0;

	// Look at each child node in turn
	std::string key, val; HashVal hash;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// Hash the value for more efficient lookups
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		// Test for each required field
		if (hash == HashedStrings::H_Code)
		{
			val = child->GetText();
			model->SetCode(val);
		}
		else if (hash == HashedStrings::H_Component)
		{
			// Pull data from each required attribute
			const char *cindex = child->Attribute("index");
			const char *cmodel = child->Attribute("model");
			if (!cindex || !cmodel)
			{
				SafeDelete(model);
				return ErrorCodes::InvalidComponentDataInArticulatedModelNode;
			}

			// Validate the parameters
			int index = atoi(cindex);
			std::string smodel = cmodel; StrLowerC(smodel);
			if (index < 0 || index >= count)
			{
				SafeDelete(model);
				return ErrorCodes::ArticulatedModelContainsInvalidComponentDef;
			}

			// Set the component definition
			Model *m = Model::GetModel(smodel);
			model->SetComponentDefinition(index, m);
		}
		else if (hash == HashedStrings::H_Attachment)
		{
			// Make sure we have not already created the maximum number of attachments
			if (attachcount >= count)
			{
				SafeDelete(model);
				return ErrorCodes::CannotLoadAttachmentDataForArticulatedModel;
			}

			// Pull parent and child indices from the node attributes
			const char *cparent = child->Attribute("parent");
			const char *cchild = child->Attribute("child");
			if (!cparent || !child)
			{
				SafeDelete(model);
				return ErrorCodes::CannotLoadAttachmentDataForArticulatedModel;
			}

			// Attempt to form this attachment, and return an error if any validations fail
			int iparent = atoi(cparent); 
			int ichild = atoi(cchild);
			result = model->SetAttachment(attachcount, iparent, ichild);
			if (result != ErrorCodes::NoError)
			{
				SafeDelete(model);
				return result;
			}
			
			// Now load attachment data from the contents of this node
			Attachment<ArticulatedModelComponent*> *attach = model->GetAttachment(attachcount);
			result = attach->LoadAttachmentData(child);
			if (result != ErrorCodes::NoError)
			{
				SafeDelete(model);
				return result;
			}

			// Attachment has been fully loaded; increment counter of loaded attachments
			++attachcount;
		}
		else if (hash == HashedStrings::H_ConstraintTag)
		{
			// Pull data from each required attribute
			const char *cparent = child->Attribute("parent");	if (!cparent) continue;
			const char *cchild = child->Attribute("child");		if (!cchild) continue;
			const char *ctag = child->Attribute("tag");			if (!ctag) continue;
			int parent_index = atoi(cparent); int child_index = atoi(cchild);
			std::string tag = ctag;

			// Store the new tag; model will perform validation before creating the tag
			model->AddConstraintTag(parent_index, child_index, tag);
		}
		else if (hash == HashedStrings::H_ComponentTag)
		{
			// Pull data from each required attribute
			const char *ccomponent = child->Attribute("component");	if (!ccomponent) continue;
			const char *ctag = child->Attribute("tag");				if (!ctag) continue;
			int component = atoi(ccomponent);
			std::string tag = ctag;

			// Store the new tag; model will perform validation before creating the tag
			model->AddComponentTag(component, tag);
		}
	}

	// Ensure that all components have been attached together, otherwise return failure
	if (attachcount != model->GetAttachmentCount())
	{
		SafeDelete(model);
		return ErrorCodes::CannotLoadUnlinkedArticulatedModel;
	}

	// Validate the model, and make sure we have no duplicate item already in the central collection
	if (model->GetCode() == NullString || ArticulatedModel::ModelExists(model->GetCode()))
	{
		SafeDelete(model);
		return ErrorCodes::CannotStoreNewArticulatedModelWithSpecifiedCode;
	}
	
	// Initialise the model now that all its data has been loaded
	model->PerformPostLoadInitialisation();

	// Otherwise the model is valid; add to the central collection and return success
	ArticulatedModel::AddModel(model);
	return ErrorCodes::NoError;
}

// Loads modifier data from the specified node
Result IO::Data::LoadModifier(TiXmlElement *node)
{
	if (!node) return ErrorCodes::CannotLoadModifierWithNullData;

	// Attempt to get all data from node attributes
	const char *cID = node->Attribute("id");
	const char *cDesc = node->Attribute("desc");
	if (cID == NULL || cDesc == NULL) return ErrorCodes::CannotLoadModifierWithInvalidData;
	std::string id = cID; std::string desc = cDesc;

	// See whether we already have a modifier defined with this ID
	ModifierDetails & modifier = Modifiers::Get(id);
	if (modifier.IsNull())
	{
		Modifiers::Add(id, desc);
	}
	else
	{
		modifier.SetName(id);
		modifier.SetDescription(desc);
	}
	
	// Return success
	return ErrorCodes::NoError;
}

// Load a single damage entry
Result IO::Data::LoadDamage(TiXmlElement *node, Damage & outDamage)
{
	if (!node) return ErrorCodes::CannotLoadDamageEntryWithNullInputData;

	// All data should be held within the attributes of this node
	const char *ctype = node->Attribute("type");
	const char *camt = node->Attribute("amount");
	if (!ctype || !camt) return ErrorCodes::CannotLoadDamageEntryWithoutRequiredData;

	// Return a damage entry based on these attributes
	outDamage.Type = Damage::TranslateDamageTypeFromString(ctype);
	outDamage.Amount = (float)atof(camt);
	return ErrorCodes::NoError;
}

// Load a damage set definition
Result IO::Data::LoadDamageSet(TiXmlElement *node, DamageSet & outDamageSet)
{
	if (!node) return ErrorCodes::CannotLoadDamageSetWithNullInputData;

	// Look at each child node in turn
	Result overallresult = ErrorCodes::NoError;
	std::string key; Result res; Damage damage;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);
		if (key == D::NODE_Damage)
		{
			res = LoadDamage(child, damage);
			if (res == ErrorCodes::NoError) outDamageSet.push_back(damage);
			else							overallresult = ErrorCodes::InvalidDataEncounteredInDamageSet;
		}
	}

	return overallresult;
}

// Load a single damage resistance entry
Result IO::Data::LoadDamageResistance(TiXmlElement *node, DamageResistance & outDR)
{
	if (!node) return ErrorCodes::CannotLoadDREntryWithNullInputData;

	// All data should be held within the attributes of this node
	const char *ctype = node->Attribute("type");
	const char *cthreshold= node->Attribute("threshold");
	const char *cmodifier = node->Attribute("modifier");
	if (!ctype || !cmodifier) return ErrorCodes::CannotLoadDREntryWithoutRequiredData;

	// Return a damage resistance entry based on these attributes
	outDR.Type = Damage::TranslateDamageTypeFromString(ctype);
	outDR.Modifier = (float)atof(cmodifier);
	outDR.Threshold = (cthreshold ? (float)atof(cthreshold) : 0.0f);

	return ErrorCodes::NoError;
}

// Load a damage set definition
Result IO::Data::LoadDamageResistanceSet(TiXmlElement *node, DamageResistanceSet & outDRSet)
{
	if (!node) return ErrorCodes::CannotLoadDRSetWithNullInputData;

	// Look at each child node in turn
	Result overallresult = ErrorCodes::NoError;
	std::string key; Result res; DamageResistance dr;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);
		if (key == D::NODE_DamageResistance)
		{
			res = LoadDamageResistance(child, dr);
			if (res == ErrorCodes::NoError) outDRSet.push_back(dr);
			else							overallresult = ErrorCodes::InvalidDataEncounteredInDRSet;
		}
	}

	return overallresult;
}

// Load an audio item definition (not the audio resource itself)
Result IO::Data::LoadAudioItem(TiXmlElement *node)
{
	if (!node) return ErrorCodes::CannotLoadAudioItemWithNullInput;
	
	std::string key;
	const char *name = NULL, *type = NULL, *filename = NULL, *defaultloop = NULL, *defaultvolume = NULL;

	const TiXmlAttribute *attr = node->FirstAttribute();
	while (attr)
	{
		key = attr->Name();
		if (key == "name")					name = attr->Value();
		else if (key == "type")				type = attr->Value();
		else if (key == "file")				filename = attr->Value();
		else if (key == "loop")				defaultloop = attr->Value();
		else if (key == "volume")			defaultvolume = attr->Value();

		attr = attr->Next();
	}

	// Certain parameters are mandatory
	if (!name || !type || !filename) return ErrorCodes::CannotLoadAudioItemWithoutMandatoryData;

	// Process other, optional parameters
	bool loop = (defaultloop != NULL && strcmp(defaultloop, "true") == 0);
	float default_volume = (defaultvolume == NULL ? 1.0f : (float)atof(defaultvolume));

	// Register a new audio entry with the audio manager, though do not load the 
	// resource itself at this point
	return Game::Engine->GetAudioManager()->RegisterSound(name, type, filename, loop, default_volume, false);
}

// Load a set of audio parameters.  Will return AudioParameters::Null if any required data is missing or invalid
AudioParameters IO::Data::LoadAudioParameters(TiXmlElement *node)
{
	if (!node) return AudioParameters::Null;

	const char *audio_name = node->Attribute(HashedStrings::H_Name.CStr());
	const char *audio_volume = node->Attribute(HashedStrings::H_Volume.CStr());
	if (!audio_name) return AudioParameters::Null;
		
	return AudioParameters(audio_name, (audio_volume ? (float)atof(audio_volume) : 0.0f));
}

// Atempts to locate a ship section in the temporary loading buffer, returning NULL if no match exists
ComplexShipSection *IO::Data::FindInTemporaryCSSBuffer(const std::string & code)
{
	std::vector<ComplexShipSection*>::const_iterator it_end = __TemporaryCSSLoadingBuffer.end();
	for (std::vector<ComplexShipSection*>::const_iterator it = __TemporaryCSSLoadingBuffer.begin(); it != it_end; ++it)
	{
		if ((*it)->GetCode() == code) return (*it);
	}

	return NULL;
}


