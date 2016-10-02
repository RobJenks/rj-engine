#pragma once

#ifndef __CapitalShipPerimeterBeaconH__
#define __CapitalShipPerimeterBeaconH__

#include "iSpaceObject.h"
#include "GameVarsExtern.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class CapitalShipPerimeterBeacon : public ALIGN16<CapitalShipPerimeterBeacon>, public iSpaceObject 
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(CapitalShipPerimeterBeacon)

	// Default constructor
	CapitalShipPerimeterBeacon(void);

	// Destructor; attempts to break the attachment to any parent ship before deallocating the beacon
	~CapitalShipPerimeterBeacon(void);

	// Methods to retrieve the parent ship details for this beacon
	CMPINLINE iSpaceObject *	GetParentShip(void)			{ return m_parentship; }
	CMPINLINE Game::ID_TYPE		GetParentShipID(void)		{ return m_parentshipid; }
	
	// Links this beacon to the specified parent ship, creating space object attachments to keep it in place during movement
	void						AssignToShip(iSpaceObject *ship, FXMVECTOR position);

	// Implemented to satisfy iObject interface.  In reality all beacon positions are determined by the parent ship
	CMPINLINE void				SimulateObject(void) { }

	// Virtual inheritance from iObject.  This object does not collide with anything so do nothing here.
	CMPINLINE void				CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact) { }

	// Virtual inheritance from iObject.  There are no intra-frame activities we can take to refresh our data, since we 
	// are effectively just a wrapper around a position
	CMPINLINE void				RefreshPositionImmediate(void) { }
	
	// Event triggered upon destruction of the object; inherited from iObject/iTakesDamage and not required here
	CMPINLINE void				DestroyObject(void) { }

	// Shutdown method for a beacon, to break any link to a parent ship and deallocate all resources 
	void						Shutdown(void);

public:
	bool						Active;							// If the beacon is currently being simulated in the world
	AXMVECTOR					BeaconPos;						// Position of the beacon relative to the parent ship

private:

	iSpaceObject *				m_parentship;					// SpaceObject that this beacon is attached to
	Game::ID_TYPE				m_parentshipid;					// SpaceObject ID of the ship this beacon is attached to
	
};


#endif