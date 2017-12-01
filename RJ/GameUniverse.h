#pragma once

#ifndef __GameUniverseH__
#define __GameUniverseH__

#include <string>
#include <unordered_map>
#include "DX11_Core.h"
#include "Rendering.h"
#include "iAcceptsConsoleCommands.h"
class SpaceSystem;


class GameUniverse : public iAcceptsConsoleCommands
{
public:
	GameUniverse(void);
	~GameUniverse(void);

	Result						InitialiseUniverse(void);
	Result						ProcessLoadedSystems(Rendering::RenderDeviceType  *device);

	// Adds a new system to the universe
	void						AddSystem(SpaceSystem *system);

	// Returns the system with the specified code, or NULL if no such system exists
	SpaceSystem *				GetSystem(const std::string & code);

	// Returns the current active system
	CMPINLINE SpaceSystem &		GetCurrentSystem(void)					{ return *m_currentsystem; }

	// Sets the current active system; will be repointed to the null system if "sys" is not valid
	void						SetCurrentSystem(SpaceSystem * sys);

	// Event raised when the current system changes
	void						CurrentSystemChanged(SpaceSystem & old_system, SpaceSystem & new_system);

	// Shuts down and deallocates all resources used by the universe & component systems
	void						TerminateUniverse(void);

	// Collection type for the set of systems in this universe
	typedef						std::unordered_map<std::string, SpaceSystem*> SystemRegister;

	// The collection of all space systems in the game
	SystemRegister				Systems;					

	// Returns a reference to the "null system", used in any situation that we are not referencign a real system
	CMPINLINE SpaceSystem &		NullSystem(void)						{ return *m_nullsystem; }

	// Virtual inherited method to accept a command from the console
	bool						ProcessConsoleCommand(GameConsoleCommand & command);

protected:

	// The current system.  Not necessarily the same system that contains the player character, if e.g. the camera
	// is currently viewing something in a different location
	SpaceSystem *				m_currentsystem;

	// The "null system", used in any situation that the player is not actually in a real system
	SpaceSystem	*				m_nullsystem;

};


#endif