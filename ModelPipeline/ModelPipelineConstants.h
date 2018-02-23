#pragma once

#include <string>


class ModelPipelineConstants
{
public:

	// Model pipeline application version
	static const std::string				VERSION;


	// Logging levels
	enum class LoggingType
	{
		Normal = 0,
		Verbose = 1,
		DebugVerbose = 2
	};

	// Application logging level
	static LoggingType						LogLevel;

};