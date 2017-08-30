#include <vector>
#include "DX11_Core.h"
#include "GameVarsExtern.h"
#include "GameObjects.h"
#include "Player.h"
#include "Ship.h"
#include "Engine.h"
#include "iSpaceObject.h"
#include "Utility.h"
#include "Actor.h" // DBG
#include "MovementLogic.h"

void Game::Logic::BeginSimulationCycle(void)
{
	// Set the update flag of all space objects to false for this cycle
	Game::ObjectRegister::iterator it_end = Game::Objects.end();
	for (Game::ObjectRegister::iterator it = Game::Objects.begin(); it != it_end; ++it)
	{
		if (it->second.Active && it->second.Object) ((iObject*)(it->second.Object))->ResetSimulationFlags();
	}
}

void Game::Logic::SimulateAllObjects(void)
{
	iObject *obj; 
	
	// Lock the central object registers while iterating through the full collection; any collection
	// changes will then be held and applied at the end of the frame
	Game::LockObjectRegisters();

	// Process the set of objects in scope for simulation (TODO: in future, this should be the locally-relevant subset) 
	Game::ObjectRegister::iterator it_end = Game::Objects.end();
	for (Game::ObjectRegister::iterator it = Game::Objects.begin(); it != it_end; ++it)
	{
		// Get a handle to this object and make sure it is valid
		obj = it->second.Object; if (!obj || !it->second.Active) continue;
		
		// Handle any change to the object simulation state since last cycle
		if (obj->SimulationStateChangePending()) obj->SimulationStateChanged();

		// Simulate this object
		obj->Simulate();

		// Revert the 'currently visible' flag, which will be updated by the core engine ready for next frame
		obj->RemoveCurrentVisibilityFlag();
	}

	// Unlock the central object registers following processing of the full object collection
	Game::UnlockObjectRegisters();

	// Process any pending object register/deregister requests
	Game::UpdateGlobalObjectCollection();

	// Reset the null object reference every frame so that its reference count never goes out of bounds
	Game::ResetNullObjectReference();
}

// Updates the current ship vector based upon mouse input data
void Game::Logic::Move::UpdateShipMovementViaMouseFlightData(Ship *ship, float x_mv, float y_mv)
{
	// Initiate a turn of the ship based on the mouse input data
	if (ship) ship->TurnShip(x_mv, y_mv, true);
}

// Updates the current on-foot player view based on mouse input
void Game::Logic::Move::UpdatePlayerViewViaMouseData(float x_mv, float y_mv)
{
	// Rotate the player view by the specified percentages of the player turn rate
	Game::CurrentPlayer->RotatePlayerView(	x_mv * Game::C_PLAYER_MOUSE_TURN_MODIFIER_YAW,
											y_mv * Game::C_PLAYER_MOUSE_TURN_MODIFIER_PITCH );
}






