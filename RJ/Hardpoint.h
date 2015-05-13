#pragma once

#ifndef __HardpointH__
#define __HardpointH__

#include "DX11_Core.h"

#include "Equip.h"
#include "Equipment.h"
#include "Ship.h"
#include "CompilerSettings.h"
#include <string>

using namespace std;
struct D3DXQUATERNION;
class Hardpoints;
class CoreEngine;
class iContainsHardpoints;


class Hardpoint
{
public:

	// Returns the type of this hardpoint.  Virtual derived method so that subclasses can return their type
	// via overriding function in the Hardpoint vftable
	virtual CMPINLINE Equip::Class	GetType() const					= 0;

	string							Code;
	D3DXVECTOR3						Position;
	D3DXQUATERNION					Orientation;

	// Virtual clone and delete methods to allow dynamic overriding by each subclass
	virtual Hardpoint *				Clone() const					= 0;
	virtual void					Delete() const					= 0;

	// Virtual mount method, which allows each subclass to mount equipment based on the specific type properties
	virtual void					MountEquipment(Equipment *e)	= 0;
	
	// Returns or sets the parent hardpoints collection that this hardpoint belongs to
	CMPINLINE Hardpoints *			GetParent()						{ return m_parent; }
	CMPINLINE void					SetParent(Hardpoints *hp)		{ m_parent = hp; }

	// Returns the equipment mounted on this hardpoint, or NULL if nothing is mounted
	CMPINLINE Equipment *			GetEquipment()					{ return m_equipment; }

	// Returns the object that ultimately owns this hardpoint, e.g. the ship or ship section
	iObject * 						GetParentObject(void);
	iContainsHardpoints  *			GetParentHPObject(void);


	// Default constructor / copy constructor / assignment operator / destructor
	Hardpoint(void);
	~Hardpoint(void);
	Hardpoint(const Hardpoint &H);
	Hardpoint& operator =(const Hardpoint &H);


protected:

	// The parent hardpoints collection
	Hardpoints *					m_parent;

	// The equipment mounted on this hardpoint
	Equipment *						m_equipment;

};






#endif