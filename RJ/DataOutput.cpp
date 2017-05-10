#include "stdio.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "time.h"

#include "GameDataExtern.h"
#include "Utility.h"
#include "XMLGenerator.h"

#include "BoundingObject.h"
#include "Model.h"
#include "SimpleShip.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "ComplexShipElement.h"
#include "iContainsComplexShipTiles.h"
#include "ComplexShipTile.h"
#include "StaticTerrainDefinition.h"
#include "StaticTerrain.h"
#include "CSCorridorTile.h"
#include "Hardpoint.h"
#include "Hp.h"
#include "Engine.h"

#include "DataOutput.h"


Result IO::Data::SaveXMLDocument(TiXmlElement *root, const string &filename)
{
	// Make sure we have received all required information
	if (!root) return ErrorCodes::NullPointerToRootXMLElement;
	if (filename == NullString) return ErrorCodes::NullFilenamePointer;

	// Create a new document and link the root node to it
	TiXmlDocument *doc = new TiXmlDocument();
	doc->LinkEndChild(root);

	// Save the XML tree to disk
	if (!doc->SaveFile(filename.c_str())) { delete doc; return ErrorCodes::CouldNotSaveXMLDocument; }
	delete doc;

	// If we reach this point the save was successful
	return ErrorCodes::NoError;
}

bool IO::Data::SaveObjectData(TiXmlElement *node, iObject *object)
{
	// Parameter check
	if (!node || !object) return false;

	// Save each key iObject field below the supplied node
	// TODO: need to improve/replace the whole serialization & deserialization process.  When doing so,
	// there should be a way to distinguish between "instance" cases (which include e.g. "position") and 
	// "generic" cases (which would represent the "class" of object, and would not have such instance-type attributes)
	IO::Data::LinkStringXMLElement("Code", object->GetCode(), node);
	IO::Data::LinkStringXMLElement("Name", object->GetName(), node);
	IO::Data::LinkBoolXMLElement("StandardObject", object->IsStandardObject(), node);
	if (object->GetModel()) IO::Data::LinkStringXMLElement("Model", object->GetModel()->GetCode(), node);

	// Some fields are applicable to instance (non-archetype) objects only
	if (!object->IsStandardObject())
	{
		IO::Data::LinkVector3AttrXMLElement("Position", object->GetPosition(), node);
		IO::Data::LinkQuaternionAttrXMLElement("Orientation", object->GetOrientation(), node);
		IO::Data::LinkBoolXMLElement("Visible", object->IsVisible(), node);

		// Set the simulation state last, since when loading the XML back in the change in state will be applied as soon as this value is read
		IO::Data::LinkStringXMLElement("SimulationState", iObject::TranslateSimulationStateToString(object->SimulationState()), node);
	}


	// Return success
	return true;
}

bool IO::Data::SaveActiveObjectData(TiXmlElement *node, iActiveObject *object)
{
	// Parameter check
	if (!node || !object) return false;

	// Save data for direct inherited classes first
	IO::Data::SaveObjectData(node, object);

	// Save each key iActiveObject field below the supplied node
	IO::Data::LinkDoubleXMLElement("Mass", object->GetMass(), node);

	// Return success
	return true;
}

bool IO::Data::SaveStaticObjectData(TiXmlElement *node, iStaticObject *object)
{
	// Parameter check
	if (!node || !object) return false;

	// Save each key iStaticObject field below the supplied node
	// (No additional fields currently in scope)

	// Return success
	return true;
}

bool IO::Data::SaveSpaceObjectData(TiXmlElement *node, iSpaceObject *object)
{
	// Parameter check
	if (!node || !object) return false;

	// Save data for direct inherited classes first
	IO::Data::SaveActiveObjectData(node, object);

	// Save each key iSpaceObject field below the supplied node
	// (No additional fields currently in scope)

	// Return success
	return true;
}

bool IO::Data::SaveEnvironmentObjectData(TiXmlElement *node, iEnvironmentObject *object)
{
	// Parameter check
	if (!node || !object) return false;

	// Save data for direct inherited classes first
	IO::Data::SaveActiveObjectData(node, object);

	// Save each key iEnvironmentObject field below the supplied node
	// (No additional fields currently in scope)

	// Return success
	return true;
}

bool IO::Data::SaveSpaceObjectEnvironmentData(TiXmlElement *node, iSpaceObjectEnvironment *object)
{
	// Parameter check
	if (!node || !object) return false;

	// Save data for direct inherited classes first
	// NOTE: we do NOT follow the main object hierarchy from iSpaceObjectEnvironment.  We instead
	// follow it through the ComplexShip > Ship > iSpaceObject > iActiveObject > iObject route.
	// Passing back to iSpaceObject here would result in duplication of data from that point

	// Save each key iContainsComplexShipElements field below the supplied node
	INTVECTOR3 elsize = object->GetElementSize();
	IO::Data::LinkIntVector3AttrXMLElement("ElementSize", elsize, node);

	// Add an entry for each complex ship element
	ComplexShipElement *elements = object->GetElements(); if (!elements) return false;
	int n = object->GetElementCount();
	for (int i = 0; i < n; ++i)
	{
		IO::Data::SaveComplexShipElement(node, elements[i]);
	}

	// Return success
	return true;
}

bool IO::Data::SaveShipData(TiXmlElement *node, Ship *object)
{
	// Parameter check
	if (!node || !object) return false;

	// Save data for direct inherited classes first
	IO::Data::SaveSpaceObjectData(node, object);

	// Save each key Ship field below the supplied node
	IO::Data::LinkFloatXMLElement("VelocityLimit", object->VelocityLimit.BaseValue, node);
	IO::Data::LinkFloatXMLElement("AngularVelocityLimit", object->AngularVelocityLimit.BaseValue, node);
	IO::Data::LinkFloatXMLElement("BrakeFactor", object->BrakeFactor.BaseValue, node);
	IO::Data::LinkFloatXMLElement("TurnAngle", object->TurnAngle.BaseValue, node);
	IO::Data::LinkFloatXMLElement("TurnRate", object->TurnRate.BaseValue, node);
	IO::Data::LinkFloatXMLElement("BankRate", object->BankRate.BaseValue, node);
	IO::Data::LinkVector3AttrXMLElement("BankExtent", object->BankExtent, node);
	IO::Data::LinkStringXMLElement("DefaultLoadout", object->GetDefaultLoadout(), node);
	IO::Data::LinkStringXMLElement("VisibilityTestingMode", TranslateVisibilityModeToString(object->GetVisibilityTestingMode()), node);

	// Return success
	return true;
}





Result IO::Data::SaveSimpleShip(TiXmlElement *parent, SimpleShip *object)
{
	// Parameter check
	if (!parent || !object) return ErrorCodes::CannotSaveSimpleShipWithNullReferences;

	// Create the top-level ship node
	TiXmlElement *node = new TiXmlElement(D::NODE_SimpleShip);

	// Save data for all direct inherited classes first
	IO::Data::SaveShipData(node, object);

	// Now save any data that is specific to this class
	IO::Data::LinkVector3AttrXMLElement("CameraPosition", object->CameraPosition, node);
	IO::Data::LinkVector3AttrXMLElement("CameraRotation", (object->CameraPosition * _180BYPI), node);	// Convert radians>degrees
	IO::Data::LinkFloatXMLElement("CameraElasticity", object->CameraElasticity, node);

	// Save data on each hardpoint
	Hardpoints::IndexedHardpointCollection::const_iterator it_end = object->GetHardpoints().GetAllHardpoints().end();
	for (Hardpoints::IndexedHardpointCollection::const_iterator it = object->GetHardpoints().GetAllHardpoints().begin(); it != it_end; ++it)
	{
		// Save data on this hardpoint
		IO::Data::SaveHardpoint(node, it->second);
	}
	

	// Finally, link the new node to the parent and return success
	parent->LinkEndChild(node);
	return ErrorCodes::NoError;
}


Result IO::Data::SaveComplexShip(TiXmlElement *parent, ComplexShip *object)
{
	// Parameter check
	if (!parent || !object) return ErrorCodes::CannotSaveComplexShipWithNullReferences;

	// Vector of external files that should be included as part of this complex ship definition
	std::vector<TiXmlElement*> includefiles;

	// Create the top-level ship node
	TiXmlElement *node = new TiXmlElement(D::NODE_ComplexShip);

	// Save data for all direct inherited classes first
	IO::Data::SaveShipData(node, object);
	IO::Data::SaveSpaceObjectEnvironmentData(node, object);

	// Save all ship section data
	ComplexShip::ComplexShipSectionCollection::const_iterator s_it_end = object->GetSections()->end();
	for (ComplexShip::ComplexShipSectionCollection::const_iterator s_it = object->GetSections()->begin(); s_it != s_it_end; ++s_it)
	{
		// Record the instance of this ship section that is present in the ship
		IO::Data::SaveComplexShipSectionInstance(node, (*s_it));

		// If this is a standard section then it will exist in the central collection so we don't need to save any further 
		// details; if not, we need to save the section separately and add an 'include' reference in this file
		// NOTE: We now do not need to do this.  Sections are all generic and so we will always be referencing
		// a section that exists in the global collection
		/*if ((*s_it)->IsStandardObject() == false)
		{
			// Save the section to an external file
			SaveComplexShipSection(NULL, (*s_it));		// 'Null' parameter for parent node means that the method will store data in a new file

			// Create a new node to include this external data in the CS definition
			TiXmlElement *incl = new TiXmlElement("include");
			incl->SetAttribute("file", (*s_it)->DetermineXMLDataFullFilename().c_str());
			includefiles.push_back(incl);
		}*/	
	}

	// Save all tile data for the ship
	TiXmlElement *tile;
	iContainsComplexShipTiles::ConstTileIterator t_it_end = object->GetTiles().end();
	for (iContainsComplexShipTiles::ConstTileIterator t_it = object->GetTiles().begin(); t_it != t_it_end; ++t_it)
	{
		tile = (*t_it).value->GenerateXML();
		if (tile) node->LinkEndChild(tile);
	}

	// Now save any other data that is specific to this class
	IO::Data::LinkIntVector3AttrXMLElement("SDOffset", object->GetSDOffset(), node);

	// We want to first link any include files to the parent node, so that dependent data is always loaded first
	std::vector<TiXmlElement*>::const_iterator it_end = includefiles.end();
	for (std::vector<TiXmlElement*>::const_iterator it = includefiles.begin(); it != it_end; ++it)
	{
		node->LinkEndChild(*it);
	}

	// Finally, link the ship data itself to the parent node and return success
	parent->LinkEndChild(node);
	return ErrorCodes::NoError;
}

Result IO::Data::SaveComplexShipSection(TiXmlElement *parent, ComplexShipSection *object)
{
	// Parameter check
	if (!parent || !object) return ErrorCodes::CannotSaveComplexShipSectionWithNullReferences;

	// Create a top-level node for this ship section
	TiXmlElement *node = new TiXmlElement(D::NODE_ComplexShipSection);

	// Save data for all direct inherited classes first
	IO::Data::SaveSpaceObjectData(node, object);

	// Now save fields specific to this class
	IO::Data::LinkIntVector3AttrXMLElement("ElementLocation", object->GetElementLocation(), node);
	//IO::Data::LinkIntVectorAttrXMLElement("ElementSize", object->GetElementSize(), node);
	IO::Data::LinkIntegerXMLElement("Rotation", (int)object->GetRotation(), node);
	IO::Data::LinkFloatXMLElement("VelocityLimit", object->GetVelocityLimit(), node);
	IO::Data::LinkFloatXMLElement("AngularVelocityLimit", object->GetAngularVelocityLimit(), node);
	IO::Data::LinkFloatXMLElement("BrakeFactor", object->GetBrakeFactor(), node);
	IO::Data::LinkFloatXMLElement("TurnAngle", object->GetTurnAngle(), node);
	IO::Data::LinkFloatXMLElement("TurnRate", object->GetTurnRate(), node);
	IO::Data::LinkFloatXMLElement("BankRate", object->GetBankRate(), node);
	IO::Data::LinkVector3AttrXMLElement("BankExtent", (object->GetBankExtents() * _180BYPI), node);		// Convert radians to degrees
	IO::Data::LinkStringXMLElement("PreviewImage", object->GetPreviewImage()->GetFilename(), node);

	// Save each hardpoint in turn
	std::vector<Hardpoint*>::const_iterator it_end = object->GetHardpoints().end();
	for (std::vector<Hardpoint*>::const_iterator it = object->GetHardpoints().begin(); it != it_end; ++it)
	{
		SaveHardpoint(node, (*it));
	}

	// Link this ship section to the parent node and return success
	parent->LinkEndChild(node);
	return ErrorCodes::NoError;
}


Result IO::Data::SaveComplexShipSectionInstance(TiXmlElement *parent, ComplexShipSection *object)
{
	// Parameter check
	if (!parent || !object) return ErrorCodes::CannotSaveComplexShipSectionWithNullReferences;

	// Create a top-level node for this ship section
	TiXmlElement *node = new TiXmlElement(D::NODE_ComplexShipSectionInstance);

	// Save the necessary data for this instance
	IO::Data::LinkStringXMLElement("Code", object->GetCode(), node);
	IO::Data::LinkVector3AttrXMLElement("Position", object->GetPosition(), node);
	IO::Data::LinkIntVector3AttrXMLElement("ElementLocation", object->GetElementLocation(), node);
	IO::Data::LinkIntegerXMLElement("Rotation", (int)object->GetRotation(), node);

	// Link to the parent node and return success
	parent->LinkEndChild(node);
	return ErrorCodes::NoError;
}

Result IO::Data::SaveComplexShipElement(TiXmlElement *parent, const ComplexShipElement &e)
{
	// Parameter check
	if (!parent) return ErrorCodes::CannotSaveComplexShipElementWithNullReferences;

	// If this is an inactive element then we don't need to save anything, since this is the default
	if (!e.IsActive()) return ErrorCodes::NoError;

	// Create a node for this element, with ID specified in the attributes
	TiXmlElement *node = new TiXmlElement(D::NODE_ComplexShipElement);
	node->SetAttribute("id", e.GetID());

	// Store all other element data
	IO::Data::LinkIntVector3AttrXMLElement("elementlocation", e.GetLocation(), node);
	IO::Data::LinkIntegerXMLElement("properties", e.GetProperties(), node);
	IO::Data::LinkDoubleXMLElement("health", e.GetHealth(), node);
	IO::Data::LinkDoubleXMLElement("connections", e.GetConnectionState(), node);

	// Add all attach point data
	TiXmlElement *attach;
	for (int i = 0; i < ComplexShipElement::AttachType::_AttachTypeCount; ++i)
	{
		attach = IO::Data::NewIntegerXMLElement("attachpoint", e.GetAttachmentState((ComplexShipElement::AttachType)i));
		attach->SetAttribute("type", i);
		node->LinkEndChild(attach);
	}

	// Finally link the new node to the parent and return success
	parent->LinkEndChild(node);
	return ErrorCodes::NoError;	
}

Result IO::Data::SaveHardpoint(TiXmlElement *parent, Hardpoint *object)
{
	// Parameter check
	if (!parent || !object) return ErrorCodes::CannotSaveHardpointWithNullReferences;

	// Create a top-level node for this ship section
	TiXmlElement *node = new TiXmlElement(D::NODE_Hardpoint);

	// Set attributes on the hardpoint to specify its type and unique code
	node->SetAttribute("Type", Hp::ToString(object->GetType()).c_str());
	node->SetAttribute("Code", object->Code.c_str());

	// Now add child nodes for other required parameters
	IO::Data::LinkVector3AttrXMLElement("Position", object->Position, node);
	IO::Data::LinkQuaternionAttrXMLElement("Orientation", object->Orientation, node);

	// Link this ship section to the parent node and return success
	parent->LinkEndChild(node);
	return ErrorCodes::NoError;
}

Result IO::Data::SaveCollisionOBB(TiXmlElement *parent, OrientedBoundingBox *obb)
{
	// Parameter check
	if (!parent || !obb) return ErrorCodes::CannotSaveOBBWithNullReferences;

	// Get a reference to the core OBB data, which will also ensure it is refreshed if the data has become invalidated
	OrientedBoundingBox::CoreOBBData & obb_data = obb->Data();

	// Create the node; if the OBB is set to auto-fit then simply add a "skip" attribute
	TiXmlElement *node = new TiXmlElement("CollisionOBB");
	if (obb->AutoFitObjectBounds())
	{
		node->SetAttribute("skip", "true");
	}
	else
	{
		// If this OBB has an offset then decompose its offset matrix to determine the relative position & orientation
		if (obb->HasOffset())
		{
			// Decompose the OBB offset matrix
			XMVECTOR scale, trans, rot;
			XMMatrixDecompose(&scale, &rot, &trans, obb->Offset);
			
			// Store a local float representation
			XMFLOAT3 pos; XMStoreFloat3(&pos, trans);
			XMFLOAT4 orient; XMStoreFloat4(&orient, rot);

			// Set position
			node->SetDoubleAttribute("px", pos.x);
			node->SetDoubleAttribute("py", pos.y);
			node->SetDoubleAttribute("pz", pos.z);

			// Set orientation
			node->SetDoubleAttribute("ox", orient.x);
			node->SetDoubleAttribute("oy", orient.y);
			node->SetDoubleAttribute("oz", orient.z);
			node->SetDoubleAttribute("ow", orient.w);
		}
		else
		{
			// Set default position and orientation; only the extent can vary if the OBB has no offset
			node->SetDoubleAttribute("px", 0.0f);
			node->SetDoubleAttribute("py", 0.0f);
			node->SetDoubleAttribute("pz", 0.0f);
			node->SetDoubleAttribute("ox", 0.0f);
			node->SetDoubleAttribute("oy", 0.0f);
			node->SetDoubleAttribute("oz", 0.0f);
			node->SetDoubleAttribute("ow", 1.0f);
		}

		// Set extents
		node->SetDoubleAttribute("ex", obb_data.ExtentF.x);
		node->SetDoubleAttribute("ey", obb_data.ExtentF.y);
		node->SetDoubleAttribute("ez", obb_data.ExtentF.z);
	}

	// Specify the number of child OBBs to be allocated below this one
	int childcount = obb->ChildCount;
	node->SetAttribute("numchildren", childcount);

	// Now move recursively down into any children and add them as nodes below this one
	Result result = ErrorCodes::NoError, overallresult = ErrorCodes::NoError;
	for (int i = 0; i < childcount; ++i)
	{
		result = SaveCollisionOBB(node, &(obb->Children[i]));
		if (result != ErrorCodes::NoError) overallresult = result;
	}

	// Finally, link this new node back to the parent node and return success (or error, if propogated from children)
	parent->LinkEndChild(node);
	return overallresult;
}

// Saves the full set of connection data in the specified collection.  Only saves non-zero connection states.
// The "element_name" parameter indicates the label applied to each new element generated below the parent
Result IO::Data::SaveTileConnectionState(TiXmlElement *parent, const std::string & element_name, TileConnections *connection_data)
{
	// Parameter check
	if (!parent || !connection_data) return ErrorCodes::CannotSaveNullTileConnectionData;

	// Element name will be applied to all new elements generated under the parent
	const char *elname = element_name.c_str();
	bitstring data;

	// Iterate through the full set of possible connections
	const INTVECTOR3 & connsize = connection_data->GetElementSize();
	for (int i = 0; i < (int)TileConnections::TileConnectionType::_COUNT; ++i)
	{
		for (int x = 0; x < connsize.x; ++x)
		{
			for (int y = 0; y < connsize.y; ++y)
			{
				for (int z = 0; z < connsize.z; ++z)
				{
					// Only save data for those elements that have some ( != 0) connection data
					data = connection_data->GetConnectionState((TileConnections::TileConnectionType)i, INTVECTOR3(x, y, z));
					if (data != 0U)
					{
						TiXmlElement *conn = new TiXmlElement(elname);
						conn->SetAttribute("type", i);
						conn->SetAttribute("x", x);
						conn->SetAttribute("y", y);
						conn->SetAttribute("z", z);
						conn->SetAttribute("State", data);
						parent->LinkEndChild(conn);
					}
				}
			}
		}
	}

	// Return success once all data is saved
	return ErrorCodes::NoError;
}

Result IO::Data::SaveStaticTerrain(TiXmlElement *parent, StaticTerrain *terrain)
{
	// Parameter check
	if (!parent || !terrain) return ErrorCodes::CannotSaveTerrainWithNullReferences;

	// Create a new node to hold the terrain data
	TiXmlElement *node = new TiXmlElement(D::NODE_StaticTerrain);

	// If the terrain is tied to a definition then store the definition code here
	if (terrain->GetDefinition()) node->SetAttribute("code", terrain->GetDefinition()->GetCode().c_str());

	// Get local float representations so we can store per-component data
	XMFLOAT3 centre; XMStoreFloat3(&centre, terrain->GetOBBData().Centre);
	XMFLOAT4 orient; XMStoreFloat4(&orient, terrain->GetOrientation());

	// Add other key parameters
	node->SetDoubleAttribute("px", centre.x);
	node->SetDoubleAttribute("py", centre.y);
	node->SetDoubleAttribute("pz", centre.z);
	node->SetDoubleAttribute("ox", orient.x);
	node->SetDoubleAttribute("oy", orient.y);
	node->SetDoubleAttribute("oz", orient.z);
	node->SetDoubleAttribute("ow", orient.w);
	node->SetDoubleAttribute("ex", terrain->GetOBBData().ExtentF.x);
	node->SetDoubleAttribute("ey", terrain->GetOBBData().ExtentF.y);
	node->SetDoubleAttribute("ez", terrain->GetOBBData().ExtentF.z);

	// Link this new node to its parent and return success
	parent->LinkEndChild(node);
	return ErrorCodes::NoError;
}

Result IO::Data::SaveBoundingObject(TiXmlElement *parent, BoundingObject *bound, int id)
{
	// Make sure we have valid parameters
	if (!parent || !bound) return ErrorCodes::CannotSaveBoundingVolumeWithNullReferences;

	// Create the node and add properties common to all bounding volume types
	TiXmlElement *nbound = new TiXmlElement( "BoundingObject" );
	BoundingObject::Type type = bound->GetType();
	nbound->SetAttribute("id", id);
	nbound->SetAttribute("type", BoundingObject::TranslateTypeToString(type).c_str());

	// Now add additional type-specific properties
	if (type == BoundingObject::Type::Sphere) {
		nbound->SetDoubleAttribute("radius", bound->GetSphereRadius());
	}
	else if (type == BoundingObject::Type::Cube) {
		nbound->SetDoubleAttribute("radius", bound->GetCubeRadius());
	}
	else if (type == BoundingObject::Type::Cuboid) {
		nbound->SetDoubleAttribute("sizex", bound->GetCuboidXSize());
		nbound->SetDoubleAttribute("sizey", bound->GetCuboidYSize());
		nbound->SetDoubleAttribute("sizez", bound->GetCuboidZSize());
	}

	// Link this new node to the parent and return success
	parent->LinkEndChild(nbound);
	return ErrorCodes::NoError;
}


Result IO::Data::SaveEngine(TiXmlElement *parent, Engine *e)
{
	// Parameter check
	if (!parent || !e) return ErrorCodes::CannotSaveEngineWithNullReferences;

	// Create the top-level engine node
	TiXmlElement *node = new TiXmlElement(D::NODE_Engine);

	// Now create child nodes for basic ship data
	IO::Data::LinkStringXMLElement("Name", e->Name, node);
	IO::Data::LinkDoubleXMLElement("MaxHealth", e->GetMaxHealth(), node);
	IO::Data::LinkDoubleXMLElement("MaxThrust", e->MaxThrust, node);
	IO::Data::LinkDoubleXMLElement("Acceleration", e->Acceleration, node);

	// Finally, link the new node to the parent and return success
	parent->LinkEndChild(node);
	return ErrorCodes::NoError;
}

Result IO::Data::GenerateComplexShipRegisterXMLFile(void)
{
	// Generate a root node for the ship register
	TiXmlElement *root = IO::Data::NewGameDataXMLNode();

	// Now iterate through all ships in the register
	DataRegister<ComplexShip>::RegisterType::const_iterator it_end = D::ComplexShips.Data.end();
	for (DataRegister<ComplexShip>::RegisterType::const_iterator it = D::ComplexShips.Data.begin(); it != it_end; ++it)
	{
		// Make sure this is a valid entry in the register
		if (!it->second) continue;

		// Generate a new node for this ship
		TiXmlElement *el = new TiXmlElement( "include" );
		el->SetAttribute("file", it->second->DetermineXMLDataFullFilename().c_str());

		// Link this node to the root
		root->LinkEndChild(el);
	}

	// Save this register data to file and return the result
	return IO::Data::SaveXMLDocument(root, concat(D::DATA)("\\")(D::FILE_ComplexShipRegister).str());
}


