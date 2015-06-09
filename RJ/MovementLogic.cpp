#include <vector>
#include "DX11_Core.h"
#include "GameVarsExtern.h"
#include "Player.h"
#include "Ship.h"
#include "Engine.h"
#include "iSpaceObject.h"
#include "Utility.h"

#include "Actor.h" // DBG

#include "MovementLogic.h"
using namespace std;

void Game::Logic::BeginSimulationCycle(void)
{
	// Set the update flag of all space objects to false for this cycle
	Game::ObjectRegister::iterator it_end = Game::Objects.end();
	for (Game::ObjectRegister::iterator it = Game::Objects.begin(); it != it_end; ++it)
	{
		if (it->second) ((iObject*)(it->second))->ResetSimulationFlags();
	}
}

void Game::Logic::SimulateAllObjects(void)
{
	iObject *obj;
	
	// Process the set of objects in scope for simulation (TODO: in future, this should be the locally-relevant subset) 
	Game::ObjectRegister::iterator it_end = Game::Objects.end();
	for (Game::ObjectRegister::iterator it = Game::Objects.begin(); it != it_end; ++it)
	{
		// Get a handle to this object and make sure it is valid
		obj = it->second; if (!obj) continue;

		// Handle any change to the object simulation state since last cycle
		if (obj->SimulationStateChangePending()) obj->SimulationStateChanged();

		// Only simulate the object if it has not already been simulated
		if (obj->Simulated() == false)
		{
			// Simulate the object, and set the flag once complete
			obj->SimulateObject();
			obj->SetSimulatedFlag(true);

			// Update the position of any child objects, if we have any (checking count!=0 here, instead of in function, avoids unnecessary function calls)
			if (obj->HasChildAttachments()) obj->UpdatePositionOfChildObjects();
		}

		// If the object position changed during this frame then recalculate any derived data (e.g. world matrices)
		if (obj->SpatialDataChanged())
		{
			// Perform base object updates required when the object moves, e.g. ensuring quaternions are normalised
			obj->RenormaliseSpatialData();

			/* if (obj->IsPostSimulationUpdateRequired()) */		// TODO: Add this back if the flag becomes more than just "return (m_spatialdatachanged)"
			obj->PerformPostSimulationUpdate();
		}

		// Revert the 'currently visible' flag, which will be updated by the core engine ready for next frame
		obj->RemoveCurrentVisibilityFlag();
	}

	// Process any pending object register/deregister requests
	iObject::UpdateGlobalObjectCollection();
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






