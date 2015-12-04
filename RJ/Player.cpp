#include "Utility.h"
#include "FastMath.h"
#include "GameInput.h"
#include "Ship.h"
#include "Ships.h"
#include "GameDataExtern.h"
#include "ComplexShip.h"
#include "ComplexShipTile.h"
#include "iObject.h"
#include "Actor.h"
#include "ActorBase.h"
#include "ActorAttributes.h"
#include "MovementLogic.h"
#include "iSpaceObjectEnvironment.h"
class ComplexShipElement;
class ComplexShipSection;

#include "Player.h"


Player::Player(void)
{
	// Initialise player state to be unknown until specified
	m_state = StateType::UnknownState;

	// Initialise key fields to default values
	m_actor = NULL;
	m_playership = NULL;
	m_systemenv = NULL;
	m_parentenv = NULL;
	m_complexshipenvelement = NULL;
	m_complexshipenvelementlocation = INTVECTOR3(0, 0, 0);
	m_position = NULL_VECTOR;
	m_envposition = NULL_VECTOR;
	m_orientation = ID_QUATERNION;
	m_envorientation = ID_QUATERNION;
	m_viewpositionoffset = XMLoadFloat3(&Game::C_ACTOR_DEFAULT_VIEW_OFFSET);
	m_vieworientationoffset = ID_QUATERNION;
	m_viewoffsetmatrix = ID_MATRIX;
	m_orientationmatrix = ID_MATRIX;
	m_viewpitch = 0.0f;
	m_runspeed = m_walkspeed = m_strafespeed = 0.0f;
	m_headbobposition = 0.0f;
	m_headbobascending = true;
	m_ismoving = false;
	m_envoverride = PlayerEnvironmentOverrideType::NoOverride;
	m_overridepriorstate = Player::StateType::UnknownState;
	m_overridesystem = NULL;
	m_overrideposition = NULL_VECTOR;
	m_overrideorientation = ID_QUATERNION;
	m_overridepriorship = NULL;

	// Initialise default player values that will in future be set from external sources
	m_headbobmax = Game::C_ACTOR_DEFAULT_HEAD_BOB_AMOUNT;
	m_headbobspeed = Game::C_ACTOR_DEFAULT_HEAD_BOB_SPEED;
}


// Updates the player state; called once per cycle before rendering begins
void Player::UpdatePlayerState(void)
{
	// Assuming the player state is not manually overriden it will be updated from the relevant entities
	if (m_envoverride == Player::PlayerEnvironmentOverrideType::NoOverride)
	{
		// Take different action depending on the current player environment
		if (m_state == Player::StateType::ShipPilot && m_playership)
		{
			// If the player is currently piloting a ship
			UpdatePlayerShipState();
		}
		else if (m_state == Player::StateType::OnFoot && m_actor)
		{
			// If the player is currently on foot
			UpdatePlayerActorState();
		}
	}
	else
	{
		// If the override is active, directly override the relevant fields with stored parameters now
		ExecuteOverrideOfPlayerEnvironment();
	}
}

void Player::UpdatePlayerShipState(void)
{
	// If the player is piloting a ship then inherit position/orientation from the player ship.  
	// Master source = ship in this case.  Easier since ship will calculate all trajectories etc for us
	m_position		= m_envposition			= m_playership->GetPosition();
	m_orientation	= m_envorientation		= m_playership->GetOrientation();

	// Store a reference to the current player system
	m_systemenv = m_playership->GetSpaceEnvironment();
}

void Player::UpdatePlayerActorState(void)
{
	// Retrieve position & orientation data from the player actor
	m_envposition = m_actor->GetEnvironmentPosition();
	m_envorientation = m_actor->GetEnvironmentOrientation();

	// We also want to keep a local copy of the player absolute pos & orient, so retrieve them now
	m_position = m_actor->GetPosition();
	m_orientation = m_actor->GetOrientation();

	// Retrieve data relating to the immediate player environment
	if (m_parentenv)
	{
		// Get a reference to the ship element this player is located in
		m_complexshipenvelementlocation = Game::PhysicalPositionToElementLocation(m_envposition);
		m_complexshipenvelement = m_parentenv->GetElement(m_complexshipenvelementlocation);

		// Also store a reference to the ultimate player system, i.e. the system this player's environment sits within
		m_systemenv = m_parentenv->GetSpaceEnvironment();
	}


	// If we are not moving we want to gradually remove any head-bob effect
	if (!m_ismoving && m_headbobposition > 0.0f)
	{
		m_headbobposition -= (m_headbobspeed * Game::TimeFactor);
		if (m_headbobposition < 0.0f) m_headbobposition = 0.0f;
	}
		
	// Recalculate the view offset matrix each frame
	RecalculatePlayerOffsetMatrix();
}

// Initialises the player for a new simulation cycle
void Player::BeginSimulationCycle(void)
{
	// Reset state flags
	m_ismoving = false;
}

// Accepts keyboard input and updates the player state accordingly
void Player::AcceptKeyboardInput(GameInputDevice *keyboard)
{
	// Read the key data from the keyboard controller
	BOOL *keys = keyboard->GetKeys();

	// Take different action depending on the current player state
	if (m_state == StateType::OnFoot)
	{
		// If we are on foot then use the keyboard input to update player actor state
		if (keys[DIK_W])			{ MovePlayerActor(Direction::Up, true); }
		if (keys[DIK_A])			{ MovePlayerActor(Direction::Left, true); }
		if (keys[DIK_S])			{ MovePlayerActor(Direction::Down, true); }
		if (keys[DIK_D])			{ MovePlayerActor(Direction::Right, true); }
		if (keys[DIK_SPACE])		{ ActorJump(); }
	}
	else if (m_state == StateType::ShipPilot && m_playership)
	{
		/* Otherwise if we are piloting a ship, use the keyboard input to adjust ship engines, heading etc. */
		
		// Ship thrust control
		if (keys[DIK_W])			{ m_playership->IncrementTargetThrustOfAllEngines(); }
		if (keys[DIK_S])			{ m_playership->DecrementTargetThrustOfAllEngines(); }
		if (keys[DIK_T])			{ m_playership->SetTargetSpeedPercentage(1.0f);			keyboard->LockKey(DIK_T); }
		if (keys[DIK_BACKSPACE])	{ m_playership->SetTargetSpeed(0.0f);					keyboard->LockKey(DIK_BACKSPACE); }
		if (keys[DIK_SPACE])		{ ToggleShipMouseControlMode();							keyboard->LockKey(DIK_SPACE); }
	}
}

// Toggles the player mouse control mode when piloting a ship
void Player::ToggleShipMouseControlMode(void)
{
	if (Game::MouseControlMode == MouseInputControlMode::MC_MOUSE_FLIGHT)
	{
		// We are switching from mouse flight to cockpit control
		SetShipMouseControlMode(MouseInputControlMode::MC_COCKPIT_CONTROL);

		// Send a one-time update of the ship mouse view with mouse position set to the origin.  This will 
		// prevent the ship continuing in the last active trajectory when it stops receiving new mouse input
		Game::Logic::Move::UpdateShipMovementViaMouseFlightData(m_playership, 0.0f, 0.0f);
	}
	else
	{
		// We are switching from cockpit control to mouse flight
		SetShipMouseControlMode(MouseInputControlMode::MC_MOUSE_FLIGHT);
	}
}

// Sets the player mouse control mode to the specified value
void Player::SetShipMouseControlMode(MouseInputControlMode mode)
{
	// If we are already in the desired control mode then make no further change
	if (Game::MouseControlMode == mode) return;

	// If we are currently in mouse-flight mode then zero out any current turn.  If autopilot is on it should overwrite this within a flight computer cycle
	if (Game::MouseControlMode == MouseInputControlMode::MC_MOUSE_FLIGHT)
		m_playership->OverrideTargetAngularVelocity(NULL_VECTOR);

	// Store the new mouse control mode
	Game::MouseControlMode = mode;
}

// Moves the player in the specified direction.  Only options are forward/back (normal walking) and left/right (strafing)
void Player::MovePlayerActor(Direction direction, bool run)
{
	// Record the fact that we are moving
	if (!m_actor) return;
	m_ismoving = true;

	// Move the actor being controlled by this player
	m_actor->Move(direction, run);

	// Calculate the 'head-bob' effect for this movement
	if (m_headbobascending) m_headbobposition += (m_headbobspeed * Game::TimeFactor);
	else					m_headbobposition -= (m_headbobspeed * Game::TimeFactor);

	// Invert direction of the head-bob if we have reached an inflection point
	if (m_headbobascending && m_headbobposition >= m_headbobmax)	m_headbobascending = false;
	else if (!m_headbobascending && m_headbobposition <= 0.0f)		m_headbobascending = true;

	// Clamp within the range of valid values
	m_headbobposition = max(0.0f, min(m_headbobmax, m_headbobposition));
}

// Rotates the on-foot player view by the specified amounts in the Y and X axes
void Player::RotatePlayerView(float y, float x)
{
	if (!m_actor) return;

	// Turn the actor by the specified angle about the y axis
	m_actor->Turn(y);

	// Update the X rotation (pitch); clamp values to ensure that we are not pitching outside the valid player look range.
	// This will also take care of the 0-2PI range so long as the clamp range is set sensibly (i.e. within 0-2PI)
	m_viewpitch += (x * Game::TimeFactor);
	if (m_viewpitch < Game::C_PLAYER_PITCH_MIN)		m_viewpitch = Game::C_PLAYER_PITCH_MIN;
	if (m_viewpitch > Game::C_PLAYER_PITCH_MAX)		m_viewpitch = Game::C_PLAYER_PITCH_MAX;

	// Update the view offset orientation (i.e. not the actor) with the PITCH component.  We don't want the actor themself
	// to be pitching about the x axis, so any pitch is included solely within the view offset quaternion
	m_vieworientationoffset = XMQuaternionRotationRollPitchYaw(m_viewpitch, 0.0f, 0.0f);
}

// Attempts to perform a jump as the player actor
void Player::ActorJump(void)
{
	// Parameter check
	if (!m_actor) return;

	// Pass control to the player actor
	m_actor->Jump();
}

// Moves the player back into their ship, whether simple or complex
void Player::ReturnToShip(void)
{
	// If the player doesn't have a ship then we can't do anything
	if (!this->m_playership) return;

	// Take different action depending on the player ship type
	if (this->m_playership->GetShipClass() == Ships::Class::Simple)
	{
		// Simply set the player to be piloting their simple ship
		m_state = StateType::ShipPilot;
		
		// Inactivate the player actor, and activate the player ship
		//m_actor->SetSimulationState(iObject::ObjectSimulationState::NoSimulation);
		//m_playership->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);

		// Update the player state to take account of its new environment
		UpdatePlayerState();
	}
	else if (this->m_playership->GetShipClass() == Ships::Class::Complex)
	{
		// Move the player into this complex ship
		EnterEnvironment((ComplexShip*)m_playership);
	}
}

// Set the player environment to the specified system.  Force the player back into their own ship, if they have one
void Player::EnterEnvironment(SpaceSystem *system)
{
	// Make sure a system is specified, and that the player has a ship; if not, they cannot be placed into the space environment
	if (!m_playership || !system) return;

	// Store the system environment that the player is being placed into
	m_systemenv = system;

	// Update the player state accordingly
	m_state = StateType::ShipPilot;

	// Remove the designation of the player actor as a simulation hub, and instead set the player ship to be a hub
	m_actor->RemoveSimulationHub();
	m_playership->SetAsSimulationHub();

	// If the player ship is not currently located in this system then move it now
	if (m_playership->GetSpaceEnvironment() != m_systemenv)
	{
		m_playership->MoveIntoSpaceEnvironment(m_systemenv, NULL_VECTOR);
	}
}

// Set the player environment to the specified complex ship.  Force state to on-foot if not already
void Player::EnterEnvironment(ComplexShip *complexship)
{
	// Get a reference to the ship and make sure we have required details
	if (!complexship || !m_actor) return;

	// Get a reference to the first corridor tile within this ship (NOTE: TODO: we can later locate the bridge/entry doors and move the player there)
	ComplexShipTile *tile = complexship->GetFirstTileOfType(D::TileClass::Corridor);
	if (!tile) return;

	// Set the player to be on-foot within the complex ship
	m_state = StateType::OnFoot;
	m_parentenv = complexship;

	// Remove the designation of the player actor as a simulation hub, and instead set the player ship to be a hub
	m_playership->RemoveSimulationHub();
	m_actor->SetAsSimulationHub();

	// Determine an offset to apply that will place the player in the centre of a specific element
	float centreoffset = Game::ElementLocationToPhysicalPosition(1) * 0.5f;
	XMVECTOR offset = XMVectorSet(centreoffset, 0.0f, centreoffset, 0.0f);			// TODO: Set at just above floor height in future

	// Set the actor's parent environment to this ship
	if (m_actor) m_actor->MoveIntoEnvironment(complexship);

	// Move the player to be within this identified tile within the ship
	// Pos = (ELocation + (ESize * 0.5))
	m_actor->SetEnvironmentPositionAndOrientation(Game::ElementPartialLocationToPhysicalPosition(
		XMVectorAdd(VectorFromIntVector3(tile->GetElementLocation()), 
			XMVectorMultiply(VectorFromIntVector3(tile->GetElementSize()), HALF_VECTOR))), ID_QUATERNION);		

	// Update the player state to take account of its new environment
	UpdatePlayerState();
}


// Sets the current player ship.  Also updates the current player state to account for this change
void Player::SetPlayerShip(Ship *ship)
{
	// If we previously controlled a ship, remove its designation as a simulation hub
	if (m_playership) m_playership->RemoveSimulationHub();

	// Store the new player ship
	m_playership = ship;

	// Set the new ship as a simulation hub, if we are currently in player ship mode (vs actor mode)
	if (m_state == Player::StateType::ShipPilot) m_playership->SetAsSimulationHub();

	// Update the player state based on this new ship assignment
	UpdatePlayerState();
}


// Sets the current player actor.  Also updates this actor based on the player state
void Player::SetActor(Actor *actor)
{
	// Parameter check
	if (!actor || !actor->GetBase()) return;

	// If we previously controlled an actor, remove its designation as a simulation hub
	if (m_actor) m_actor->RemoveSimulationHub();

	// Store the new player actor
	m_actor = actor;

	// Set the new actor as a simulation hub, if we are currently in player actor mode (vs as a ship pilot)
	if (m_state == Player::StateType::OnFoot) m_actor->SetAsSimulationHub();

	// Retrieve data from the actor that will influence how the player operates
	m_runspeed = actor->Attributes[ActorAttr::A_RunSpeed].Value;
	m_walkspeed = actor->Attributes[ActorAttr::A_WalkSpeed].Value;
	m_strafespeed = actor->Attributes[ActorAttr::A_StrafeSpeed].Value;
	m_viewpositionoffset = actor->GetBase()->GetViewOffset();
	m_headbobmax = actor->GetBase()->GetHeadBobAmount();
	m_headbobspeed = actor->GetBase()->GetHeadBobSpeed();

	// Update the actor state based on this new assignment to the player
	UpdatePlayerState();
}

// Sets the view position offset and recalculates the view offset matrix
void Player::SetViewPositionOffset(const FXMVECTOR pos)
{
	// Store the new position offset
	m_viewpositionoffset = pos;

	// Recalculate the view offset matrix based on this new position
	RecalculatePlayerOffsetMatrix();
}

// Sets the view orientation offset and recalculates the view offset matrix
void Player::SetViewOrientationOffset(const FXMVECTOR orient)
{
	// Store the new orientation offset
	m_vieworientationoffset = orient;

	// Recalculate the view offset matrix based on this new orientation
	RecalculatePlayerOffsetMatrix();
}

// Sets both the view position and orientation offsets, then recalculates the view offset matrix
void Player::SetViewPositionAndOrientationOffset(const FXMVECTOR pos, const FXMVECTOR orient)
{
	// Store the new position offset
	m_viewpositionoffset = pos; 

	// Store the new orientation offset
	m_vieworientationoffset = orient;

	// Recalculate the view offset matrix based on this new position
	RecalculatePlayerOffsetMatrix();
}

// Recalculates the view offset matrix for the player character
void Player::RecalculatePlayerOffsetMatrix(void)
{
	// Create a new rotation matrix based upon the view orientation offset quaternion (pitch about the x axis)
	m_viewoffsetmatrix = XMMatrixRotationQuaternion(m_vieworientationoffset);

	// Directly set the translation row of this matrix for efficiency
	m_viewoffsetmatrix.r[3] = 
		XMVectorSetW(											// 3. Ensure matrix diagonal is always 1.0
		XMVectorAdd(m_viewpositionoffset,						// 2. The translation row should incorporate the regular view offset translation...
		XMVectorSetY(NULL_VECTOR, m_headbobposition)), 1.0f);	// 1. ...and an additional Y component to account for 'head bob'
}

// Change the current player position.  Takes different action depending on whether the player is on foot or in a ship
void Player::SetPosition(const FXMVECTOR pos)
{
	// Take different action depending on the current player environment
	if (m_state == Player::StateType::ShipPilot && m_playership) 
	{
		// If the player is currently piloting a ship
		m_playership->SetPosition(pos);
	}
	else if (m_state == Player::StateType::OnFoot && m_actor)
	{
		// If the player is currently on foot
		m_actor->SetEnvironmentPosition(pos);
	}

	// Update the player state following this change
	UpdatePlayerState();
}

// Change the current player position.  Takes different action depending on whether the player is on foot or in a ship
void Player::SetOrientation(const FXMVECTOR orient)
{
	// Take different action depending on the current player environment
	if (m_state == Player::StateType::ShipPilot && m_playership) 
	{
		// If the player is currently piloting a ship
		m_playership->SetOrientation(orient);
	}
	else if (m_state == Player::StateType::OnFoot && m_actor)
	{
		// If the player is currently on foot
		m_actor->SetEnvironmentOrientation(orient);
	}

	// Update the player state following this change
	UpdatePlayerState();
}

// Method to force-set the player environment , without the player entity needing to be there.  Used for e.g.
// rendering scenes within the null system.  Will revert to normal once released.  Game should generally be paused first.
void Player::OverridePlayerEnvironment(SpaceSystem *system, Ship *playership, const FXMVECTOR position, const FXMVECTOR orientation)
{
	// Parameter check
	if (system)
	{
		// Store the prior player state, for when we need to restore upon releasing the override
		m_overridepriorstate = m_state;

		// Set the override flag
		m_envoverride = PlayerEnvironmentOverrideType::SpaceEnvironmentOverride;
	
		// Store the relevant fields
		m_overridesystem = system;
		m_overrideposition = position;
		m_overrideorientation = orientation;

		// Also switch to the (optional) payer ship, if specified.  If not, the player remains in their current ship, 
		// even if the system is changed to be inconsistent with that ship
		if (playership)
		{
			m_overridepriorship = m_playership;
			m_overrideship = playership;
		}
		else
		{
			m_overrideship = m_overridepriorship = NULL;
		}
	}
}

// Method to release any override of the player environment, and return normal view & control to the player
void Player::ReleasePlayerEnvironmentOverride(void)
{
	// Make sure the state is currently overriden
	if (m_envoverride != Player::PlayerEnvironmentOverrideType::NoOverride)
	{
		// Reset the pre-override player state
		m_state = m_overridepriorstate;

		// Reset all override fields; correct values will be restored in the next cycle upon calling "UpdatePlayerState"
		m_envoverride = PlayerEnvironmentOverrideType::NoOverride;
		m_overridesystem = NULL;
		m_overrideposition = NULL_VECTOR;
		m_overrideorientation = ID_QUATERNION;
		m_overridepriorstate = Player::StateType::UnknownState;
		
		if (m_overridepriorship)
		{
			m_playership = m_overridepriorship;
			m_overridepriorship = NULL;
			m_overrideship = NULL;
		}
	}
}

// Method which performs the override of the player environment
void Player::ExecuteOverrideOfPlayerEnvironment(void)
{
	// Override player fields with the stored values
	m_state = Player::StateType::ShipPilot;
	m_systemenv = m_overridesystem;
	m_position = m_overrideposition;
	m_orientation = m_overrideorientation;

	// Optional override of player ship
	if (m_overrideship) m_playership = m_overrideship;
}

// Default destructor
Player::~Player(void)
{
}