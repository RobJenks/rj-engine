
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

/*template <class T> T *Hardpoint::Copy(void)
{
	// Create a copy of this hardpoint using the custom copy constructor, templated to call the relevant subclass first
	T *h = new T(*this);

	// Return the copied hardpoint
	return h;
}*/

CMPINLINE Hardpoints *Hardpoint::GetParent() { return this->m_parent; }

// Shortcut method; follows two levels of pointers to locate the parent ship (iObject) object, performing NULL tests on the way
iObject *Hardpoint::GetShip()
{
	Hardpoints *hp = this->GetParent();
	if (!hp) return NULL;
	return hp->GetParent();
}

// Shortcut method; follows two levels of pointers to locate the parent hardpoint container (iContainsHardpoints) object, performing NULL tests on the way
iContainsHardpoints *Hardpoint::GetHardpointContainingShip(void)
{
	Hardpoints *hp = this->GetParent();
	if (!hp) return NULL;
	return hp->GetHPParent();
}
