#include "GameConsoleCommand.h"

// Instantiate the static 'null' command
const GameConsoleCommand GameConsoleCommand::NullCommand = GameConsoleCommand();

// Static method that translates a status code to a string
std::string GameConsoleCommand::StatusToString(GameConsoleCommand::CommandResult status)
{
	switch (status)
	{
		case GameConsoleCommand::CommandResult::Success:	return "Success";
		case GameConsoleCommand::CommandResult::Warning:	return "Warning";
		case GameConsoleCommand::CommandResult::Failure:	return "Failure";
		default:											return "";
	}
}