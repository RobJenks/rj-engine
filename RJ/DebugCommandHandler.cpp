#include "Utility.h"
#include "GameVarsExtern.h"
#include "GameDataExtern.h"
#include "Player.h"
#include "Ship.h"
#include "SimpleShip.h"
#include "GameUniverse.h"
#include "SpaceSystem.h"
#include "Order_MoveToPosition.h"

#include "DebugCommandHandler.h"


// Default constructor
DebugCommandHandler::DebugCommandHandler(void)
{

}

// Virtual inherited method to accept a command from the console
bool DebugCommandHandler::ProcessConsoleCommand(GameConsoleCommand & command)
{
	/* Change player environment to a system */
	if (command.InputCommand == "enter_system_env")
	{
		SpaceSystem *system = Game::Universe->GetSystem(command.Parameter(0));
		if (!system) { command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::SystemDoesNotExist,
				concat("System \"")(command.Parameter(0))("\" does not exist").str()); return true; }
		Game::CurrentPlayer->EnterEnvironment(system);
		command.SetSuccessOutput(concat("Changing player environment to system \"")(command.Parameter(0))("\"").str());
		return true;
	}

	/* Change player environment to a complex ship environment */
	else if (command.InputCommand == "enter_ship_env")
	{
		iObject *object = NULL;
		if (command.Parameter(0) == "") { command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
								concat("Environment not specified").str()); return true; }
	
		object = Game::FindObjectInGlobalRegister(command.Parameter(0));
		if (!object) { command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
				concat("Object \"")(command.Parameter(0))("\" does not exist").str()); return true; }

		if (!object->IsEnvironment() || object->GetObjectType() != iObject::ObjectType::ComplexShipObject) 
			{ command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectIsNotEnvironment,
				"Cannot enter target; object is not an environment"); return true; }

		Game::CurrentPlayer->EnterEnvironment((ComplexShip*)object);
		command.SetSuccessOutput(concat("Moving player to environment \"")(command.Parameter(0))("\"").str());
		return true;
	}

	/* Spawn the specified ships at the player location */
	else if (command.InputCommand == "spawn_ships")
	{
		std::string scount = command.Parameter(1);
		if (command.Parameter(0) == "" || scount == "") 
			{ command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::InvalidParameters, 
				"Invalid parameters; format is \"spawn_ships <template> <count>\""); return true; }
		
		SimpleShip *ship = D::GetSimpleShip(command.Parameter(0));
		if (!ship) { command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
				concat("Cannot locate template ship with code \"")(command.Parameter(0))("\"").str()); return true; }

		int count = atoi(scount.c_str());
		SpawnDebugShips(ship, count);

		command.SetSuccessOutput(concat("Spawning ")(count)(" ships of class \"")(command.Parameter(0))("\"").str());
		return true;
	}

	/* Remove any ships that have been created via spawn_ships */
	else if (command.InputCommand == "remove_spawned_ships")
	{
		ClearAllSpawnedShips();
		command.SetSuccessOutput("Removing all debug-spawned ships"); 
		return true;
	}

	/* Activate or deactivate  the debug camera */
	else if (command.InputCommand == "debug_camera")
	{
		bool b = (command.Parameter(0) == "1");
		if (b)
		{
			Game::Engine->GetCamera()->ActivateDebugCamera();
			command.SetSuccessOutput("Activating debug camera");
		}
		else
		{
			Game::Engine->GetCamera()->DeactivateDebugCamera();
			command.SetSuccessOutput("Deactivating debug camera");
		}
		return true;
	}

	/* Force a release of the camera, overriding any fixed/path/debug camera currently active.  Could cause issues */
	else if (command.InputCommand == "force_release_camera")
	{
		Game::Engine->GetCamera()->ReleaseCamera();
		command.SetSuccessOutput("Releasing camera");
		return true;
	}

	// We did not recognise the command
	return false;
}

// Debug spawn a set of ships near the player
void DebugCommandHandler::SpawnDebugShips(SimpleShip *template_ship, int count)
{
	// Parameter checks
	if (!template_ship || count <= 0 || !Game::CurrentPlayer || !Game::CurrentPlayer->GetPlayerShip()) return;

	// Determine the best spawn point
	XMVECTOR spawnpos = XMVectorSetZ(NULL_VECTOR, max(template_ship->GetCollisionSphereRadius()*1.2f, 25.0f));			// Player-relative spawn point for ships
	XMVECTOR spawninterval = XMVectorSetX(NULL_VECTOR, max(template_ship->GetCollisionSphereRadius()*1.1f, 30.0f));	// Spacing between spawned ships
	
	if (Game::CurrentPlayer->GetState() == Player::StateType::ShipPilot)
	{
		spawnpos = XMVector3TransformCoord(spawnpos, Game::CurrentPlayer->GetPlayerShip()->GetWorldMatrix());
		spawninterval = XMVector3TransformCoord(spawninterval, Game::CurrentPlayer->GetPlayerShip()->GetWorldMatrix());
		spawninterval = XMVectorSubtract(spawninterval, Game::CurrentPlayer->GetPlayerShip()->GetPosition());
	}
	else {
		spawnpos = XMVectorAdd(spawnpos, Game::CurrentPlayer->GetPosition());
	}

	// Spawn ships either side of the spawn point
	for (int i = 0; i < count; i++)
	{
		Ship *s = SimpleShip::Create(template_ship); if (!s) continue;
		SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip((SimpleShip*)s);
		s->SetName("DebugSpawnedShip");										// Will be used to remove ship later
		s->MoveIntoSpaceEnvironment(Game::CurrentPlayer->GetSystem(), spawnpos + ((i - count*0.5f) * spawninterval));
		s->SetOrientation(ID_QUATERNION);
		
		// Assign a default order for now
		Order_MoveToPosition *o = new Order_MoveToPosition(
			XMVectorSet(frand_lh(-1000.0f, 1000.0f), frand_lh(-400.0f, 400.0f), frand_lh(-1000.0f, 1000.0f), 0.0f), 25.0f);
		s->AssignNewOrder(o);
	}
}

void DebugCommandHandler::ClearAllSpawnedShips(void)
{
	// Iterate through the active ships collection and remove any which have their name set to the debug string
	Game::ObjectRegister::iterator it = Game::Objects.begin();
	Game::ObjectRegister::iterator it_end = Game::Objects.end();
	while (it != it_end)
	{
		// If this ship was debug-spawned then remove it now
		if (it->second.Active && it->second.Object && it->second.Object->GetName() == "DebugSpawnedShip")
		{
			it->second.Object->Shutdown();			// Call virtual shutdown method on the object
			delete (it++)->second.Object;			// Delete the object, using post-increment to avoid invalidating the pointer
		}
		else
		{
			++it;									// If this object was not debug-spawned
		}
	}
	
}

// Default destructor
DebugCommandHandler::~DebugCommandHandler(void)
{

}