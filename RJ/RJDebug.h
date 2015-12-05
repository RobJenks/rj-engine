#pragma once

#ifndef __DebugFunctionsH__
#define __DebugFunctionsH__

#include <stdarg.h>
#include "CompilerSettings.h"
#include "Utility.h"
#include "GameVarsExtern.h"


class RJDebug
{
public:

	enum LogPrefix { NoPrefix = 0, ClockTime, ClockMs };

	CMPINLINE static void Print(RJDebug::LogPrefix prefix, const std::string & str)
	{
		std::string full_str = _ConstructPrefixedString(str, prefix);
		_DebugPrint_Core(full_str.c_str());
	}

	CMPINLINE static void Print(const std::string & str)
	{
		std::string full_str = _ConstructPrefixedString(str, RJDebug::LogPrefix::NoPrefix);
		_DebugPrint_Core(full_str.c_str());
	}

	CMPINLINE static void Print(RJDebug::LogPrefix prefix, const char *format_string, ...)
	{
		// Allocate a temporary destination buffer
		const size_t max_length = 1024U;
		char *output = new (nothrow) char[1024];
		if (!output) return;

		// Use the (outdated) variadic arguments functionality to wrap sprintf
		va_list args;
		va_start(args, format_string);
		vsprintf_s(output, (max_length * sizeof(char)), format_string, args);
		va_end(args);

		// Prepend the required prefix value, if required
		std::string str = _ConstructPrefixedString(std::string(output), prefix);

		// Convert to the final string representation and pass to the core output function
		_DebugPrint_Core(str.c_str());

		// Deallocate the temporary buffer that was allocated
		delete output;
	}

	CMPINLINE static void Print(const char *format_string, ...)
	{
		// Use the (outdated) variadic arguments functionality to pass the variable-length argument string
		va_list args;
		va_start(args, format_string);
		Print(RJDebug::LogPrefix::NoPrefix, format_string, args);
		va_end(args);
	}

protected:

	// Core output function to the debug output
	CMPINLINE static void	_DebugPrint_Core(const char *output)
	{
		if (output) OutputDebugString(output);
	}

	// Adds any required prefix and returns the updated string
	CMPINLINE static std::string _ConstructPrefixedString(const std::string & str, RJDebug::LogPrefix prefix)
	{
		switch (prefix)
		{
			case RJDebug::LogPrefix::ClockTime:
				return concat("[")(Game::ClockTime)("] ")(str).str(); 
			case RJDebug::LogPrefix::ClockMs:
				return concat("[")(Game::ClockMs)("] ")(str).str(); 
			default:
				return str;
		}
	}

};








#endif