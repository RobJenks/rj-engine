#include <string>
#include <unordered_map>
#include "DX11_Core.h"
#include "Logging.h"
#include "SpaceSystem.h"

#include "GameUniverse.h"


// Default constructor
GameUniverse::GameUniverse(void)
{
	// Initialise the null system object
	m_nullsystem = new SpaceSystem();
	m_nullsystem->SetCode("NULL");
	m_nullsystem->SetName("(Null system)");
	m_nullsystem->SetSize(XMVectorReplicate(1000.0f));

	// Initialise the "current system" pointer to the null system
	m_currentsystem = m_nullsystem;
}

Result GameUniverse::InitialiseUniverse(void)
{
	// No initialisation to be performed; simply return success
	return ErrorCodes::NoError;
}

Result GameUniverse::ProcessLoadedSystems(ID3D11Device *device)
{
	Result overallresult = ErrorCodes::NoError;

	// Initialise each loaded system in turn
	SystemRegister::const_iterator it_end = Systems.end();
	for (SystemRegister::const_iterator it = Systems.begin(); it != it_end; ++it) 
	{
		if (it->second) 
		{
			// Initialise the system
			Result res = it->second->InitialiseSystem(device);
			if (res != ErrorCodes::NoError)
			{
				overallresult = res;
				Game::Log << LOG_ERROR << "Error during post-processing of system \"" << it->second->GetName() << "\"\n";
			}
		}
	}
	
	// Return success if no errors were encounterd loading systems.  TODO: could make this more fault tolerant
	// by not returning error on the first failure, and proceeding with the remaining systems regardless
	return overallresult;
}

void GameUniverse::AddSystem(SpaceSystem *system)
{
	// Make sure the system has a valid code, otherwise we will not add it
	if (!system || system->GetCode() == NullString) return;

	// Add to the systems collection
	Systems[system->GetCode()] = system;
}

SpaceSystem	*GameUniverse::GetSystem(const std::string & code)
{
	// Attempt to locate this system in the collection
	if (code == NullString) return NULL;
	GameUniverse::SystemRegister::const_iterator it = Systems.find(code);
	
	// Return the system, if we could find it
	return (it == Systems.end() ? NULL : it->second);
}

// Sets the current active system; will be repointed to the null system if "sys" is not valid
void GameUniverse::SetCurrentSystem(SpaceSystem * sys)
{
	// Reject any null parameter, and instead map it to the non-null "null system"
	SpaceSystem *new_system = (sys ? sys : m_nullsystem);

	// If this is already the current system we can quit immediately
	if (new_system == m_currentsystem) return;

	// Otherwise we are changing systems; store the new system...
	SpaceSystem *old_system = m_currentsystem;
	m_currentsystem = new_system;

	// ...and raise the relevant game events
	CurrentSystemChanged(*old_system, *m_currentsystem);
}

// Event raised when the current system changes
void GameUniverse::CurrentSystemChanged(SpaceSystem & old_system, SpaceSystem & new_system)
{
	OutputDebugString(concat("=== Current system changed from  ")(old_system.DebugString())(" to ")(new_system.DebugString())("\n").str().c_str());
}

// Termination function: Clears all universe and system data
void GameUniverse::TerminateUniverse()
{
	// Delete each system in turn
	SystemRegister::const_iterator it_end = Systems.end();
	for (SystemRegister::const_iterator it = Systems.begin(); it != it_end; ++it) {
		if (it->second) 
		{
			// Delete the object and deallocate memory
			it->second->TerminateSystem();
			delete (it->second);
		}
	}

	// Empty the register now it is full of null/invalid pointers
	Systems.clear();
}

// Default destructor
GameUniverse::~GameUniverse(void)
{
	// Clear the current system - this can only ever be performed at universe shutdown, since it is otherwise guaranteed
	// to be non-null during normal operation
	m_currentsystem = NULL;

	// Shutdown and deallocate the null system
	m_nullsystem->TerminateSystem();
	SafeDelete(m_nullsystem);
}



// Virtual inherited method to accept a command from the console
bool GameUniverse::ProcessConsoleCommand(GameConsoleCommand & command)
{
	if (command.InputCommand == "GetSystem")
	{
		if (command.InputParameters.size() < 1)
		{
			command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::RequiredCommandParametersNotProvided,
				"System code not specified");
		}
		else
		{
			SpaceSystem *sys = GetSystem(command.Parameter(0));
			if (sys == NULL)
			{
				command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::RequiredCommandParametersNotProvided,
					concat("System with code \"")(command.Parameter(0))("\" does not exist").str().c_str());
			}
			else
			{
				command.SetSuccessOutput(sys->DebugString());
			}
		}
		return true;
	}
	else if (command.InputCommand == "GetCurrentSystem")
	{
		command.SetSuccessOutput(m_currentsystem->DebugString());
		return true;
	}
	else if (command.InputCommand == "GetSystemLights")
	{
		const std::vector<ObjectReference<LightSource>> & lights = m_currentsystem->SystemLightSources();
		std::string s = concat("Lights[")(lights.size())("] = {").str();

		std::vector<ObjectReference<LightSource>>::const_iterator it_end = lights.end();
		for (std::vector<ObjectReference<LightSource>>::const_iterator it = lights.begin(); it != it_end; ++it)
		{
			s = concat(s)(it == lights.begin() ? " " : ", ")("[ID=")((*it)()->GetID())("]").str();
		}
		s = concat(s)(" }").str();

		command.SetSuccessOutput(s);
		return true;
	}

	// We did not recognise the command
	return false;
}