#include "stdio.h"
#include <iostream>
#include <fstream>
#include <vector>

#include "GameDataExtern.h"
#include "CoreEngine.h"
#include "Utility.h"
#include "HashFunctions.h"
#include "FileInput.h"
#include "Model.h"

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
#include "StaticTerrain.h"
#include "StaticTerrainDefinition.h"

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
#include "EffectManager.h"
#include "FireEffect.h"
#include "ParticleEngine.h"
#include "ParticleEmitter.h"

#include "UserInterface.h"
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

TiXmlDocument *IO::Data::LoadXMLDocument(const string &filename)
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

Result IO::Data::LoadGameDataFile(const string &filename) { return LoadGameDataFile(filename, true); }

Result IO::Data::LoadGameDataFile(const string &file, bool follow_indices)
{
	// Record the time taken to process this file; store the start time before beginning
	unsigned int processtime = (unsigned int)timeGetTime();

	// Build full filename
	if (file == NullString) return ErrorCodes::NullFilenamePointer;
	string &filename = BuildStrFilename(D::DATA, file);

	// Attempt to load the XML data file
	Result res = ErrorCodes::NoError;
	TiXmlDocument *doc = IO::Data::LoadXMLDocument(filename);
	if (doc == NULL) return ErrorCodes::CannotLoadXMLDocument;

	// The first (and only) root note should be a "GameData" node; if not then stop
	TiXmlElement *root = doc->FirstChildElement();
	if (root == NULL) { delete doc; return ErrorCodes::CannotFindXMLRoot; }

	// Make sure the root name is valid
	string rname = root->Value(); StrLowerC(rname);
	if (!(rname == D::NODE_GameData)) { delete doc; return ErrorCodes::InvalidXMLRootNode; }

	// Now iterate through each child element in turn; these elements at level one should denote the type of object
	string name = "";
	TiXmlElement *child = root->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Test the type of this node
		// TODO: Add error handling if a function returns =/= 0
		name = child->Value(); StrLowerC(name);

		if (name == D::NODE_FileIndex) {
			res = IO::Data::LoadXMLFileIndex(child);

			// If we caught and terminated an infinite file loop we need to propogate the error backwards to stop it simply repeating
			if (res == ErrorCodes::ForceTerminatedInfiniteCircularFileIndices) 
				return ErrorCodes::ForceTerminatedInfiniteCircularFileIndices;

		} else if (name == D::NODE_SimpleShip) {
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
		} else if (name == D::NODE_StaticTerrainDefinition) {
			res = IO::Data::LoadStaticTerrainDefinition(child);
		} else {
			// Unknown level one node type
			res = ErrorCodes::UnknownDataNodeType;
		}

		// If we encountered any errors loading this node then log them and continue
		if (res != ErrorCodes::NoError)
		{
			Game::Log << LOG_INIT_START << "* ERROR " << res << " loading \"" << name << "\" from game data file \"" << file << "\"\n";
			res = ErrorCodes::NoError;
		}
	}

	// Calculate the total time taken to process this file and log it
	processtime = ((unsigned int)timeGetTime() - processtime);
	Game::Log << LOG_INIT_START << "Game data file \"" << file << "\" processed [" << processtime << "ms]\n";

	// Dispose of memory no longer required and return success
	if (doc) delete doc;
	return ErrorCodes::NoError;
}

Result IO::Data::LoadXMLFileIndex(TiXmlElement *node) 
{
	// Maintain an invocation counter to catch infinite circular links between data files
	static int _INVOKE_COUNT = 0;
	string name; Result res;

	// If we have hit the invocation limit we are most likely in an infinite circular loop
	if (++_INVOKE_COUNT > IO::Data::_FILE_INDEX_INVOKE_LIMIT) 
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

Result IO::Data::LoadModelData(TiXmlElement *node)
{
	Model *model;
	Model::ModelClass mclass;
	string key, code, type, fname, tex;
	D3DXVECTOR3 acteffsize, effsize;
	INTVECTOR3 elsize;
	HashVal hash;

	// Set defaults before loading the model
	code = type = fname = tex = "";
	acteffsize = effsize = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
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
		else if (hash == HashedStrings::H_EffectiveSize) {					/* Effective size (i.e. not just vertex max-min) for the model */
			effsize = IO::GetVector3FromAttr(child);
		}
		else if (hash == HashedStrings::H_ActualEffectiveSize) {			/* Actual (i.e. in-game) effective size.  Determines scaling if set */
			acteffsize = IO::GetVector3FromAttr(child);
		}
		else if (hash == HashedStrings::H_ElementSize) {					/* Mapping to element dimensions; optional, and used to scale to fit elements by load post-processing */
			elsize = IO::GetInt3CoordinatesFromAttr(child);	
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
	string filename = BuildStrFilename(D::DATA, fname);
	string texture = BuildStrFilename(D::DATA, tex);

	// Otherwise create a new model here
	model = new Model();
	model->SetCode(code);
	model->SetModelClass(mclass);
	model->SetFilename(filename);
	model->SetTextureFilename(texture);
	model->SetEffectiveModelSize(effsize);
	model->SetElementSize(elsize);

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

	// If a desired actual effective size has been set then apply it and scale accordingly, otherwise just use the current size
	//if (acteffsize.x <= Game::C_EPSILON && acteffsize.y <= Game::C_EPSILON && acteffsize.z <= Game::C_EPSILON)
	//	model->SetActualModelSize(acteffsize);
	//else
	//	model->SetActualModelSize(model->GetEffectiveModelSize());

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
	else if (hash == HashedStrings::H_Orientation)					object->SetOrientation(IO::GetD3DXQUATERNIONFromAttr(node));
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

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the fields.  So return true here.
	return true;
}

// Loads iActiveObject class data, returning true if the value was loaded
bool IO::Data::LoadActiveObjectData(TiXmlElement *node, HashVal hash, iActiveObject *object)
{	
	// Compare the hash against all iActiveObject-related fields
	if (hash == HashedStrings::H_Mass)							object->SetMass(GetFloatValue(node));

	/* Now pass to each direct superclass if we didn't match any field in this class */
	else if (LoadObjectData(node, hash, object))				return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else														return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the direct class fields.  So return true here.
	return true;
	
}

// Loads iStaticObject class data, returning true if the value was loaded
bool IO::Data::LoadStaticObjectData(TiXmlElement *node, HashVal hash, iStaticObject *object)
{
	// Compare the hash against all iStaticObject-related fields
	// (No iStaticObject-specific fields at this point)

	/* Now pass to each direct superclass if we didn't match any field in this class */
	if (LoadObjectData(node, hash, object)) return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else														return false;
	
	// If we didn't hit the "else" clause, and return false, we must have matched one of the direct class fields.  So return true here.
	return true;
}

bool IO::Data::LoadSpaceObjectData(TiXmlElement *node, HashVal hash, iSpaceObject *object)
{
	// Compare the hash against all iSpaceObject-related fields
	// (No iSpaceObject-specific fields at this point)

	/* Now pass to each direct superclass if we didn't match any field in this class */
	if (LoadActiveObjectData(node, hash, object))				return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else														return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the direct class fields.  So return true here.
	return true;
}

bool IO::Data::LoadEnvironmentObjectData(TiXmlElement *node, HashVal hash, iEnvironmentObject *object)
{
	// Compare the hash against all iEnvironmentObject-related fields
	// (No iEnvironmentObject-specific fields at this point)

	/* Now pass to each direct superclass if we didn't match any field in this class */
	if (LoadActiveObjectData(node, hash, object))				return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else														return false;

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
		object->SetElementSize(IO::GetVector3FromAttr(node));
		object->InitialiseAllElements();
	}
	else if (hash == HashedStrings::H_ComplexShipElement)			LoadComplexShipElement(node, object);
	else if (hash == HashedStrings::H_StaticTerrain)
	{
		StaticTerrain *t = LoadStaticTerrain(node);
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
	else if (hash == HashedStrings::H_BankExtent)					object->BankExtent = (IO::GetVector3FromAttr(node) * PIBY180);	// Convert degrees > radians
	else if (hash == HashedStrings::H_DefaultLoadout)				object->SetDefaultLoadout(GetLCString(node));
	else if (hash == HashedStrings::H_Mass)							object->SetBaseMass(GetFloatValue(node));	// Overrides the iActiveObject behaviour, since ships have base & overall mass

	/* Now pass to each direct superclass if we didn't match any field in this class */
	if (LoadSpaceObjectData(node, hash, object))					return true;

	// 'Else' case - none of the fields matched this hash, so return false now
	else															return false;

	// If we didn't hit the "else" clause, and return false, we must have matched one of the fields.  So return true here.
	return true;

}

Result IO::Data::LoadSimpleShip(TiXmlElement *root)
{
	// Create a new SimpleShip instance to hold the data
	SimpleShip *object = new SimpleShip();
	object->SetShipClass(Ships::Class::Simple);

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
				if (h) object->GetShipHardpoints()->AddHardpoint(h);
			}
		}
	}

	// Validation; make sure key mandatory fields are supplied, and the code is not already in use, otherwise we will not create the ship
	if (object->GetCode() == NullString || D::SimpleShips.count(object->GetCode()) > 0)
	{
		SafeDelete(object);
		return ErrorCodes::CannotLoadSimpleShipDetailsWithDuplicateCode;
	}

	// Otherwise, calculate the ship data and store in the central collection
	object->RecalculateAllShipData();
	D::AddStandardSimpleShip(object);

	// Return success
	return ErrorCodes::NoError;
}


Result IO::Data::LoadComplexShip(TiXmlElement *root)
{
	// Create a new ComplexShip instance to hold the data
	ComplexShip *object = new ComplexShip();
	object->SetShipClass(Ships::Class::Complex);

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
			// Compare the hash against all SimpleShip-related fields
			if		(hash == HashedStrings::H_SDOffset)						object->SetSDOffset(IO::GetInt3CoordinatesFromAttr(node));
			else if (hash == HashedStrings::H_ComplexShipSectionInstance)	result = LoadComplexShipSectionInstance(node, object);
			else if (hash == HashedStrings::H_ComplexShipTile)
			{
				result = LoadComplexShipTile(node, &tile);
				if (tile) tile->LinkToParent(object);
			}
		}
	}

	// Validation; make sure key mandatory fields are supplied, and the code is not already in use, otherwise we will not create the ship
	if (object->GetCode() == NullString || D::ComplexShips.count(object->GetCode()) > 0)
	{
		SafeDelete(object);
		__TemporaryCSSLoadingBuffer.clear();
		return ErrorCodes::CannotLoadComplexShipDetailsWithDuplicateCode;
	}

	// Otherwise, calculate the ship data and store in the central collection
	object->RecalculateAllShipData();
	D::AddStandardComplexShip(object);

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
			if (hash == HashedStrings::H_ElementSize)						object->SetElementSize(IO::GetInt3CoordinatesFromAttr(node));
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
				if (h) object->GetHardpoints()->AddHardpoint(h);
			}
		}
	}

	// Validation; make sure key mandatory fields are supplied, and the code is not already in use, otherwise we will not create the ship
	if (object->GetCode() == NullString || D::ComplexShipSections.count(object->GetCode()) > 0)
	{
		SafeDelete(object);
		return ErrorCodes::CannotLoadCSSectionDetailsWithDuplicateCode;
	}

	// Otherwise, recalculate the section data 
	object->RecalculateShipDataFromCurrentState();

	// Store in either the central collection or the temporary loading buffer, depending on whether this is a standard section
	if (object->IsStandardObject())
	{
		// If this is a standard section, simply add to the central data collection
		D::AddStandardComplexShipSection(object);
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
	std::string a_code = ""; D3DXVECTOR3 a_pos = NULL_VECTOR; INTVECTOR3 a_elpos = NULL_INTVECTOR3; 
	Rotation90Degree a_rot = Rotation90Degree::Rotate0;
	TiXmlElement *node = root->FirstChildElement();
	for (node; node; node = node->NextSiblingElement())
	{
		// Hash the value for more efficient lookups
		key = node->Value(); StrLowerC(key);
		hash = HashString(key);

		// Test for each required field
		if		(hash == HashedStrings::H_Code)				a_code = node->GetText();
		else if (hash == HashedStrings::H_Position)			a_pos = IO::GetVector3FromAttr(node);
		else if (hash == HashedStrings::H_ElementLocation)	a_elpos = IO::GetInt3CoordinatesFromAttr(node);
		else if (hash == HashedStrings::H_Rotation)			a_rot = TranslateRotation90Degree(node->GetText());
	}

	// Make sure we have mandatory fields
	if (a_code == NullString) return ErrorCodes::CannotLoadInstanceOfNullCSSection;

	// (Check for validity, e.g. make sure no overlap with existing section)

	// Check whether a section exists with this code
	StrLowerC(a_code);
	ComplexShipSection *sec = NULL;
	if (D::ComplexShipSections.count(a_code) > 0)
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
	sec->SetRelativePosition(a_pos);
	sec->SetElementLocation(a_elpos);
	sec->SetRotation(a_rot);

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
	string type = c_type;

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
	string key, val;
	HashVal hash;
	
	// Make sure we have a valid section of document to work on
	if (!node || !parent) return ErrorCodes::NullComplexShipElementNodeProvided;
	
	// Attempt to get the element at the specified location.  This also performs validation of the 
	// indices provided (if they were provided) as well by returning NULL if they are invalid
	INTVECTOR3 location = IO::GetInt3CoordinatesFromAttr(node);
	ComplexShipElement *el = parent->GetElement(location.x, location.y, location.z);
	if (el == NULL) return ErrorCodes::CannotLoadComplexShipElementForInvalidCoords;

	// Parse the contents of this node to populate the element details
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key); 
		hash = HashString(key);

		if (hash == HashedStrings::H_Properties) {
			// Properties are saved one-per-attribute
			const TiXmlAttribute *attr; ComplexShipElement::PROPERTY prop;
			for (attr = child->FirstAttribute(); attr; attr = attr->Next() )
			{
				prop = ComplexShipElement::TranslatePropertyFromName(attr->Name());
				if (ComplexShipElement::IsValidProperty(prop)) 
					el->SetProperty(prop, (strcmp(attr->Value(), "true") == 0));
			}
		}
		else if (hash == HashedStrings::H_AttachPoint) {
			// Retrieve the attachment point data from file
			const char *sedge = child->Attribute("edge");
			const char *stype = child->Attribute("type");
			if (!sedge || !stype) continue;

			// Create a new attachment point by translating this data into attach point values
			el->AddAttachPoint(ComplexShipElement::ElementAttachPoint(DirectionFromString(sedge),
																	  ComplexShipElement::AttachTypeFromString(stype)));
		}
		else if (hash == HashedStrings::H_NavNodePositionCount) { 
			// Allocate space for nav point data (deal with deallocation if necessary)
			cval = child->GetText();
			int n = atoi(cval);
			el->AllocateNavPointPositionData(n);
		}
		else if (hash == HashedStrings::H_NavNodeConnectionCount) { 
			// Allocate space for nav point connection data (deal with deallocation if necessary)
			cval = child->GetText();
			int n = atoi(cval);
			el->AllocateNavPointConnectionData(n);
		}
		else if (hash == HashedStrings::H_NavNodePosition) {			
			// Get the node index and make sure it is valid
			const char *cindex = child->Attribute("Index"); if (!cindex) continue;
			int index = atoi(cindex); if (index < 0 || index >= el->GetNavPointPositionCount()) continue;

			// Pull other required parameters
			el->GetNavPointPositionData()[index].Position = IO::GetInt3CoordinatesFromAttr(child);
			const char *ccostmod = child->Attribute("CostModifier"); if (!ccostmod) continue;
			float costmod = (float)atof(ccostmod); if (costmod > 0.0f) el->GetNavPointPositionData()[index].CostModifier = costmod;

			// Record the number of connections to other nav nodes, or outside of the current element
			const char *cnumconns = child->Attribute("NumConnections"); if (!cnumconns) continue;
			int numconns = atoi(cnumconns); 
			if (numconns > 0) el->GetNavPointPositionData()[index].NumConnections = numconns;
		}
		else if (hash == HashedStrings::H_NavNodeConnection) { 
			// Retrieve the basic properties of this connection
			const char *cid = child->Attribute("ID"); if (!cid) continue;
			const char *csrc = child->Attribute("Source"); if (!csrc) continue;
			const char *ctgt = child->Attribute("Target"); if (!ctgt) continue;
			const char *ccost = child->Attribute("Cost");  // Not mandatory

			// Make sure the source node is valid
			int navcount = el->GetNavPointPositionCount();
			int src = atoi(csrc); 
			if (src < 0 || src >= navcount) continue;

			// Get the target node; first, see if this is a direction out of the element (rather than a specific node)
			bool isdir = false; int tgt;
			Direction dir = DirectionFromString(ctgt);
			if (dir != Direction::None)
				isdir = true;
			else
			{
				isdir = false;
				tgt = atoi(ctgt);
				if (tgt < 0 || tgt >= navcount || tgt == src) continue;
			}

			// Make sure the connection ID is valid
			int id = atoi(cid); 
			if (id < 0 || id >= el->GetNavPointConnectionCount()) continue;

			// Attempt to retrieve the connection cost, if it has been specified
			int cost = 0;
			if (ccost) cost = atoi(ccost); 

			// If no cost is specified, or if it is <=0, then derive it instead from the distance between src & tgt
			if (cost <= 0) 
			{
				INTVECTOR3 psrc, ptgt;
				psrc = el->GetNavPointPositionData()[src].Position;
				if (isdir)	ptgt = ComplexShipElement::GetAdjacentElementCentrePosition(dir);
				else		ptgt = el->GetNavPointPositionData()[tgt].Position;
				INTVECTOR3 diff = psrc - ptgt;
				cost = (int)floorf(sqrtf((float)(diff.x * diff.x) + (float)(diff.y * diff.y) + (float)(diff.z * diff.z)));
			}

			// Make sure we now have a valid cost
			if (cost <= 0) continue;

			// We have all the data we need so store the connection
			el->GetNavPointConnectionData()[id].Source = src;
			el->GetNavPointConnectionData()[id].IsDirection = isdir;
			el->GetNavPointConnectionData()[id].Cost = cost;
			if (isdir)
				el->GetNavPointConnectionData()[id].Target = (int)dir;
			else
				el->GetNavPointConnectionData()[id].Target = tgt;
		}
	}
	
	// Return success
	return ErrorCodes::NoError;
}

Result IO::Data::LoadComplexShipTileClass(TiXmlElement *node)
{
	string key, val;
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
			ComplexShipObjectClass *obj = D::GetComplexShipObjectClass(aobj);
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
	if (D::ComplexShipTileClasses.count(cls->GetCode()) > 0)
	{
		delete cls; cls = NULL; 
		return ErrorCodes::CouldNotLoadTileClassWithDuplicateCode;
	}
	
	// Add the class to the global collection and return success
	D::AddStandardComplexShipTileClass(cls);
	return ErrorCodes::NoError;
}

Result IO::Data::LoadComplexShipTileDefinition(TiXmlElement *node)
{
	string key, val;
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
	ComplexShipTileClass *tc = D::GetComplexShipTileClass(val);
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
			if (val == NullString)						tiledef->SetTileLevel(1);
			else								tiledef->SetTileLevel(atoi(val.c_str()));
			if (tiledef->GetTileLevel() < 1)	tiledef->SetTileLevel(1);
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
		else if (hash == HashedStrings::H_StaticTerrain) {
			// An entry for a terrain object which is tied to this tile.  Attempt to load from the node
			StaticTerrain *t = LoadStaticTerrain(child);
			if (!t) continue;

			// Add to the vector of terrain objects in the tile definition
			tiledef->AddTerrainObject(t);
		}
		else if (hash == HashedStrings::H_DefaultProperty) {
			// Pull the relevant properties
			const char *pname = child->Attribute("name");
			const char *pval = child->Attribute("value");
			if (!pname || !pval) continue;

			// Attempt to locate this property name
			ComplexShipElement::PROPERTY prop = ComplexShipElement::TranslatePropertyFromName(pname);
			if (prop == ComplexShipElement::PROPERTY::PROP_UNKNOWN) continue;

			// Set this property in the tile definition
			string sval = pval; StrLowerC(sval);
			tiledef->DefaultProperties.push_back(ComplexShipElement::PropertyValue(prop, (sval == "true")));
		}
		else if (hash == HashedStrings::H_ProductionCost) {
			// We supply one special attribute for tile production; whether these are per-element or overall requirements.  Store it now
			bool perelement = true;										// We assume production cost is per-element unless specified
			const char *ctype = child->Attribute("type");
			if (ctype) { string stype = ctype; StrLowerC(stype); perelement = !(stype == "total"); }

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
	if (D::ComplexShipTiles.count(tiledef->GetCode()) != 0)
	{
		tiledef->Shutdown(); delete tiledef; tiledef = NULL;
	}
	else
	{
		D::AddStandardComplexShipTileDefinition(tiledef);
	}

	// Return success
	return ErrorCodes::NoError;
}

Result IO::Data::LoadComplexShipTile(TiXmlElement *node, ComplexShipTile **pOutShipTile)
{
	Result result;
	string key, val;
	ComplexShipTile *tile = NULL;

	// Make sure we have a valid section of document to work on
	if (!node || !pOutShipTile) return ErrorCodes::CannotLoadTileWithInvalidParameters;
	
	// The node must also have a valid class attribute, otherwise we cannot load it
	const char *ctcode = node->Attribute("code");
	if (!ctcode) return ErrorCodes::CannotLoadTileWithoutCodeSpecified;

	// Attempt to get the tile definition corresponding to this code
	string stcode = ctcode; StrLowerC(stcode);
	ComplexShipTileDefinition *def = D::GetComplexShipTile(stcode);
	if (!def) return ErrorCodes::CannotLoadTileWithInvalidDefinitionCode;

	// Create a new tile from this definition
	tile = def->CreateTile();
	if (!tile) return ErrorCodes::UnknownErrorInCreatingTileFromDefinition;

	// Read the XML data in to populate the properties of this tile
	ComplexShipTile::ReadBaseClassXML(node, tile);

	// Now attempt to compile and validate the tile based on the data that was loaded from XML
	result = def->CompileAndValidateTile(tile);
	if (result != ErrorCodes::NoError)
	{
		delete tile; tile = NULL;
		return result;
	}
	
	// Set a pointer to the new tile and return success to indicate that the tile was created successfully
	(*pOutShipTile) = tile;
	return ErrorCodes::NoError;
}

Result IO::Data::LoadComplexShipTileCompoundModel(TiXmlElement *node, ComplexShipTileDefinition *tiledef)
{
	const char *c_attr;
	string key, mtype, mcode;
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


// Loads a static terrain definition and stores it in the global collection
Result IO::Data::LoadStaticTerrainDefinition(TiXmlElement *node)
{
	// Parameter check
	if (!node) return ErrorCodes::CannotLoadTerrainDefinitionWithInvalidParams;

	// Create a new terrain definition object to store the data
	StaticTerrainDefinition *def = new StaticTerrainDefinition();

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
			def->SetDefaultExtent(IO::GetVector3FromAttr(child));
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
	if (D::GetStaticTerrain(def->GetCode()) != NULL)
	{
		SafeDelete(def);
		return ErrorCodes::CouldNotLoadDuplicateTerrainDefinition;
	}
	else
	{
		// Add to the collection and return success
		D::AddStaticTerrain(def);
		return ErrorCodes::NoError;
	}
}

// Load an instance of static terrain
StaticTerrain *IO::Data::LoadStaticTerrain(TiXmlElement *node)
{
	std::string name, val;
	StaticTerrainDefinition *def;
	D3DXVECTOR3 vpos, vextent;
	D3DXQUATERNION qorient;

	// Parameter check
	if (!node) return NULL;

	// Create a new terrain object to hold the data, and default placeholders to hold the data
	StaticTerrain *obj = new StaticTerrain();
	def = NULL;
	vpos = vextent = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	qorient = D3DXQUATERNION(0.0f, 0.0f, 0.0f, 0.0f);

	// The terrain details are fully contained within the attributes of this one elemnet
	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = attr->Name(); StrLowerC(name);

		// Take different action depending on the attribute name
		if (name == "code") 
		{
			// Attempt to get the terrain definition with this code
			val = attr->Value(); StrLowerC(val);
			def = D::GetStaticTerrain(val);								// This can be null, for e.g. pure non-renderable collision volumes
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

	// Apply the placeholder data, if it was correctly & fully set from the node attributes
	if (vpos != NULL_VECTOR) obj->SetPosition(vpos);
	if (!(qorient.x == 0.0f && qorient.y == 0.0f && qorient.z == 0.0f && qorient.w == 0.0f)) obj->SetOrientation(qorient);
	if (vextent != NULL_VECTOR) obj->SetExtent(vextent);

	// Assign the terrain definition last, if it was provided, since this will default some values (e.g. extent) if they have
	// not already been set.  Can be NULL, in the case of non-renderable collision volumes
	obj->SetDefinition(def);

	// Return a reference to the new terrain object
	return obj;
}

ProductionCost *IO::Data::LoadProductionCostData(TiXmlElement *node)
{
	// Parameter check
	if (!node) return NULL;

	// Create a new object to store this production cost data
	ProductionCost *pcost = new ProductionCost();

	// Parse the contents of this node to populate the tile definition details
	const char *cstr = NULL;
	string key, val1, val2, val3, val4;
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
	string key, val;
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
	if (D::Resources.count(res->GetCode()) != 0)
	{
		delete res; res = NULL;
	}
	else
	{
		D::AddStandardResource(res);
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
	string type = c_type;

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
	string key, val;
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		key = child->Value(); StrLowerC(key);
		if (key == "position") {				
			// Read position data from this node
			hp->Position = IO::GetVector3FromAttr(child);
		}
		else if (key == "orientation") {
			// Read the orientation D3DXQUATERNION from this node
			hp->Orientation = IO::GetD3DXQUATERNIONFromAttr(child);
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
	string key; SimpleShip *ship = NULL;
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
			string cval = val; StrLowerC(cval);			// Make lowercase, non-const std::string copy
			
			// Attempt to locate this ship; if it cannot be found then return an error now
			ship = D::GetSimpleShip(cval);
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
				string c_hp = cc_hp;
				if (c_hp != NullString) 
				{
					Hardpoint *hp = ship->GetShipHardpoints()->Get(c_hp);
					if (hp) 
					{
						// Retrieve the equipment name and validate it
						const char *cc_eq = child->Attribute("equip");
						if (cc_eq)
						{
							string c_eq = cc_eq;
							if (c_eq != NullString)
							{
								// Add a map to this loadout;  no (equip!=NULL) check since NULL is valid, to set an empty HP
								Equipment *e = D::Equipment[c_eq];
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
	if (L->Code == NullString || D::SSLoadouts.count(L->Code))
		return ErrorCodes::CannotLoadSimpleShipLoadoutWithDuplicateCode;
	else
	{
		// Add the loadout and return success
		D::AddStandardSSLoadout(L);
		return ErrorCodes::NoError;
	}
}

CompoundLoadoutMap *IO::Data::LoadCompoundLoadoutMap(TiXmlElement *node, SimpleShipLoadout *L, SimpleShip *targetshiptype)
{
	// Parameter check
	if (!node || !L || !targetshiptype) return NULL;

	// The hardpoint name should be specified as an attribute of this top-level element
	const char *cc_hp = node->Attribute("hp"); if (!cc_hp) return NULL;
	string c_hp = cc_hp;

	// Attempt to match the hardpoint to one on this ship; if it does not exist, go no further
	Hardpoint *hp = targetshiptype->GetShipHardpoints()->Get(c_hp);
	if (!hp) return NULL;

	// Create a new compound loadout object to hold the data
	CompoundLoadoutMap *map = new CompoundLoadoutMap();
	map->HP = c_hp;

	// Now look at each child element in turn and pull data from them
	string key, val;
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
				string c_eqp = cc_eqp; StrLowerC(c_eqp);

				// Make sure we have equipment specified, and a prob of >~0 (else no point)
				if (c_eqp != NullString && prob > Game::C_EPSILON)
				{
					// Attempt to locate this item of equipment and add an entry to the map
					// No (!= NULL) check since NULL is valid, to specify the chance of an empty HP in the loadout
					Equipment *e = D::GetEquipment(c_eqp);
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
	string key;
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
			e->BaseMaxThrust = e->MaxThrust = (float)atof(c_val);		// Defaults to 0.0f if no conversion possible
		}
		else if (key == "minthrust") {
			c_val = child->GetText();
			e->BaseMinThrust = e->MinThrust = (float)atof(c_val);		// Defaults to 0.0f if no conversion possible
		}
		else if (key == "acceleration") {
			c_val = child->GetText();
			e->Acceleration = (float)atof(c_val);						// Defaults to 0.0f if no conversion possible
		}
		else if (key == "emitterclass") {
			e->EmitterClass = child->GetText();
		}
	}

	// Add this engine to the global collection, IF it has at least an internal code to identify it
	if (e->Code != NullString) {
		D::AddStandardEquipment(e);
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
	std::string key, val; HashVal hash;

	// Parameter check
	if (!object || !node) return;

	// Fields to store data as it is being loaded
	D3DXVECTOR3 pos = NULL_VECTOR;
	D3DXVECTOR3 extent = NULL_VECTOR;
	D3DXQUATERNION orient = ID_QUATERNION;
	int numchildren = 0; bool skip = false;

	// Get required data from the node attributes
	for (TiXmlAttribute *attr = node->FirstAttribute(); attr; attr = attr->Next())
	{
		// Get and hash the attribute key
		key = attr->Name(); StrLowerC(key); 
		hash = HashString(key);

		// Set values based on this hash
		if (hash == HashedStrings::H_Px)					pos.x = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Py)				pos.y = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Pz)				pos.z = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ox)				orient.x = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Oy)				orient.y = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Oz)				orient.z = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ow)				orient.w = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ex)				extent.x = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ey)				extent.y = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_Ez)				extent.z = (float)atof(attr->Value());
		else if (hash == HashedStrings::H_NumChildren)		numchildren = atoi(attr->Value());
		else if (hash == HashedStrings::H_Skip)
		{
			std::string val = attr->Value(); StrLowerC(val);
			skip = (val == "true");
		}
	}
	
	// Only update the data for this node if the 'skip' flag was not set.  Usually used to skip the (auto-calculated) root obb
	if (!skip)
	{
		// We do not want to autocalculate this OBB if we have explicit data for it
		obb.SetAutoFitMode(false);

		// Build a transform matrix to determine the obb basis vectors
		D3DXMATRIX mrot, mtrans, mworld;
		D3DXMatrixRotationQuaternion(&mrot, &orient);
		D3DXMatrixTranslation(&mtrans, pos.x, pos.y, pos.z);
		mworld = (mrot * mtrans);

		// Take different action depending on whether this is the root node
		if (isroot)
		{
			// Set the obb centre, extent and axes based upon this loaded data
			obb.Data.Centre = object->GetPosition();
			obb.Data.Axis[0] = D3DXVECTOR3(mworld._11, mworld._12, mworld._13); D3DXVec3Normalize(&obb.Data.Axis[0], &obb.Data.Axis[0]);
			obb.Data.Axis[1] = D3DXVECTOR3(mworld._21, mworld._22, mworld._23); D3DXVec3Normalize(&obb.Data.Axis[1], &obb.Data.Axis[1]);
			obb.Data.Axis[2] = D3DXVECTOR3(mworld._31, mworld._32, mworld._33); D3DXVec3Normalize(&obb.Data.Axis[2], &obb.Data.Axis[2]);
			obb.Data.Extent = extent;
			obb.SetOffsetFlag(false);

			// Recalculate the OBB data
			obb.RecalculateData();
		}
		else
		{
			// This is not the root, so instead of setting axes we will set the offset matrix
			obb.Data.Centre = object->GetPosition();
			obb.Data.Extent = extent;
			obb.SetOffsetFlag(true);
			obb.Offset = mworld;
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

Result IO::Data::LoadAllModelGeometry(void)
{
	Result res			= ErrorCodes::NoError; 
	Result overallres	= ErrorCodes::NoError;

	// Iterate over each model in the central collection and load the geometry one by one
	Model::ModelCollection::iterator it_end = Model::Models.end();
	for (Model::ModelCollection::iterator it = Model::Models.begin(); it != it_end; ++it) 
	{
		if (it->second) 
		{
			// Load the model geometry
			res = IO::Data::LoadModelGeometry(it->second);
		} 
		else 
		{
			/* If object is NULL then report an error and load no mesh */
			res = ErrorCodes::CannotLoadMeshForNullObject;
		}
	
		// After each iteration we need to report any error that arises and then move onto the next object
		if (res != ErrorCodes::NoError) {
			overallres = ErrorCodes::ErrorsOccuredWhileLoadingMeshes;
			Game::Log << LOG_INIT_START << "ERROR loading model geometry for \"" << (it->second ? it->second->GetCode() : "(NULL)") << "\"\n";
		}
	}

	// Return the overall success (or otherwise) value once we have loaded as much as possible
	return overallres;
}

// Loads the geometry for the specified model
Result IO::Data::LoadModelGeometry(Model *model)
{
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
		D3DXVECTOR3 size = model->GetEffectiveModelSize();
		if (size.x < Game::C_EPSILON || size.y < Game::C_EPSILON || size.z < Game::C_EPSILON)
			model->SetEffectiveModelSize(model->GetModelSize());

		// Set the actual size now.  Method will take care of deriving missing parameters if required, so no validation needed here
		model->SetActualModelSize(model->GetActualModelSize());
	}

	// Return the result of the model loading operation
	return r;
}

// Runs post-load-processing of all model geometry as necessary
Result IO::Data::PostProcessAllModelGeometry(void)
{
	Model *model;
	INTVECTOR3 elsize;

	// Iterate over each model in the central collection and check whether each needs to be post-processed
	Model::ModelCollection::const_iterator it_end = Model::Models.end();
	for (Model::ModelCollection::const_iterator it = Model::Models.begin(); it != it_end; ++it) 
	{
		// Make sure this is a valid model
		model = it->second;				if (!model) continue;
			
		// Test whether this model has an element size specified
		elsize = model->GetElementSize();
		if (elsize.x > 0 && elsize.y > 0 && elsize.z > 0)
		{
			// An element size has been specified, so scale the mesh to be mapped onto the specified number of standard-sized game elements
			model->SetActualModelSize(Game::ElementLocationToPhysicalPosition(elsize));
		}
	}

	// Return success once all post-processing is complete
	return ErrorCodes::NoError;
}

Result IO::Data::PostProcessResources(void)
{
	Result result;
	Resource *res;
	ProductionCost *pcost;

	// Create a temporary vector of resource pointers for linear processing reasons
	vector<Resource*> resources; 
	VectorFromUnorderedMap<string, Resource*>(D::Resources, &resources);

	// First, we need to build the set of dependencies between resources based on their respective production costs
	int n = resources.size();
	for (int i = 0; i < n; i++)
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
	for (int i = 0; i < n; i++)
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
	for (int i = 0; i < n; i++)
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
	vector<ComplexShipTileDefinition*> tiles;
	VectorFromUnorderedMap<string, ComplexShipTileDefinition*>(D::ComplexShipTiles, &tiles);

	// We need to run post-processing on the tile production requirements, to link to resources/other tile dependencies
	int n = tiles.size();
	for (int i = 0; i < n; i++)
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
	string key, val;

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
			D3DXVECTOR3 size = D3DXVECTOR3(1000.0f, 1000.0f, 1000.0f);
			IO::GetVector3FromAttr(child, &size);
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
	FireEffect *e = new FireEffect(Game::Engine->GetDXLocaliser());
	
	// Local variables for extracting data
	string key, val;

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
			string filename = BuildStrFilename(D::DATA, child->GetText());	// Attempt to load the fire texture used for this effect (no error handling)
			e->SetFireTexture(filename.c_str());				
		}
		else if (key ==	"noisetexture") { 
			string filename = BuildStrFilename(D::DATA, child->GetText());	// Attempt to load the noise texture used for this effect (no error handling)
			e->SetNoiseTexture(filename.c_str());				
		}
		else if (key == "alphatexture") {
			string filename = BuildStrFilename(D::DATA, child->GetText());	// Attempt to load the alpha texture used for this effect (no error handling)
			e->SetAlphaTexture(filename.c_str());				
		}
		else if (key == "noisescrollspeed") {
			e->SetScrollSpeeds(IO::GetVector3FromAttr(child));				// 3x scroll speeds for the texture translation
		}
		else if (key == "noisescaling") {									// 3x scaling factors for the texture sampling
			e->SetScaling(IO::GetVector3FromAttr(child));
		}
		else if (key == "noisedistortion1") {
			e->SetDistortionParameters1(IO::GetVector2FromAttr(child));	// Noise distortion for sample #1
		}
		else if (key == "noisedistortion2") {
			e->SetDistortionParameters2(IO::GetVector2FromAttr(child));	// Noise distortion for sample #2
		}
		else if (key == "noisedistortion3") {
			e->SetDistortionParameters3(IO::GetVector2FromAttr(child));	// Noise distortion for sample #3
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
	string key, val;

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
			e->LoadTexture(Game::Engine->GetDevice(), BuildStrFilename(D::DATA, child->GetText()).c_str());
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
			e->SetInitialParticleLocation(ParticleEmitter::Prop::MinValue, (IO::GetVector3FromAttr(child)));
		}
		else if (key == "initialposition.max") {
			e->SetInitialParticleLocation(ParticleEmitter::Prop::MaxValue, (IO::GetVector3FromAttr(child)));
		}
		else if (key == "initialcolour.min") {
			e->SetInitialParticleColour(ParticleEmitter::Prop::MinValue, (IO::GetVector4FromAttr(child)));
		}
		else if (key == "initialcolour.max") {
			e->SetInitialParticleColour(ParticleEmitter::Prop::MaxValue, (IO::GetVector4FromAttr(child)));
		}
		else if (key == "initialsize") {
			const char *mn = child->Attribute("min");
			const char *mx = child->Attribute("max");
			if (mn) e->SetInitialParticleSize(ParticleEmitter::Prop::MinValue, (float)atof(mn));
			if (mx) e->SetInitialParticleSize(ParticleEmitter::Prop::MaxValue, (float)atof(mx));
		}
		else if (key == "initialvelocity.min") {
			e->SetInitialParticleVelocity(ParticleEmitter::Prop::MinValue, (IO::GetVector3FromAttr(child)));
		}
		else if (key == "initialvelocity.max") {
			e->SetInitialParticleVelocity(ParticleEmitter::Prop::MaxValue, (IO::GetVector3FromAttr(child)));
		}
		else if (key == "updatecolour.min") {
			e->SetParticleColourUpdate(ParticleEmitter::Prop::MinValue, (IO::GetVector4FromAttr(child)));
		}
		else if (key == "updatecolour.max") {
			e->SetParticleColourUpdate(ParticleEmitter::Prop::MaxValue, (IO::GetVector4FromAttr(child)));
		}
		else if (key == "updatesize") {
			const char *mn = child->Attribute("min");
			const char *mx = child->Attribute("max");
			if (mn) e->SetParticleSizeUpdate(ParticleEmitter::Prop::MinValue, (float)atof(mn));
			if (mx) e->SetParticleSizeUpdate(ParticleEmitter::Prop::MaxValue, (float)atof(mx));
		}
		else if (key == "updatevelocity.min") {
			e->SetParticleVelocityUpdate(ParticleEmitter::Prop::MinValue, (IO::GetVector3FromAttr(child)));
		}
		else if (key == "updatevelocity.max") {
			e->SetParticleVelocityUpdate(ParticleEmitter::Prop::MaxValue, (IO::GetVector3FromAttr(child)));
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
	string key, val;

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
			string code = child->Attribute("code");
			string texture = child->Attribute("texture");
			D3DXVECTOR3 pos = IO::GetVector3FromAttr(child);
			const char *cwidth = child->Attribute("width");
			const char *cheight = child->Attribute("height");
			string brender = child->Attribute("render");

			// Make sure we were able to retrieve all required information
			if (code == NullString || texture == NullString || !cwidth || !cheight) continue;

			// Process relevant fields to get the correct format
			StrLowerC(code); 
			string texfile = BuildStrFilename(D::DATA, texture);
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
			string code = child->Attribute("code");
			string value = child->Attribute("value");

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
			string srender; bool render;
			D3DXVECTOR4 col;
			
			// Default position
			int ix, iy;
			if (x) ix = atoi(x); else ix = 0;
			if (y) iy = atoi(y); else iy = 0;

			// Default text
			string stext = (text ? text : "");			
			const char *textbuffer = stext.c_str();

			// Default font
			int font;
			if (cfont) font = atoi(cfont); else font = Game::Fonts::FONT_BASIC1;

			// Default size
			float size;
			if (csize) size = (float)atof(csize); else size = 1.0f;

			// Default colour
			if (r && g && b && a) 
				col = D3DXVECTOR4((FLOAT)atof(r), (FLOAT)atof(g), (FLOAT)atof(b), (FLOAT)atof(a));
			else
				col = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);

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
			D3DXVECTOR4 col;
			if (r && g && b && a)
				col = D3DXVECTOR4((FLOAT)atof(r), (FLOAT)atof(g), (FLOAT)atof(b), (FLOAT)atof(a));
			else
				col = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);

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
			string edefault = child->Attribute("default");
			string ehover = child->Attribute("hover");
			string edown = child->Attribute("down");

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
			string sdef = cdef;
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
	Render2DGroup::MouseEventCollection::const_iterator it_end = group->Components.MouseEvents.Items()->end();
	for (Render2DGroup::MouseEventCollection::const_iterator it = group->Components.MouseEvents.Items()->begin(); it != it_end; ++it)
		it->second->ResolveAllComponents(group->Components.Image2D.Items());

	// Final post-processing step; disable rendering of the group by default, which also updates all components to a consistent initial state
	group->SetRenderActive(false);

	// Return success
	return ErrorCodes::NoError;
}

// Loads a 2D image group and, if successful, registers with the specified render group
Result IO::Data::LoadImage2DGroup(TiXmlElement *node, Render2DGroup *group)
{
	string key;
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
	string stexfile = BuildStrFilename(D::DATA, texture);
	const char *texfile = stexfile.c_str();
	string grouprender = (brender ? brender : ""); StrLowerC(grouprender);
	string acceptsmouse = (smouse ? smouse : ""); StrLowerC(acceptsmouse);

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
			string srender = (instrender ? instrender : "");
			string srotate = (rotate ? rotate : "");
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
	string key;
	const char *citemcode, *citemkey;
	string itemcode, itemkey;

	// Parameter check 
	if (!node || !group) return ErrorCodes::CannotLoadUIComponentGroupWithNullParameters;

	// Retrieve top-level information on the group itself
	const char *code = node->Attribute("code");
	if (!code) return ErrorCodes::CannotLoadUIComponentGroupWithNullParameters;

	// Create a new component group 
	UIComponentGroup *cg = new UIComponentGroup();
	cg->SetCode(code);

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
	group->Components.ComponentGroups.AddItem(code, cg);
	return ErrorCodes::NoError;
}

// Loads a managed control definition, that specifies all the components within a particular type of managed control
Result IO::Data::LoadUIManagedControlDefinition(TiXmlElement *node)
{
	string key;
	const char *citemkey, *citemval;
	string itemkey, itemval;

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
			string sclass = child->GetText();
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
			string filename = BuildStrFilename(D::DATA, itemval);

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
	string key = "", code = "", fname = "", texdir = "", val = "";

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
				sm->SetModelSize(IO::GetVector3FromAttr(child));
			}
			else if (key == "viewoffset") {
				sm->SetViewOffsetPercentage(IO::GetVector3FromAttr(child));
			}
		}
	}

	// Make sure the model could be created
	if (!sm || !modelcreated) return ErrorCodes::SkinnedModelCreationFailedDuringLoad; 

	// Make sure a model doesn't already exist with this code
	if (D::SkinnedModels.count(code) > 0) return ErrorCodes::CouldNotLoadDuplicateSkinnedModel;

	// Store the newly-created model in the central collection
	D::AddStandardSkinnedModel(sm);

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
	string key, val;

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
		else if (key == "attribute") {
			// Pull xml attrs for this attribute
			const char *ctype = child->Attribute("type");
			const char *cmin = child->Attribute("min");
			const char *cmax = child->Attribute("max");
			const char *cminbound = child->Attribute("minbound");
			const char *cmaxbound = child->Attribute("maxbound");
			const char *cderive = child->Attribute("derivation");

			// Check for required parameters.  Use defaults if not specified and if not mandatory
			ActorBaseAttributeData attr;
			if (!ctype)			continue;
			if (cmin)			attr.BaseMinValue = (float)atof(cmin);
			if (cmax)			attr.BaseMaxValue = (float)atof(cmax);
			if (cminbound)		attr.MinBound = (float)atof(cminbound);
			if (cmaxbound)		attr.MaxBound = (float)atof(cmaxbound);
			if (cderive)		{ string sderive = cderive; attr.DerivationType = TranslateAttributeDerivationTypeFromString(sderive); }

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
	if (D::Actors.count(a->GetCode()) > 0) return ErrorCodes::CouldNotLoadDuplicateActorBaseData;

	// Store the new actor base data in the central collection and return success
	D::AddStandardActor(a);
	return ErrorCodes::NoError;
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

