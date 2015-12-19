#pragma once

#ifndef __DefaultValuesH__
#define __DefaultValuesH__

#include "CompilerSettings.h"

// Define default null values for common types
template <typename T> class DefaultValues
{
public:
	CMPINLINE static T NullValue(void) { throw "Default values not defined; unknown type";  }
	CMPINLINE static T OneValue(void) { throw "Default values not defined; unknown type"; }
	CMPINLINE static T EpsilonValue(void) { throw "Default values not defined; unknown type"; }
};

// Integer
template <> class DefaultValues<int>
{
public:
	CMPINLINE static int NullValue(void) { return 0; }
	CMPINLINE static int OneValue(void) { return 1; }
	CMPINLINE static int EpsilonValue(void) { throw "Epsilon value not valid for type"; }
};

// Float
template <> class DefaultValues<float>
{
public:
	CMPINLINE static float NullValue(void) { return 0.0f; }
	CMPINLINE static float OneValue(void) { return 1.0f; }
	CMPINLINE static float EpsilonValue(void) { return Game::C_EPSILON; }
};

// Short
template <> class DefaultValues<short>
{
public:
	CMPINLINE static short NullValue(void) { return 0; }
	CMPINLINE static short OneValue(void) { return 1; }
	CMPINLINE static short EpsilonValue(void) { throw "Epsilon value not valid for type"; }
};

// Long
template <> class DefaultValues<long>
{
public:
	CMPINLINE static long NullValue(void) { return 0; }
	CMPINLINE static long OneValue(void) { return 1; }
	CMPINLINE static long EpsilonValue(void) { throw "Epsilon value not valid for type"; }
};

// Double
template <> class DefaultValues<double>
{
public:
	CMPINLINE static double NullValue(void) { return 0.0; }
	CMPINLINE static double OneValue(void) { return 1.0; }
	CMPINLINE static double EpsilonValue(void) { return Game::C_EPSILON_DP; }
};

// Unsigned int
template <> class DefaultValues<unsigned int>
{
public:
	CMPINLINE static unsigned int NullValue(void) { return 0U; }
	CMPINLINE static unsigned int OneValue(void) { return 1U; }
	CMPINLINE static unsigned int EpsilonValue(void) { throw "Epsilon value not valid for type"; }
};

// Char
template <> class DefaultValues<char>
{
public:
	CMPINLINE static char NullValue(void) { return 0; }
	CMPINLINE static char OneValue(void) { throw "No default value for type"; return 0; }
	CMPINLINE static char EpsilonValue(void) { throw "Epsilon value not valid for type"; }
};

// Char *
template <> class DefaultValues<char*>
{
public:
	CMPINLINE static char* NullValue(void) { return ""; }
	CMPINLINE static char* OneValue(void) { throw "No default value for type"; return ""; }
	CMPINLINE static char* EpsilonValue(void) { throw "Epsilon value not valid for type"; }
};

// Const char *
template <> class DefaultValues<const char*>
{
public:
	CMPINLINE static const char* NullValue(void) { return ""; }
	CMPINLINE static const char* OneValue(void) { throw "No default value for type"; return ""; }
	CMPINLINE static const char* EpsilonValue(void) { throw "Epsilon value not valid for type"; }
};

// Bool
template <> class DefaultValues<bool>
{
public:
	CMPINLINE static bool NullValue(void) { return false; }
	CMPINLINE static bool OneValue(void) { return true; }
	CMPINLINE static bool EpsilonValue(void) { throw "Epsilon value not valid for type"; }
};

// String 
template <> class DefaultValues<std::string>
{
public:
	CMPINLINE static std::string NullValue(void) { return ""; }
	CMPINLINE static std::string OneValue(void) { throw "No default value for type"; return ""; }
	CMPINLINE static std::string EpsilonValue(void) { throw "Epsilon value not valid for type"; }
};


#endif













