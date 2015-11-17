#pragma once

#ifndef __iAcceptsConsoleCommandsH__
#define __iAcceptsConsoleCommandsH__

#include "Utility.h"
#include "GameConsole.h"
struct GameConsoleCommand;

// Interface for any object that accepts commands from the console.  Objects implementing this interface are 
// automatically added to a list of components which are polled to accept each console command.  Should 
// be reserved for only a few, core components.  Should also be reserved for components that are onlhy 
// instantiated once, otherwise the first instance of an implementing class will process the command and
// subsequent instances will not, which is probably not the intended outcome.
// This class has no special alignment requirements
class iAcceptsConsoleCommands
{
public:

	// Default constructor; automatically registers the object with a list of components which are polled 
	// to accept each console command.  As a result, this interface should be limited to only key components
	iAcceptsConsoleCommands(void)
	{
		GameConsole::CommandReceivers.push_back(this);
	}

	// Virtual inherited method to accept a command from the console
	virtual bool ProcessConsoleCommand(GameConsoleCommand & command) = 0;

	// Default destructor; automatically removes the object from a list of components which are polled to
	// to accept each console command.
	~iAcceptsConsoleCommands(void)
	{
		RemoveFromVector<iAcceptsConsoleCommands*>(GameConsole::CommandReceivers, this);
	}
};




#endif