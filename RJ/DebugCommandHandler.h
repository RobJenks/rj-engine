#pragma once

#ifndef __DebugCommandHandlerH__
#define __DebugCommandHandlerH__

#include <vector>
class Actor;

#include "iAcceptsConsoleCommands.h"

// This class has no special alignment requirements
class DebugCommandHandler : public iAcceptsConsoleCommands
{
public:

	// Default constructor
	DebugCommandHandler(void);

	// Virtual inherited method to accept a command from the console
	bool					ProcessConsoleCommand(GameConsoleCommand & command);

	// Default destructor
	~DebugCommandHandler(void);


protected:


	// Debug spawn a set of ships near the player
	void					SpawnDebugShips(SimpleShip *template_ship, int count);

	// Clear any ships that have been spawned via debug commands in the past
	void					ClearAllSpawnedShips(void);

	// Prints a log of all registered game objects to the debug output stream
	void					DebugPrintAllGameObjects(void);




protected:

};


#endif