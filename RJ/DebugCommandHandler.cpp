#include "Utility.h"
#include "GameVarsExtern.h"
#include "GameDataExtern.h"
#include "CoreEngine.h"
#include "GameObjects.h"
#include "ObjectReference.h"
#include "Player.h"
#include "GameUniverse.h"
#include "SpaceSystem.h"
#include "Order_MoveToPosition.h"
#include "RJMain.h"
#include "UserInterface.h"

// Debug command handler needs to include the full object hierarchy to support per-object command handling
#include "Actor.h"
#include "CapitalShipPerimeterBeacon.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "LightSource.h"
#include "SpaceProjectile.h"
#include "Ship.h"
#include "SimpleShip.h"
#include "SpaceEmitter.h"



#include "DebugCommandHandler.h"


// Default constructor
DebugCommandHandler::DebugCommandHandler(void)
{

}

// Virtual inherited method to accept a command from the console
bool DebugCommandHandler::ProcessConsoleCommand(GameConsoleCommand & command)
{
	/* Execute debug command on a specific object */
	if (command.InputCommand == "obj")
	{
		iObject *object = NULL;
		if (command.Parameter(0) == "") {
			command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
				concat("Object not specified").str()); return true;
		}
		if (command.Parameter(1) == "") {
			command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectCommandDoesNotSpecifyFunction,
				concat("Object debug function not specified").str()); return true;
		}

		// First attempt to get the object using this as an instance code
		object = Game::GetObjectByInstanceCode(command.Parameter(0));
		if (!object) {

			// Also try to use this as an ID, if numeric and if it can be found
			int id = atoi(command.Parameter(0).c_str());
			object = Game::GetObjectByID(id);
			if (!object) {
				command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectDoesNotExist,
					concat("Object \"")(command.Parameter(0))("\" does not exist").str()); return true;
			}
		}

		ExecuteDebugCommandOnObject(object, command);
		if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)
		{
			command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectCannotAcceptConsoleComand,
				concat("Object \"")(command.Parameter(0))("\" cannot execute command \"")(command.Parameter(1))("\"").str());
		}
		
		// We have handled this command, whether or not it was successful
		return true;
	}

	/* Change player environment to a system */
	else if (command.InputCommand == "enter_system_env")
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
	
		object = Game::GetObjectByInstanceCode(command.Parameter(0));
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
		
		SimpleShip *ship = D::SimpleShips.Get(command.Parameter(0));
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

	/* Print a log of all registered game objects to the debug output */
	else if (command.InputCommand == "print_objects")
	{
		DebugPrintAllGameObjects();
		command.SetSuccessOutput("Printed all registered game objects to debug output");
		return true;
	}

	/* Enable the frame profiler for the next frame */
	else if (command.InputCommand == "profile_frame")
	{
		// We need to deactivate the console before profiling since it will otherwise prevent most rendering from being performed
		D::UI->DeactivateConsole();
		FrameProfiler::ProfileNextFrame();
		command.SetSuccessOutput("Frame profiling completed (console was closed to enable full profiling)");
		return true;
	}

	/* Adjust various oxygen simulation parameters */
	else if (command.InputCommand == "set_oxygen_falloff")
	{
		Oxygen::BASE_OXYGEN_FALLOFF = command.ParameterAsFloat(0);
		command.SetSuccessOutput(concat("Set oxygen falloff /sec to ")(Oxygen::BASE_OXYGEN_FALLOFF).str().c_str());
		return true;
	}
	else if (command.InputCommand == "set_oxygen_transfer_limit")	
	{
		Oxygen::BASE_TRANSMISSION_LIMIT = command.ParameterAsFloat(0);
		command.SetSuccessOutput(concat("Set oxygen transfer limit /sec to ")(Oxygen::BASE_TRANSMISSION_LIMIT).str().c_str());
		return true;
	}
	else if (command.InputCommand == "set_oxygen_simfreq_full")
	{
		Oxygen::OXYGEN_UPDATE_INTERVAL_FULL_SIMULATION = (unsigned int)command.ParameterAsInt(0);
		command.SetSuccessOutput(concat("Set oxygen simulation interval (full sim) to ")(Oxygen::OXYGEN_UPDATE_INTERVAL_FULL_SIMULATION)("ms").str().c_str());
		return true;
	}

	/* Quit the application */
	else if (command.InputCommand == "exit")
	{
		Game::Application.Quit();
	}

	// We did not recognise the command
	return false;
}


// Executes a debug command on the specified object.  Will locate the appropriate subclass to begin
// passing the command down from.  Ugly, but avoids adding another function to the iObject vtable
void DebugCommandHandler::ExecuteDebugCommandOnObject(iObject *object, GameConsoleCommand & command)
{
	// Parameter check
	if (!object) return;

	// Manually determine the subclass that we want to use (avoiding the vptr)
	switch (object->GetObjectType())
	{
		case iObject::ObjectType::ActorObject:							((Actor*)object)->ProcessDebugCommand(command);							return;
		case iObject::ObjectType::CapitalShipPerimeterBeaconObject:		((CapitalShipPerimeterBeacon*)object)->ProcessDebugCommand(command);	return;
		case iObject::ObjectType::ComplexShipObject:					((ComplexShip*)object)->ProcessDebugCommand(command);					return;
		case iObject::ObjectType::ComplexShipSectionObject:				((ComplexShipSection*)object)->ProcessDebugCommand(command);			return;
		case iObject::ObjectType::LightSourceObject:					((LightSource*)object)->ProcessDebugCommand(command);					return;
		case iObject::ObjectType::ProjectileObject:						((SpaceProjectile*)object)->ProcessDebugCommand(command);				return;
		case iObject::ObjectType::ShipObject:							((Ship*)object)->ProcessDebugCommand(command);							return;
		case iObject::ObjectType::SimpleShipObject:						((SimpleShip*)object)->ProcessDebugCommand(command);					return;
		case iObject::ObjectType::SpaceEmitterObject:					((SpaceEmitter*)object)->ProcessDebugCommand(command);					return;
	}

	// This object type is not currently supported; return an error
	command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ObjectTypeCannotAcceptDebugCommands,
		concat("Object of type \"")(object->GetObjectType())("\" cannot accept debug commands").str());
}

// Debug spawn a set of ships near the player
void DebugCommandHandler::SpawnDebugShips(SimpleShip *template_ship, int count)
{
	// Parameter checks
	if (!template_ship || count <= 0 || !Game::CurrentPlayer || !Game::CurrentPlayer->GetPlayerShip()) return;

	// Determine the best spawn point
	SpaceSystem *spawnsystem = NULL;
	XMVECTOR spawnpos = XMVectorSetZ(NULL_VECTOR, max(template_ship->GetCollisionSphereRadius()*1.2f, 25.0f));			// Player-relative spawn point for ships
	XMVECTOR spawninterval = XMVectorSetX(NULL_VECTOR, max(template_ship->GetCollisionSphereRadius()*1.1f, 30.0f));		// Spacing between spawned ships
	
	if (Game::CurrentPlayer->GetState() == Player::StateType::ShipPilot && Game::CurrentPlayer->GetPlayerShip() != NULL)
	{
		spawnsystem = Game::CurrentPlayer->GetPlayerShip()->GetSpaceEnvironment();
		if (!spawnsystem) spawnsystem = &Game::Universe->GetCurrentSystem();
		spawnpos = XMVector3TransformCoord(spawnpos, Game::CurrentPlayer->GetPlayerShip()->GetWorldMatrix());
		spawninterval = XMVector3TransformCoord(spawninterval, Game::CurrentPlayer->GetPlayerShip()->GetWorldMatrix());
		spawninterval = XMVectorSubtract(spawninterval, Game::CurrentPlayer->GetPlayerShip()->GetPosition());
	}
	else 
	{
		spawnsystem = &Game::Universe->GetCurrentSystem();
		spawnpos = XMVectorAdd(spawnpos, Game::Engine->GetCamera()->GetPosition());
	}

	// Spawn ships either side of the spawn point
	for (int i = 0; i < count; i++)
	{
		Ship *s = SimpleShip::Create(template_ship); if (!s) continue;
		SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip((SimpleShip*)s);
		s->SetName("DebugSpawnedShip");										// Will be used to remove ship later
		s->MoveIntoSpaceEnvironment(spawnsystem);
		s->SetPosition(spawnpos + ((i - count*0.5f) * spawninterval));
		s->SetOrientation(ID_QUATERNION);
		
		// Assign a default order for now
		s->AssignNewOrder(new Order_MoveToPosition(
			XMVectorSet(frand_lh(-1000.0f, 1000.0f), frand_lh(-400.0f, 400.0f), frand_lh(-1000.0f, 1000.0f), 0.0f), 25.0f));
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
			++it;
		}
		else
		{
			++it;									// If this object was not debug-spawned
		}
	}
}


// Prints a log of all registered game objects to the debug output stream
void DebugCommandHandler::DebugPrintAllGameObjects(void)
{
	// Generate and output a header row
	std::string header1 = "ID    Act.  Type   Name   InstanceCode   Code   RC";
	std::string header2 = "--------------------------------------------------";
	OutputDebugString(std::string("\n\n" + header1 + "\n" + header2 + "\n").c_str());

	// Store a temporary vector of pointers to register entries, so we can sort them locally before printing
	std::vector<Game::ObjectRegisterEntry*> objects;
	Game::ObjectRegister::iterator it_end = Game::Objects.end();
	for (Game::ObjectRegister::iterator it = Game::Objects.begin(); it != it_end; ++it)
	{
		Game::ObjectRegisterEntry * entry = &(it->second);
		std::vector<Game::ObjectRegisterEntry*>::iterator insert_pt = std::upper_bound(objects.begin(), objects.end(), entry->ID,
			[](Game::ID_TYPE const& id, Game::ObjectRegisterEntry const* obj) { return (id < obj->ID); });
		objects.insert(insert_pt, entry);
	}

	// Iterate through each sorted object register entry in turn
	iObject *obj;
	std::vector<Game::ObjectRegisterEntry*>::iterator it2_end = objects.end();
	for (std::vector<Game::ObjectRegisterEntry*>::iterator it2 = objects.begin(); it2 != it2_end; ++it2)
	{
		// The first part of the line can be constructed based on the entry only, whether or not the object exists
		Game::ID_TYPE id = (*it2)->ID;
		std::string s = concat(id)((id >= 0 && id < 10 ? "     " : (id >= 10 && id < 100 ? "    " : (id >= 100 && id < 1000 ? "   " : "  ")))).str();	// ID & padding
		s += ((*it2)->Active ? " Yes  " : " No   ");

		// Now print the object-related information
		obj = (*it2)->Object;
		if (obj)
		{
			s += (iObject::TranslateObjectTypeToString(obj->GetObjectType()) + "  \"" + obj->GetName() + "\"  \"" + obj->GetInstanceCode()
				+ "\"  \"" + obj->GetCode() + "\"  ");
		}
		else
		{
			s += ("(Null)  (Null)  (Null)  (Null)  ");
		}

		// Final entry-related data
		s = concat(s)((*it2)->RefCount).str();

		// Print this line to the debug output
		OutputDebugString((s + "\n").c_str());
	}
}

// Default destructor
DebugCommandHandler::~DebugCommandHandler(void)
{

}