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

	// Executes a debug command on the specified object.  Will locate the appropriate subclass to begin
	// passing the command down from.  Ugly, but avoids adding another function to the iObject vtable
	void					ExecuteDebugCommandOnObject(iObject *object, GameConsoleCommand & command);

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