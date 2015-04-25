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
	virtual CMPINLINE Equip::Class	GetType() const = 0;

	string				Code;
	D3DXVECTOR3			Position;
	D3DXQUATERNION		Orientation;

	// Virtual clone and delete methods to allow dynamic overriding by each subclass
	virtual Hardpoint * Clone() const = 0;
	virtual void		Delete() const = 0;

	// Virtual mount method, which allows each subclass to mount equipment based on the specific type properties
	virtual void		MountEquipment(Equipment *e) = 0;
	
	Equipment *					GetEquipment(void);
	CMPINLINE Hardpoints *		GetParent(void);
	iObject  *					GetShip(void);
	iContainsHardpoints *		GetHardpointContainingShip(void);

	Hardpoint(void);
	~Hardpoint(void);
	Hardpoint(const Hardpoint &H);
	Hardpoint& operator =(const Hardpoint &H);

	Hardpoints *		m_parent;
	Equipment *			m_equipment;
};

CMPINLINE Equipment *Hardpoint::GetEquipment() { return m_equipment; }




#endif