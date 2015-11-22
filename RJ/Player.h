#pragma once

#ifndef __PlayerH__
#define __PlayerH__

#include "FastMath.h"
#include "Utility.h"
class GameInputDevice;
class Ship;
class ComplexShip;
class ComplexShipSection;
class ComplexShipElement;
class SpaceSystem;
class Actor;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Player : public ALIGN16<Player>
{

public:
	
	// Enumeration of possible player states
	enum StateType { UnknownState = 0, ShipPilot, OnFoot };

	// Enumeration used to indicate whether the player is forced to appear in a particular environment, for e.g. special/cutscene rendering
	enum PlayerEnvironmentOverrideType { NoOverride = 0, SpaceEnvironmentOverride };

	// Method to retrieve the current player state.  Read-only
	CMPINLINE StateType				GetState(void)							{ return m_state; }

	// Methods to get and set the current player actor
	CMPINLINE Actor *				GetActor(void)							{ return m_actor; }
	void							SetActor(Actor *actor);

	// Methods to get and set the current player ship
	CMPINLINE Ship *				GetPlayerShip(void)						{ return m_playership; }
	void							SetPlayerShip(Ship *ship);

	// Retrieve the system that this player is currently in; either directly if piloting a ship, or indirectly
	// if on foot (in which case this returns the system that our parent environment resides in)
	CMPINLINE SpaceSystem *			GetSystem(void)							{ return m_systemenv; }

	// Retrieve the player's parent environment.  Only relevant if state == OnFoot
	CMPINLINE iSpaceObjectEnvironment *	GetParentEnvironment(void)			{ return m_parentenv; }

	// Methods to force-set the player environment , without the player entity needing to be there.  Used for e.g.
	// rendering scenes within the null system.  Will revert to normal once released.  Game should generally be paused first.
	void							OverridePlayerEnvironment(SpaceSystem *system, Ship *playership, const FXMVECTOR position, const FXMVECTOR orientation);
	void							ReleasePlayerEnvironmentOverride(void);

	// Position and orientation of the player (derived either from on-foot or ship state)
	CMPINLINE XMVECTOR				GetPosition(void)						{ return m_position; }
	CMPINLINE XMVECTOR				GetOrientation(void)					{ return m_orientation; }
	CMPINLINE XMVECTOR				GetEnvironmentPosition(void)			{ return m_envposition; }
	CMPINLINE XMVECTOR				GetEnvironmentOrientation(void)			{ return m_envorientation; }
	
	// Change the current player position/orientation.  Takes different action depending on whether the player is on foot or in a ship
	void							SetPosition(const FXMVECTOR pos);
	void							SetOrientation(const FXMVECTOR orient);

	// Offset of the camera from the player position & orientation.  Only required for on-foot camera (otherwise is taking from ship camera details)
	CMPINLINE XMVECTOR				GetViewPositionOffset(void)		{ return m_viewpositionoffset; }
	CMPINLINE XMVECTOR				GetViewOrientationOffset(void)	{ return m_vieworientationoffset; }
	void							SetViewPositionOffset(const FXMVECTOR pos);
	void							SetViewOrientationOffset(const FXMVECTOR orient);
	void							SetViewPositionAndOrientationOffset(const FXMVECTOR pos, const FXMVECTOR orient);
	CMPINLINE const XMMATRIX &		GetViewOffsetMatrix(void) const	{ return m_viewoffsetmatrix; }

	// Methods to retrieve the current player element and location, when located inside a complex ship
	CMPINLINE INTVECTOR3			GetComplexShipEnvironmentElementLocation(void)	{ return m_complexshipenvelementlocation; }
	CMPINLINE ComplexShipElement *	GetComplexShipEnvironmentElement(void)			{ return m_complexshipenvelement; }
	
	// Methods to set the player environment
	void							EnterEnvironment(SpaceSystem *system);
	void							EnterEnvironment(ComplexShip *complexship);

	// Moves the player back into their own ship, whether simple or complex
	void							ReturnToShip(void);
	
	// Returns a reference to the active player spaceobject; will either be the player character or the ship they are piloting, depending on state
	CMPINLINE iObject *				GetActivePlayerObject(void) const 
									{ return (m_state == Player::StateType::ShipPilot ? (iObject*)m_playership : (iObject*)m_actor); }

	// Initialises the player for a new simulation cycle
	void							BeginSimulationCycle(void);

	// Updates the player state; called once per cycle before rendering begins
	void							UpdatePlayerState(void);

	// Moves the player in the specified direction.  Only options are forward/back (normal walking) and left/right (strafing)
	void							MovePlayerActor(Direction direction, bool run);

	// Rotates the on-foot player view by the specified amounts in the Y and X axes
	void							RotatePlayerView(float y, float x);

	// Attempts to perform a jump as the player actor
	void							ActorJump(void);

	// Accepts keyboard input and updates the player state accordingly
	void							AcceptKeyboardInput(GameInputDevice *keyboard);

	// Methods to change the player mouse control mode when piloting a ship
	void							ToggleShipMouseControlMode(void);
	void							SetShipMouseControlMode(MouseInputControlMode mode);

	// Constructor / destructor
	Player(void);
	~Player(void);

public:
	
	// The actor representing this player
	Actor *					m_actor;

	// The current player ship (either simple or complex, where complex ship is the one owned by the player [even if not currently in it]
	Ship *					m_playership;

	// Store the player state; either in a ship or on foot
	StateType				m_state;

	// Fields storing the player environment in each possible form
	SpaceSystem *				m_systemenv;
	iSpaceObjectEnvironment *	m_parentenv;

	// Efficiency measure; stores the ship element which the player is currently located in (if env = complexship)
	INTVECTOR3				m_complexshipenvelementlocation;
	ComplexShipElement *	m_complexshipenvelement;
	
	// The player position & orientation.  Absolute and relative (to environment) values are stored for efficiency
	AXMVECTOR				m_position, m_envposition;
	AXMVECTOR				m_orientation, m_envorientation;

	// Matrix representing the orientation of the player in world space
	AXMMATRIX				m_orientationmatrix;

	// View offset from the player position & orientation.  Only relevant for on-foot camera (otherwise is automatically pulled from the ship camera details)
	AXMVECTOR				m_viewpositionoffset;
	AXMVECTOR				m_vieworientationoffset;
	AXMMATRIX				m_viewoffsetmatrix;

	// Stores the rotation (in radians) about the Y and X axis, plus any pending rotations, for on-foot camera view
	float					m_viewpitch;

	// Flags storing the current state of the player
	bool					m_ismoving;

	// Fields storing the movement parameters for this player.  Inherited from the actor representing the player
	float					m_runspeed, m_walkspeed, m_strafespeed;

	// Stores data relating to the player 'head-bob', a view position offset to better simulate movement
	float					m_headbobmax, m_headbobspeed;		// Maximum height of offset, and amount per second
	float					m_headbobposition;					// Current height of the offset
	bool					m_headbobascending;					// Whether or not the movement is currently upwards

	// Recalculates the view offset matrix for the player character
	void					RecalculatePlayerOffsetMatrix(void);

	// Private sub-methods of the main state update method
	void					UpdatePlayerActorState(void);
	void					UpdatePlayerShipState(void);

	// Methods used to override and then restore the player environment, for e.g. special/cutscene rendering
	void								ExecuteOverrideOfPlayerEnvironment(void);
	PlayerEnvironmentOverrideType		m_envoverride;
	StateType							m_overridepriorstate;
	SpaceSystem *						m_overridesystem;
	AXMVECTOR							m_overrideposition;
	AXMVECTOR							m_overrideorientation;
	Ship *								m_overrideship;
	Ship *								m_overridepriorship;
};





#endif
