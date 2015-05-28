#pragma once

#ifndef __GameConsoleH__
#define __GameConsoleH__

#include <string>
#include <deque>
#include "GameConsoleCommand.h"
class iAcceptsConsoleCommands;

class GameConsole
{
public:

	// Default constructor
	GameConsole(void);

	// Method to accept a console command.  Takes a raw text input, parses into the constituent input 
	// parameters, processes the command, and then updates the command ouput parameters with the result
	void								ProcessRawCommand(GameConsoleCommand & command);

	// Method to accept a console command with defined input parameters.  Processes the command and 
	// updates the command output parameters with the result
	void								ProcessCommand(GameConsoleCommand & command);

	// Parses the raw text input of a command and populates the input parameters
	void								ParseCommandRawInput(GameConsoleCommand & command);

	// Adds a command to the history, purging old records if required
	void								AddCommandToHistory(GameConsoleCommand & command);

	// Returns a command from the specified point in the history.  Valid values are 0 to (COMMAND_HISTORY_LENGTH-1),
	// where 0 is the last command that was executed.  Returns a null command reference (with status = NotExecuted)
	// if an item is not found for the specified location in the history
	const GameConsoleCommand &			GetConsoleHistoryItem(int index);

	// Returns the number of items currently held in the console history
	CMPINLINE int						GetHistoryLength(void) const { return m_history.size(); }

	// Default destructor
	~GameConsole(void);

	// Public static vector of components that are registered to receive console inputs.  Items are 
	// automatically added and removed from this list by the iAcceptsConsoleCommands interface
	static std::vector<iAcceptsConsoleCommands*> CommandReceivers;

	// The number of commands that are held in history
	static unsigned int					COMMAND_HISTORY_LENGTH;

protected:

	// Maintain a queue of all executed commands and the results.  Will wrap around 
	// after COMMAND_HISTORY_LENGTH commands
	std::deque<GameConsoleCommand>		m_history;
};


#endif