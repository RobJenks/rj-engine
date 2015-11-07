#pragma once
#ifndef __ShipH__
#define __ShipH__

#include "DX11_Core.h"

#include "GameVarsExtern.h"
#include "Ships.h"
#include "FastMath.h"
#include "CompilerSettings.h"
#include "iSpaceObject.h"
#include "iConsumesOrders.h"
#include "iContainsHardpoints.h"
#include "iContainsTurrets.h"
#include "Equip.h"
#include <vector>
class Hardpoint;
using namespace std;

// Template class used to store ship attributes
template <typename T> class ShipAttribute
{
public:
	T				BaseValue;			// Base attribute value, before application of any modifiers
	T				Value;				// Current value of the attribute, including any modifiers

	// Convenience method to initialise all fields to the supplied value
	void SetAllValues(T val)			{ BaseValue = Value = val; }
};

// Main ship class
// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Ship : public ALIGN16<Ship>, public iSpaceObject, public iConsumesOrders, public iContainsHardpoints, public iContainsTurrets
{
public:

	// Get or set the ship class value
	CMPINLINE Ships::Class				GetShipClass(void) const		{ return m_shipclass; }
	CMPINLINE void						SetShipClass(Ships::Class cls)	{ m_shipclass = cls; }

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void								InitialiseCopiedObject(Ship *source);		

	// Default loadout is specified by its string code
	CMPINLINE std::string	GetDefaultLoadout(void) const					{ return m_defaultloadout; }
	CMPINLINE void			SetDefaultLoadout(const std::string & loadout)	{ m_defaultloadout = loadout; }

	// Primary simulation method for the ship object
	void				SimulateObject(void);

	// Simulation methods for this ship
	void				DetermineEngineThrustLevels(void);
	void				RunAllEnginesToTargetThrust(void);
	void				SimulateAllShipEngines(void);
	void				SimulateObjectPhysics(void);
	void				DetermineNewPosition(void);

	// Derives a new object world matrix
	void				aDeriveNewWorldMatrix(void);

	// Methods to adjust the target thrust of all engines on the ship
	void				SetTargetThrustOfAllEngines(float target);
	void				SetTargetThrustPercentageOfAllEngines(float target);
	void				IncrementTargetThrustOfAllEngines(void);
	void				DecrementTargetThrustOfAllEngines(void);

	// Methods to turn the ship by specified amounts, or to a specified heading
	void				TurnShip(float yaw_pc, float pitch_pc, bool bank);
	void				TurnToTarget(iObject *target, bool bank);
	void				TurnToTarget(FXMVECTOR target, bool bank);

	// Determines the yaw and pitch required to turn the ship to face a point in space.  Both values are [0.0-1.0] turn percentages
	void				DetermineYawAndPitchToTarget(FXMVECTOR target, float *pOutYaw, float *pOutPitch);
	CMPINLINE void		DetermineYawAndPitchToTarget(iObject *target, float *pOutYaw, float *pOutPitch)
						{ DetermineYawAndPitchToTarget(target->GetPosition(), pOutYaw, pOutPitch); }

	// Returns a bool indicating whether a ship can accept a specified class of order.  Overridden with additional orders by simple/complex subclasses
	bool				CanAcceptOrderType(Order::OrderType type);

	// Method to process the specified order.  Called when processing the full queue.  Return value indicates whether order is now completed & can be removed
	Order::OrderResult	ProcessOrder(Order *order);

	// Key ship properties
	float						BaseMass;					// Note: doesn't use attribute structure since current mass is stored in iActiveObject
	ShipAttribute<float>		VelocityLimit;				// The maximum velocity this ship can travel
	ShipAttribute<float>		AngularVelocityLimit;		// The maximum angular velocity this ship can sustain
	ShipAttribute<float>		BrakeFactor;				// Percentage of total velocity limit that the ship brakes can apply per second
	float						BrakeAmount;				// Absolute m/s velocity that can be reduced by ship brakes per second (no base amt req)
	ShipAttribute<float>		TurnAngle;					// Maximum turn angle, radians, calculated from ship loadout
	ShipAttribute<float>		TurnRate;					// Angular turn rate, rad/s, calculated from ship loadout
	ShipAttribute<float>		EngineAngularAcceleration;	// Derived from engine acceleration; ability of the ship to change its angular momentum
	ShipAttribute<float>		BankRate;					// Rate at which the ship will bank on turning

	// Ship size parameters
	XMVECTOR			MinBounds, MaxBounds;				// Minimum and maximum extents of the ship in world coordinates

	// Details on the ship banking range/position
	XMVECTOR			BankExtent;							// Maximum extent of the banking in each dimension
	XMVECTOR			Bank;								// The current banking position in all three dimensions

	// Runs the ship flight computer, evaluating current state and any active orders
	void				RunShipFlightComputer(void);

	// Update the collections of nearby contacts
	void				AnalyseNearbyContacts(void);

	// Methods relating to nearby contacts and AI analysis of the surrounding area
	CMPINLINE bool		HasNearbyEnemyContacts(void) const		{ return m_cached_enemy_contact_count != 0; }

	// Implementation of the virtual iContainsHardpoints event method.  Invoked when the hardpoint 
	// configuration of the object is changed.  Provides a reference to the hardpoint that was changed, or NULL
	// if a more general update based on all hardpoints is required (e.g. after first-time initialisation)
	void				HardpointChanged(Hardpoint *hp);

	// Recalculates the ship statistics based on its current state & loadout.  Called when the ship state changes during operation
	virtual void		RecalculateShipDataFromCurrentState(void) = 0;


	// Method to set the base ship mass, which will automatically recalculate the overall ship mass at the same time
	CMPINLINE void				SetBaseMass(float m)			{ this->BaseMass = m; RecalculateShipDataFromCurrentState(); }

	// Accessor methods for key properties
	CMPINLINE bool				IsBraking(void)					{ return m_isbraking; }
	CMPINLINE bool				IsTurning(void)					{ return m_isturning; }
	
	// Methods to retrieve the target flight parameters, used by the flight computer to plan engine/turn activity
	CMPINLINE XMVECTOR			GetTargetAngularVelocity(void) const			{ return m_targetangularvelocity; }
	CMPINLINE void				OverrideTargetAngularVelocity(FXMVECTOR av)		{ m_targetangularvelocity = av; }
	CMPINLINE XMVECTOR			GetEngineAngularVelocity(void) const			{ return m_engineangularvelocity; }
	CMPINLINE XMVECTOR			GetEngineAngularMomentum(void) const			{ return m_engineangularmomentum; }

	// Methods to apply/remove the ship brakes
	CMPINLINE void				ApplyBrakes(void)				{ m_isbraking = true; }
	CMPINLINE void				RemoveBrakes(void)				{ m_isbraking = false; }

	// Enable and disable ship computer control of ship functions
	CMPINLINE bool				ShipEngineControl(void)			{ return m_shipenginecontrol; }
	CMPINLINE bool				EnableShipEngineControl(void)	{ m_shipenginecontrol = true; }
	CMPINLINE bool				DisableShipEngineControl(void)	{ m_shipenginecontrol = false; }

	// Methods to set and retrieve the current ship target speed
	CMPINLINE float				GetTargetSpeed(void)			{ return m_targetspeed; }
	void						SetTargetSpeed(float target);
	void						SetTargetSpeedPercentage(float percentage);

	// Order: Moves the ship to a target position, within a certain tolerance
	Order::OrderResult			MoveToPosition(FXMVECTOR position, float closedistance);

	// Order: Moves the ship to a target object, within a certain tolerance
	Order::OrderResult			MoveToTarget(iSpaceObject *target, float closedistance);

	// Flag to determine whether any engine thrust vectors have changed
	CMPINLINE bool				ThrustVectorsChanged(void)				{ return m_thrustchange_flag; }
	CMPINLINE void				ResetThrustVectorChangeFlag(void)		{ m_thrustchange_flag = false; }
	CMPINLINE void				SetThrustVectorChangeFlag(void)			{ m_thrustchange_flag = true; }

	// Flag to determine whether the ship mass has changed
	CMPINLINE bool				ShipMassChanged(void)					{ return m_masschange_flag; }
	CMPINLINE void				ResetShipMassChangeFlag(void)			{ m_masschange_flag = false; }
	CMPINLINE void				SetShipMassChangeFlag(void)				{ m_masschange_flag = true; }

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
	// Further derived classes (e.g. ships) can implement this method and then call Ship::SimulationStateChanged() to maintain the chain
	void						SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate);

	// Terminates the ship object and deallocates storage
	void						Shutdown(void);

	// Standard constructor / destructor
	Ship(void);
	~Ship(void);


protected:
	Ships::Class		m_shipclass;				// This is either a simple or a complex ship
	std::string			m_defaultloadout;			// String ID of the default loadout to be applied to this ship

	float				m_flightcomputerinterval;			// Interval (secs) between executions of the flight computer
	float				m_timesincelastflightcomputereval;	// Time (secs) since flight computer was last executed
	bool				m_shipenginecontrol;				// Determines whether the ship will operate engines/brakes to attain target speed
	unsigned int		m_timesincelasttargetanalysis;		// Time (ms) since the last analysis of nearby targets

	float				m_targetspeed;				// The speed we want the ship flight computer to attain
	float				m_targetspeedsq;			// Precalculated squared-target speed, for inflight efficiency
	float				m_targetspeedsqthreshold;	// Precalculated 90% threshold at which we start to reduce engine thrust

	bool				m_isbraking;				// Indicates whether the ship is currently applying brakes to reduce momentum
	bool				m_isturning;				// Determines whether the ship is currently turning, and therefore whether the orientation needs to be updated each cycle
	XMVECTOR	 		m_targetangularvelocity;	// The angular velocity that the ship's engines will try to attain
	XMVECTOR			m_engineangularvelocity;	// The angular velocity that the ship's engines are currently outputting
	XMVECTOR			m_engineangularmomentum;	// The angular momentum that the ship's engines are currently outputting

	bool				m_thrustchange_flag;		// Flag indicating whether the ship thrust has changed, and requires physics recalc
	bool				m_masschange_flag;			// Flag indicating whether the ship mass has changed, and requires physics recalc

	float				m_turnmodifier;				// Modifier applied to AI ship turns, based upon their current state (peaceful, in combat, etc)
	float				m_turnmodifier_peaceful;	// Turn modifier for peaceful situations
	float				m_turnmodifier_combat;		// Turn modifier for combat situations

	XMVECTOR			m_turnrate_v, m_turnrate_nv;// Vectorised turn rate and negation for faster per-frame calculations
	XMVECTOR			m_vlimit_v, m_avlimit_v;	// Vectorised linear/angular velocity limits for faster per-frame calculations

	std::vector<iSpaceObject*>				m_cached_contacts;				// Cached collection of contacts, obtained last time the flight computer was run
	std::vector<iSpaceObject*>				m_cached_enemy_contacts;		// Cached collection of enemy contacts, obtained last time the flight computer was run
	std::vector<iSpaceObject*>::size_type	m_cached_contact_count;			// Count of cached contacts
	std::vector<iSpaceObject*>::size_type	m_cached_enemy_contact_count;	// Count of cached enemy contacts

	// Determine exact yaw and pitch to target; used for precise corrections near the target heading
	CMPINLINE float		DetermineExactYawToTarget(XMFLOAT3 tgt);
	CMPINLINE float		DetermineExactPitchToTarget(XMFLOAT3 tgt);

};

// Determines exact yaw to target; used for precise corrections near the target heading
CMPINLINE float	Ship::DetermineExactYawToTarget(XMFLOAT3 tgt) 
{
	return  -(atan2(tgt.z, tgt.x)-PIOVER2)						// Get angle, correct by 90deg, and inverse to get angle TO target...
			* Game::C_DEFAULT_FLIGHT_COMPUTER_EVAL_INTERVAL;	// ...multiplied by the eval interval, i.e. if we eval 2x a second then halve the target angle
}

// Determines exact pitch to target; used for precise corrections near the target heading
CMPINLINE float	Ship::DetermineExactPitchToTarget(XMFLOAT3 tgt) 
{
	return  -(atan2(tgt.y, tgt.z))								// Get angle, and inverse to get angle TO target...
			* Game::C_DEFAULT_FLIGHT_COMPUTER_EVAL_INTERVAL;	// ...multiplied by the eval interval, i.e. if we eval 2x a second then halve the target angle
}


#endif