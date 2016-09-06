#include "GameConsoleCommand.h"

// Instantiate the static 'null' command
const GameConsoleCommand GameConsoleCommand::NullCommand = GameConsoleCommand();

// Return a parameter as a different type, if possible
float GameConsoleCommand::ParameterAsFloat(int index)
{
	std::string s = Parameter(index);
	if (s != NullString)
	{
		const char *c = s.c_str();
		if (c)
		{
			return (float)atof(c);
		}
	}

	return 0.0f;
}

// Return a parameter as a different type, if possible
int GameConsoleCommand::ParameterAsInt(int index)
{
	std::string s = Parameter(index);
	if (s != NullString)
	{
		const char *c = s.c_str();
		if (c)
		{
			return atoi(c);
		}
	}

	return 0;
}

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