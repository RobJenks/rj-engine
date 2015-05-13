
#include "Equipment.h"
#include "iContainsHardpoints.h"
#include "Hardpoints.h"
#include "CompilerSettings.h"

#include "Hardpoint.h"

Hardpoint::Hardpoint(void)
{
	// Start off with no pointer to parent collection or to mounted equipment
	this->m_parent = NULL;
	this->m_equipment = NULL;

	// Default values for all other fields
	this->Code = "";
	this->Position =	D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	this->Orientation = D3DXQUATERNION (ID_QUATERNION);

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
	this->Position = D3DXVECTOR3(H.Position);
	this->Orientation = D3DXQUATERNION(H.Orientation);
}

Hardpoint& Hardpoint::operator =(const Hardpoint &H)
{
	// Start off with no pointer to parent or mounted equipment
	this->m_parent = NULL;
	this->m_equipment = NULL;

	// All other fields copied via member-wise clone
	this->Code = H.Code;
	this->Position = D3DXVECTOR3(H.Position);
	this->Orientation = D3DXQUATERNION(H.Orientation);

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
