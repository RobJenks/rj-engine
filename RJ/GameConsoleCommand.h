#pragma once

#ifndef __GameConsoleCommandH__
#define __GameConsoleCommandH__

#include <string>
#include <vector>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Utility.h"

// Represents one full command to the game console, including original text input (if applicable),
// input parameters and output results
// This class has no special alignment requirements
struct GameConsoleCommand
{
	// Enumeration of possible output results
	enum CommandResult { NotExecuted = 0, Success, Warning, Failure };

	// Original text input.  May not be present if command was generated internally
	std::string						RawTextInput;

	// Input parameters
	std::string						InputCommand;
	std::vector<std::string>		InputParameters;

	// Output results
	Result							OutputResult;
	CommandResult					OutputStatus;
	std::string						OutputString;

	// Default constructor
	GameConsoleCommand(void)
	{
		RawTextInput = "";
		InputCommand = "";
		OutputResult = ErrorCodes::NoError;
		OutputStatus = CommandResult::NotExecuted;
		OutputString = "";
	}

	// Constructor to generate a command directly from a raw input string
	GameConsoleCommand(const std::string & rawinput)
	{
		RawTextInput = rawinput;
		InputCommand = "";
		OutputResult = ErrorCodes::NoError;
		OutputStatus = CommandResult::NotExecuted;
		OutputString = "";
	}


	// Sets the output result of a command
	void SetOutput(GameConsoleCommand::CommandResult outputStatus, Result outputResult, const std::string & outputString)
	{
		OutputStatus = outputStatus;
		OutputResult = outputResult;
		OutputString = outputString;
	}

	// Sets the output result for a successful command execution; shortcut to SetOutput for Status==Success & Result==NoError
	void SetSuccessOutput(const std::string & outputString)
	{
		OutputStatus = CommandResult::Success;
		OutputResult = ErrorCodes::NoError;
		OutputString = outputString;
	}

	// Managed method to return the value of a parameter, or the empty string if no parameter exists
	CMPINLINE const std::string & Parameter(int index)
	{
		if (index < 0 || index >= (int)InputParameters.size())	return NullString;
		else													return InputParameters[index];
	}

	// Return a parameter as a different type, if possible
	float ParameterAsFloat(int index);
	int ParameterAsInt(int index);
	bool ParameterAsBool(int index);

	// Duplicates the command and all input parameters.  New command will always be 
	// created in not-executed state without any output results
	void CopyCommand(GameConsoleCommand & outNewCommand) const
	{
		// Duplicate all input parameters
		outNewCommand.RawTextInput = RawTextInput;
		outNewCommand.InputCommand = InputCommand;
		std::vector<std::string>::size_type paramcount = InputParameters.size();
		for (std::vector<std::string>::size_type i = 0; i < paramcount; ++i)
			outNewCommand.InputParameters.push_back(InputParameters[i]);

		// Default all output parameters, so the new command can be executed
		outNewCommand.OutputResult = ErrorCodes::NoError;
		outNewCommand.OutputStatus = CommandResult::NotExecuted;
		outNewCommand.OutputString = OutputString;
	}

	// Static instance that represents the 'null' command
	static const GameConsoleCommand NullCommand;

	// Static method that translates a status code to a string
	static std::string StatusToString(CommandResult status);		
};



#endif