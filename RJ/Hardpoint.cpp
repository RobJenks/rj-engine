
#include "Equipment.h"
#include "iContainsHardpoints.h"
#include "Hardpoints.h"
#include "CompilerSettings.h"
#include "FileInput.h"

#include "Hardpoint.h"

Hardpoint::Hardpoint(void)
{
	// Start off with no pointer to parent collection or to mounted equipment
	this->m_parent = NULL;
	this->m_equipment = NULL;

	// Default values for all other fields
	this->Code = "";
	this->Position = NULL_VECTOR;
	this->Orientation = ID_QUATERNION;

}

Hardpoint::~Hardpoint(void)
{
	this->m_parent = NULL;
	this->m_equipment = NULL;
}

Hardpoint::Hardpoint(const Hardpoint &H)
{
	// Start off with no pointer to parent collection or to mounted equipment
	this->m_parent = NULL;
	this->m_equipment = NULL;

	// All other fields copied via member-wise clone
	this->Code = H.Code;
	this->Position = H.Position;
	this->Orientation = H.Orientation;
}

Hardpoint& Hardpoint::operator =(const Hardpoint &H)
{
	// Start off with no pointer to parent or mounted equipment
	this->m_parent = NULL;
	this->m_equipment = NULL;

	// All other fields copied via member-wise clone
	this->Code = H.Code;
	this->Position = H.Position;
	this->Orientation = H.Orientation;

	return *this;
}

// Shortcut method; follows two levels of pointers to locate the parent object (iObject)
// object, performing NULL tests on the way
iObject *Hardpoint::GetParentObject(void)
{
	Hardpoints *hp = this->GetParent();
	return (hp ? hp->GetParent() : NULL);
}


// Shortcut method; follows two levels of pointers to locate the parent hardpoint container (iContainsHardpoints) 
// object, performing NULL tests on the way
iContainsHardpoints *Hardpoint::GetParentHPObject(void)
{
	Hardpoints *hp = this->GetParent();
	return (hp ? hp->GetHPParent() : NULL);
}

// Load an item of common hardpoint data from XML.  Accepts the hashed item key as a parameter
// to avoid duplication of effort.  Returns a result indicating whether the data was accepted
Result Hardpoint::ReadBaseHardpointXML(TiXmlElement *node, HashVal hashed_key)
{
	if (!node) return ErrorCodes::CannotLoadHardpointDataFromNullResources;
	
	// Test against all base class properties
	if (hashed_key == HashedStrings::H_Position)			Position = IO::GetVector3FromAttr(node);
	else if (hashed_key == HashedStrings::H_Orientation)	Orientation = IO::GetQuaternionFromAttr(node);
	
	else return ErrorCodes::CouldNotLoadUnrecognisedHardpointProperty;

	// If we reach this point we DID successfully process the element
	return ErrorCodes::NoError;
}




