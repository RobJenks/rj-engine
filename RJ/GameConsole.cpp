#include <string>
#include <vector>
#include "Utility.h"
#include "iAcceptsConsoleCommands.h"

#include "GameConsole.h"

// Public static vector of components that are registered to receive console inputs.  Items are 
// automatically added and removed from this list by the iAcceptsConsoleCommands interface
std::vector<iAcceptsConsoleCommands*> GameConsole::CommandReceivers;

// The number of commands that are held in history
unsigned int GameConsole::COMMAND_HISTORY_LENGTH = 100;

// Default constructor
GameConsole::GameConsole(void)
{
	
}

// Method to accept a console command.  Takes a raw text input, parses into the constituent input 
// parameters, processes the command, and then updates the command ouput parameters with the result
void GameConsole::ProcessRawCommand(GameConsoleCommand & command)
{
	// We need to first parse the raw text input and populate the command input parameters
	ParseCommandRawInput(command);

	// Now pass control to the main execution method
	ProcessCommand(command);
}

// Method to accept a console command with defined input parameters.  Processes the command and 
// updates the command output parameters with the result
void GameConsole::ProcessCommand(GameConsoleCommand & command)
{
	// Make sure we have at least an input command to execute
	if (command.InputCommand == NullString)
	{
		// Set the error parameters in the command output
		command.SetOutput(	GameConsoleCommand::CommandResult::Failure,
							ErrorCodes::CannotExecuteNullConsoleCommand,
							"Cannot execute null console command");

		// Always add to the command history, even though the command was null and failed
		AddCommandToHistory(command);

		// Return here since we cannot process a null command
		return;
	}

	// If we don't have a raw text version of the command, e.g. if the parameters were created
	// directly by an internal function, create it via concatenation now
	if (command.RawTextInput == NullString)
	{
		const std::string delimiter = " ";
		command.RawTextInput = concat(command.InputCommand)(delimiter)(ConcatenateStrings(command.InputParameters, delimiter)).str();
	}

	// Pass to each potential recipient in turn.  Component will return true if the command
	// was processed, in which case we can quit early since we only want to process the
	// command in one place.
	int n = CommandReceivers.size();
	for (int i = 0; i < n; ++i)
	{
		if (CommandReceivers[i]->ProcessConsoleCommand(command))
		{
			// The command was executed.  Add to the command history and quit
			AddCommandToHistory(command);
			return;
		}
	}

	// No registered recipient could accept the console command, so return failure
	command.SetOutput(	GameConsoleCommand::CommandResult::Failure,
						ErrorCodes::UnknownConsoleCommand,
						concat("Unknown console command: ")(command.InputCommand).str().c_str());

	// Always add to the command history, even though this was an unknown command that could not be executed
	AddCommandToHistory(command);
}

// Parses the raw text input of a command and populates the input parameters
void GameConsole::ParseCommandRawInput(GameConsoleCommand & command)
{
	// Split the entire command string by a ' ' delimiter, and populate in the input parameters
	command.InputParameters.clear();
	SplitString(command.RawTextInput, ' ', true, command.InputParameters);

	// The first element of the string should always be the command itself.  Assuming we do 
	// have >= 1 element, take that first element and make it the command name instead
	int numparams = command.InputParameters.size();
	if (numparams > 0)
	{
		// The first element will become the command name itself
		command.InputCommand = command.InputParameters[0];
		
		// Erase the first parameter, which will move all other parameters down to their correct positions
		command.InputParameters.erase(command.InputParameters.begin());
	}
}

// Adds a command to the history, purging old records if required
void GameConsole::AddCommandToHistory(GameConsoleCommand & command)
{
	// If the history is full then we want to purge the oldest record from the BACK of the de-queue
	if (m_history.size() >= GameConsole::COMMAND_HISTORY_LENGTH)
	{
		m_history.pop_back();
	}

	// We have at least one empty space in the command history, so add this command to the front of the dequeue
	m_history.push_front(command);
}

// Returns a command from the specified point in the history.  Valid values are 0 to (COMMAND_HISTORY_LENGTH-1),
// where 0 is the last command that was executed.  Returns a null command reference (with status = NotExecuted)
// if an item is not found for the specified location in the history
const GameConsoleCommand & GameConsole::GetConsoleHistoryItem(int index)
{
	// If the index is not valid, return a reference to the null command
	if (index < 0 || index >= (int)m_history.size()) return GameConsoleCommand::NullCommand;

	// Otherwise, return a reference to the item at this index.  Items are pushed onto the front of the dequeue, 
	// and popped from the back, (i.e. the reverse of a std::queue) so that items from 0 > n are newest > oldest.
	return m_history[index];
}

// Default destructor
GameConsole::~GameConsole(void)
{

}