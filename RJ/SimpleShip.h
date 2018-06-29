#pragma once

#ifndef __SimpleShipH__
#define __SimpleShipH__
#include "iSpaceObject.h"
#include "Ship.h"
#include "SimpleShipLoadout.h"
class iObject;
class Hardpoint;
class Hardpoints;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class SimpleShip : public Ship
{

public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(SimpleShip)

	// Default constructor & destructor
	SimpleShip(void);
	~SimpleShip(void);
	
	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void				InitialiseCopiedObject(SimpleShip *source);

	// Implementation of the virtual iContainsHardpoints event method.  Invoked when the hardpoint 
	// configuration of the object is changed.  Provides a reference to the hardpoint that was changed, or NULL
	// if a more general update based on all hardpoints is required (e.g. after first-time initialisation)
	void				HardpointChanged(Hardpoint *hp);

	// Recalculates all properties, hardpoints & statistics of the ship.  Called once the ship has been created
	void				RecalculateAllShipData(void);

	// Recalculates the ship statistics based on its current state & loadout.  Called when the ship state changes during operation
	void				RecalculateShipDataFromCurrentState(void);

	// Methods to create a new ship based on the specified template details
	static SimpleShip *SimpleShip::Create(const std::string &code);
	static SimpleShip *SimpleShip::Create(SimpleShip *template_ship);

	// Methods to apply loadouts to a ship
	Result SimpleShip::AddLoadout(SimpleShip *s);
	Result SimpleShip::AddLoadout(SimpleShip *s, const std::string &loadout);
	Result SimpleShip::AddLoadout(SimpleShip *s, SimpleShipLoadout *loadout);

	// Derive a camera matrix from the ship position/orientation, for use in player flying and tracking of ships
	XMMATRIX DeriveActualCameraMatrix(void);

	// Camera data for when this ship is being controlled by the player
	AXMVECTOR		CameraPosition;			// Coordinates of the camera position
	AXMVECTOR		CameraRotation;			// Pitch/yaw/roll for the camera, relative to this ship
	AXMMATRIX		CameraPositionMatrix;	// Efficiency measure; matrix derived from the camera position & orientation
	float			CameraElasticity;		// The amount by which the camera can deviate from centre when maneuvering


	// Method called when this object collides with another.  Virtual inheritance from iObject
	void										CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact);

	// Event triggered upon destruction of the object
	void										DestroyObject(void);

	// Shut down the ship, deallocating all resources
	void Shutdown(void);

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void									ProcessDebugCommand(GameConsoleCommand & command);


protected:

	// Methods to recalculate properties of this ship based on its configuration and current state
	void				CalculateShipSizeData(void);		// Recalculates ship size data based on mesh & vertex positions
	void				CalculateShipMass(void);			// Recalculate the total ship mass based on all contributing factors
	void				CalculateVelocityLimits(void);		// Recalculates velocity limit based on all contributing factors
	void				CalculateBrakeFactor(void);			// Recalculates brake factors for the ship (dependent on velocity limit)
	void				CalculateTurnRate(void);			// Recalculates the overall turn rate based on all contributing factors
	void				CalculateBankRate(void);			// Recalculates the overall turn bank based on all contributing factors
	void				CalculateBankExtents(void);			// Recalculates the ship banking extents based on all contributing factors
	void				CalculateEngineStatistics(void);	// Recalculates the ship data derived from its engine capabilities


};


#endif
