#pragma once

#ifndef __SimpleShipH__
#define __SimpleShipH__
#include "iSpaceObject.h"
#include "Ship.h"
#include "SimpleShipLoadout.h"
#include "iContainsHardpoints.h"
#include "FadeEffect.h"
#include "HighlightEffect.h"
class iObject;
class Hardpoint;
class Hardpoints;

class SimpleShip : public Ship
{

public:

	// Default constructor & destructor
	SimpleShip(void);
	~SimpleShip(void);
	
	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void				InitialiseCopiedObject(SimpleShip *source);

	// Updates the object before it is rendered.  Called only when the object is processed in the render queue (i.e. not when it is out of view)
	void				PerformRenderUpdate(void);

	// Implementation of the virtual iContainsHardpoints event method.  Invoked when the hardpoint 
	// configuration of the object is changed.  Provides a reference to the hardpoint that was changed, or NULL
	// if a more general update based on all hardpoints is required (e.g. after first-time initialisation)
	void				HardpointChanged(Hardpoint *hp);

	// Recalculates all properties, hardpoints & statistics of the ship.  Called once the ship has been created
	void				RecalculateAllShipData(void);

	// Recalculates the ship statistics based on its current state & loadout.  Called when the ship state changes during operation
	void				RecalculateShipDataFromCurrentState(void);


	// Methods to recalculate properties of this ship based on its configuration and current state
	void				CalculateShipSizeData(void);		// Recalculates ship size data based on mesh & vertex positions
	void				CalculateShipMass(void);			// Recalculate the total ship mass based on all contributing factors
	void				CalculateVelocityLimits(void);		// Recalculates velocity limit based on all contributing factors
	void				CalculateBrakeFactor(void);			// Recalculates brake factors for the ship (dependent on velocity limit)
	void				CalculateTurnRate(void);			// Recalculates the overall turn rate based on all contributing factors
	void				CalculateBankRate(void);			// Recalculates the overall turn bank based on all contributing factors
	void				CalculateBankExtents(void);			// Recalculates the ship banking extents based on all contributing factors
	void				CalculateEngineStatistics(void);	// Recalculates the ship data derived from its engine capabilities

	// Methods to create a new ship based on the specified template details
	static SimpleShip *SimpleShip::Create(const string &code);
	static SimpleShip *SimpleShip::Create(SimpleShip *template_ship);

	// Methods to apply loadouts to a ship
	Result SimpleShip::AddLoadout(SimpleShip *s);
	Result SimpleShip::AddLoadout(SimpleShip *s, const string &loadout);
	Result SimpleShip::AddLoadout(SimpleShip *s, SimpleShipLoadout *loadout);

	// Derive a camera matrix from the ship position/orientation, for use in player flying and tracking of ships
	void DeriveActualCameraMatrix(D3DXMATRIX &camoffset);

	// Effects that can be activated on this object
	FadeEffect									Fade;					// Allows the object to be faded in and out
	HighlightEffect								Highlight;

	// Camera data for when this ship is being controlled by the player
	D3DXVECTOR3		CameraPosition;			// Coordinates of the camera position
	D3DXVECTOR3		CameraRotation;			// Pitch/yaw/roll for the camera, relative to this ship
	D3DXMATRIX		CameraPositionMatrix;	// Efficiency measure; matrix derived from the camera position & orientation
	float			CameraElasticity;		// The amount by which the camera can deviate from centre when maneuvering


	// Method called when this object collides with another.  Virtual inheritance from iObject
	void										CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact);

	// Shut down the ship, deallocating all resources
	void Shutdown(void);

protected:

	

};


#endif
