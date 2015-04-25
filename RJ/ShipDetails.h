#pragma once
#ifndef __ShipDetailsH__
#define __ShipDetailsH__

#include "Ships.h"
#include "Hardpoints.h"
#include "CompilerSettings.h"
class Model;


class ShipDetails
{
public:
	int					ID;					// A unique ID assigned upon initialisation
	Ships::Class		Category;			// This is either a simple or a complex ship
	string				Code;				// The text code uniquely identifying this object
	string				Name;				// The string name of this ship
	bool				IsStandardShip;		// Flag indicating whether this is a standard ship definition, held in the central collection & reused by many ships
	string				DefaultLoadout;		// The default loadout for this ship class

	Model				*Model;				// Pointer to our ship model in the static vector collection

	Ship				*m_parent;			// The parent (ship) object that we are assigned to.  Used for more efficient callbacks
	Ship				*GetParent(void);	// Retrieves pointer to the parent object
	void				SetParent(Ship*);	// Sets our parent object

	Hardpoints			*HP;				// The hardpoint collection for this ship

	void				SetHullMass(float m);	// Sets mass, performs 0/-ve bounds checking
	float				HullMass;				// Mass of the entire ship.  Complex ships must implement a method to
												// recalculate mass & other vital statistics upon deforming/destruction
	
	void				SetHullVelocityLimit(float v);	// Sets the ship hull velocity limit, which contributes to overall limit
	float				HullVelocityLimit;				// The maximum velocity this ship hull can withstand

	void				SetHullAngularVelocityLimit(float v);	// Sets the ship hull angular velocity limit
	float				HullAngularVelocityLimit;				// The maximum angular velocity this ship hull can withstand

	void				SetHullBrakeFactor(float f);	// Sets the ship hull brake factor, dependent on the velocity limit
	float				HullBrakeFactor;				// The percentage of total velocity limit that the ship brakes can apply per second

	void				SetHullTurnRate(float r);		// Sets the hull turn rate, which contributes to overall ship turn rate
	float				HullTurnRate;					// The turn rate in rad/s

	void				SetHullTurnAngle(float a);		// Sets the hull turn angle, which contributes to overall ship max turning angle
	float				HullTurnAngle;					// The max turn angle in rad

	D3DXVECTOR3			HullBankExtent;					// The banking extent per x/y/z, in radians
	float				HullBankRate;					// The % of our total banking extent we can cover in 1 second

	// Virtual method to retreve the ship hardpoint collection
	virtual iHardpoints	*		GetHardpoints(void) = 0;

	// Shutdown method to deallocate all ship details data
	void						Shutdown(void);

	ShipDetails(void);
	~ShipDetails(void);
};

CMPINLINE Ship *ShipDetails::GetParent() { return this->m_parent; }



#endif