#include <string>
#include <unordered_map>
#include "DX11_Core.h"

#include "SpaceSystem.h"

#include "GameUniverse.h"
using namespace std;
using namespace std::tr1;

Result GameUniverse::InitialiseUniverse(void)
{
	// No initialisation to be performed; simply return success
	return ErrorCodes::NoError;
}

Result GameUniverse::ProcessLoadedSystems(ID3D11Device *device)
{
	// Initialise each loaded system in turn
	SystemRegister::const_iterator it_end = Systems.end();
	for (SystemRegister::const_iterator it = Systems.begin(); it != it_end; ++it) 
	{
		if (it->second) 
		{
			// Initialise the system
			Result res = it->second->InitialiseSystem(device);
			if (res != ErrorCodes::NoError) return res;
		}
	}
	
	// Return success if no errors were encounterd loading systems.  TODO: could make this more fault tolerant
	// by not returning error on the first failure, and proceeding with the remaining systems regardless
	return ErrorCodes::NoError;
}

void GameUniverse::AddSystem(SpaceSystem *system)
{
	// Make sure the system has a valid code, otherwise we will not add it
	if (!system || system->GetCode() == NullString) return;

	// Add to the systems collection
	Systems[system->GetCode()] = system;
}

SpaceSystem	*GameUniverse::GetSystem(string code)
{
	return Systems[code];
}


// Termination function: Clears all universe and system data
void GameUniverse::TerminateUniverse()
{
	// Delete each system in turn
	SystemRegister::const_iterator it_end = Systems.end();
	for (SystemRegister::const_iterator it = Systems.begin(); it != it_end; ++it) {
		if (it->second) {
			// Delete the object and deallocate memory
			it->second->TerminateSystem();
			delete (it->second);
		}
	}

	// Empty the register now it is full of null/invalid pointers
	Systems.clear();
}



GameUniverse::GameUniverse(void)
{
}

GameUniverse::~GameUniverse(void)
{
}
