#include "GameDataExtern.h"
#include "RJDebug.h"
#include "Model.h"
#include "iContainsHardpoints.h"
#include "FadeEffect.h"
#include "HpEngine.h"
#include "Engine.h"
#include "CopyObject.h"
#include "Ship.h"

#include "SimpleShip.h"



SimpleShip::SimpleShip(void)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::SimpleShipObject);

	// Set the ship class
	this->m_shipclass = Ship::ShipClass::Simple;

	// This class of space object will perform full collision detection by default (iSpaceObject default = no collision)
	this->SetCollisionMode(Game::CollisionMode::FullCollision);

	// Recalculate ship stats so they are consistent with the properties set during object construction
	RecalculateShipDataFromCurrentState();
}


// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void SimpleShip::InitialiseCopiedObject(SimpleShip *source)
{
	// Pass control to all base classes
	Ship::InitialiseCopiedObject(source);

	/* Begin SimpleShip-specific initialisation logic here */

	// Recalculate ship statistics before releasing it
	RecalculateShipDataFromCurrentState();
}


// Shuts down the ship object and releases associated memory
void SimpleShip::Shutdown(void)
{
	// Deallocate all hardpoints owned by this ship
	m_hardpoints.ShutdownAllHardpoints();

	// Shutdown and deallocate the base class
	Ship::Shutdown();
}

SimpleShip::~SimpleShip(void)
{
}

// Implementation of the virtual iContainsHardpoints event method.  Invoked when the hardpoint 
// configuration of the object is changed.  Provides a reference to the hardpoint that was changed, or NULL
// if a more general update based on all hardpoints is required (e.g. after first-time initialisation)
void SimpleShip::HardpointChanged(Hardpoint *hp)
{
	// Pass to the base class for common ship event-handling first
	Ship::HardpointChanged(hp);
}

// Recalculates all properties, hardpoints & statistics of the ship.  Called once the ship has been created
void SimpleShip::RecalculateAllShipData(void)
{
	// Recalculate camera data; generate a translation & rotation matrix for camera position & rotation
	XMMATRIX offset = XMMatrixTranslationFromVector(CameraPosition);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(CameraRotation);

	// Combine the transformations and store the camera matrix
	CameraPositionMatrix = XMMatrixMultiply(offset, rot);

	// Recalculate all hardpoint data 
	m_hardpoints.RecalculateHardpoints();

	// Perform the 'light' update method as well, to update the ship statistics based on its new state & loadout
	RecalculateShipDataFromCurrentState();
}


// Recalculates the ship statistics based on its current state & loadout.  Called when the ship state changes during operation
void SimpleShip::RecalculateShipDataFromCurrentState(void) 
{
	// Perform an update of the ship based on all hardpoints
	HardpointChanged(NULL);

	// Recalculate ship properties based on the base ship details, current loadout and any modifiers
	CalculateShipSizeData();
	CalculateShipMass();
	CalculateVelocityLimits();
	//CalculateBrakeFactor();			// Called directly by velocity limit, since brake amounts are dependent on max velocity
	CalculateTurnRate();
	CalculateBankRate();
	CalculateBankExtents();
	CalculateEngineStatistics();
}

void SimpleShip::CalculateShipSizeData(void)
{
	// There is no longer much to do here for simple ships.  We no longer calculate object size based
	// upon geometry size; we now do the reverse and model size is recalculated in iObject::SetSize().  All
	// models are also now origin-centred.  We simply need to recalculate any Ship subclass-specific fields
	XMVECTOR half_size = XMVectorMultiply(GetSize(), HALF_VECTOR);
	MinBounds = XMVectorNegate(half_size);
	MaxBounds = half_size;
	SetCentreOffsetTranslation(NULL_VECTOR);
}

void SimpleShip::CalculateShipMass()
{
	// Start from the base ship hull mass
	float mass = BaseMass;

	// Apply other factors, equipment modifiers etc.

	// Assign this total mass to the ship
	this->SetMass( mass );
}

void SimpleShip::CalculateVelocityLimits()
{
	// Start from the base ship hull velocity limit
	float vlimit = this->VelocityLimit.BaseValue;
	float alimit = this->AngularVelocityLimit.BaseValue;

	// Apply other factors, equipment modifiers etc.

	// Assign the final values after applying modifiers
	this->VelocityLimit.Value = vlimit;
	this->AngularVelocityLimit.Value = alimit;

	// Also store in vectorised form for fast per-frame calculations
	m_vlimit_v = XMVectorReplicate(VelocityLimit.Value);
	m_avlimit_v = XMVectorReplicate(AngularVelocityLimit.Value);

	// Also trigger a recalculation of the hull brake factors, which are dependent on this calculation
	CalculateBrakeFactor();
}

void SimpleShip::CalculateBrakeFactor()
{
	// Start from the base ship hull brake factor
	float brake = this->BrakeFactor.BaseValue;

	// Apply other factors, equipment modifiers etc.

	// Store the final value after applying modifiers
	this->BrakeFactor.Value = brake;

	// Calculate the absolute brake amount as well for efficiency.  Dependency on velocity limit being set.
	this->BrakeAmount = (this->BrakeFactor.Value * this->VelocityLimit.Value);
}

void SimpleShip::CalculateTurnRate()
{
	// Start from the base ship hull turn rate
	float turnrate = this->TurnRate.BaseValue;
	float turnangle = this->TurnAngle.BaseValue;

	// Apply other factors, equipment modifiers etc.

	// Store the final values after applying modifiers
	this->TurnRate.Value = turnrate;
	this->TurnAngle.Value = turnangle;

	// Also store turn rate in vectorised form for fast per-frame calculations
	m_turnrate_v = XMVectorReplicate(TurnRate.Value);
	m_turnrate_nv = XMVectorNegate(m_turnrate_v);
}

void SimpleShip::CalculateBankRate()
{
	// Start from the base ship hull turn rate
	float bankrate = this->BankRate.BaseValue;

	// Apply other factors, equipment modifiers etc.

	// Store the final value after applying modifiers
	this->BankRate.Value = bankrate;
}

void SimpleShip::CalculateBankExtents()
{
	// Nothing to do here; bank extent is currently specified directly on loading and does not change at runtime
}

void SimpleShip::CalculateEngineStatistics()
{
	// Retrieve the vector of all engine hardpoints on the ship
	Hardpoints::HardpointCollection & engines = m_hardpoints.GetHardpointsOfType(Equip::Class::Engine);

	// Now sum the total engine acceleration value from each engine to determine an engine angular acceleration value
	HpEngine *hp; Engine *e; float accel = 0.0f;
	int n = (int)engines.size(); 
	
	for (int i = 0; i < n; ++i)
	{
		// Attempt to get a reference to the hardpoint, and the engine mounted on it (if there is one)
		hp = (HpEngine*)engines.at(i);	if (!hp) continue;
		e = (Engine*)hp->GetEngine();	if (!e) continue;

		// Add the engine acceleration 
		accel += e->GetAcceleration();
	}

	// Store the base value before applying any modifiers
	this->EngineAngularAcceleration.BaseValue = accel;

	// Apply any modifiers

	// Store the final value once modifiers have been applied
	this->EngineAngularAcceleration.Value = accel;
}

SimpleShip *SimpleShip::Create(const std::string & code)
{
	// Invoke the spawn function using a template ship corresponding to this definition (if one exists)
	return (SimpleShip::Create(D::SimpleShips.Get(code)));
}

SimpleShip *SimpleShip::Create(SimpleShip *template_ship)
{
	// If we are passed an invalid class pointer then return NULL immediately
	if (template_ship == NULL) return NULL;

	// Create a new instance of the ship from this template; class-specific initialisation will all be performed
	// automatically up the inheritance hierarchy as part of this operation.  If any part of the operation fails, 
	// the returned value will be NULL (which we then pass on as the return value from this function)
	SimpleShip *ss = CopyObject<SimpleShip>(template_ship);

	// Return a pointer to the newly-created ship (or NULL if creation or initialisation failed)
	return ss;
}

Result SimpleShip::AddLoadout(SimpleShip *s)
{
	// Assigns the default loadout for this class, if one exists
	
	// Get the default loadout for this ship class, if one exists
	std::string & l = m_defaultloadout;
	if (l.empty()) return ErrorCodes::ShipHasNoDefaultLoadoutSpecified;			// If no default loadout return error

	// Call the function which takes a string representation of the loadout code as its argument
	return (SimpleShip::AddLoadout(s, l));
}

Result SimpleShip::AddLoadout(SimpleShip *s, const std::string &loadout)
{
	// Assigns a loadout with the specified code
	
	// Make sure the ship & loadout code are valid
	if (!s || !loadout.empty()) return ErrorCodes::InvalidParametersForShipLoadoutAssignment;

	// Make sure the supplied loadout code exists
	SimpleShipLoadout *L = D::SSLoadouts.Get(loadout);
	if (!L) return ErrorCodes::CannotAssignNullLoadoutToShip;

	// If the loadout is valid then call the function to assign a loadout to a ship
	return (SimpleShip::AddLoadout(s, loadout));
}

Result SimpleShip::AddLoadout(SimpleShip *s, SimpleShipLoadout *loadout)
{
	// Validate that the supplied pointers are valid
	if (!s || !loadout) return ErrorCodes::InvalidParametersForShipLoadoutAssignment;

	// Assign the loadout to this ship and return value indicating success or not
	return (SimpleShipLoadout::AssignLoadoutToSimpleShip(s, loadout));
}


XMMATRIX SimpleShip::DeriveActualCameraMatrix(void)
{
	// Generate a translation matrix to account for the camera elasticity
	// We only need to test if the bank x/y amount is nonzero; the other coords are undefined
	if (IsZeroVector2(Bank))
	{
		// No offset due to camera elasticity is required, so simply return the normal camera offset matrix
		return CameraPositionMatrix;
	}
	else
	{
		// Trans vector: [(Bank.y / BankExtent.y) * (-CameraElasticity), (Bank.x / BankExtent.x) * (-CameraElasticity), 0.0f]
		XMVECTOR trans = XMVectorDivide(Bank, BankExtent);										// [B.x/BE.x, B.y/BE/y, B.z/BE.z, B.w/BE.w]
		trans = XMVectorSetW(trans, 0.0f);														// [B.x/BE.x, B.y/BE.y, B.z/BE.z, 0.0f]
		trans = XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_W>(trans);	// [B.y/BE.y, B.x/BE.x, 0, 0]
		trans = XMVectorScale(trans, CameraElasticity);											// [B.y/BE.y * -CE, B.x/BE.x * -CE, 0, 0]
		
		// Use this translation vector to build the adjusted camera matrix
		return XMMatrixMultiply(XMMatrixTranslationFromVector(trans), CameraPositionMatrix);
	}
}

// Method called when this object collides with another.  Virtual inheritance from iSpaceObject
void SimpleShip::CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact)
{
	// Pass to the base class method
	iActiveObject::CollisionWithObject(object, impact);
}

// Event triggered upon destruction of the object
void SimpleShip::DestroyObject(void)
{
	// Generate destruction effects and debris

	// Perform any other SimpleShip-specific destruction logic here

	// Call the base class method
	Ship::DestroyObject();

	// Now call the shutdown method to remove this entity from the simulation
	Shutdown();
	OutputDebugString("Destruction of SimpleShip\n");
}


// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void SimpleShip::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods

	// Mutator methods
	REGISTER_DEBUG_FN(RecalculateAllShipData)
	REGISTER_DEBUG_FN(RecalculateShipDataFromCurrentState)

	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		Ship::ProcessDebugCommand(command);

}
