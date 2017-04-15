#pragma once

#ifndef __DefaultValuesH__
#define __DefaultValuesH__

#include <limits>
#include "CompilerSettings.h"
#include "FastMath.h"

// Define default null values for common types
template <typename T> class DefaultValues
{
public:
	CMPINLINE static T NullValue(void) { throw "Default values not defined; unknown type";  }
	CMPINLINE static T OneValue(void) { throw "Default values not defined; unknown type"; }
	CMPINLINE static T EpsilonValue(void) { throw "Default values not defined; unknown type"; }
	CMPINLINE static T InfiniteValue(void) { throw "Default values not defined; unknown type"; }
	CMPINLINE static T MinValue(void) { throw "Default values not defined; unknown type"; }
	CMPINLINE static T MaxValue(void) { throw "Default values not defined; unknown type"; }
};

// Integer
template <> class DefaultValues<int>
{
public:
	CMPINLINE static int NullValue(void) { return 0; }
	CMPINLINE static int OneValue(void) { return 1; }
	CMPINLINE static int EpsilonValue(void) { return 1; }
	CMPINLINE static int InfiniteValue(void) { return INF_INT; }
	CMPINLINE static int MinValue(void) { return std::numeric_limits<int>::lowest(); }
	CMPINLINE static int MaxValue(void) { return (std::numeric_limits<int>::max)(); }
};

// Float
template <> class DefaultValues<float>
{
public:
	CMPINLINE static float NullValue(void) { return 0.0f; }
	CMPINLINE static float OneValue(void) { return 1.0f; }
	CMPINLINE static float EpsilonValue(void) { return Game::C_EPSILON; }
	CMPINLINE static float InfiniteValue(void) { return INF_FLOAT; }
	CMPINLINE static float MinValue(void) { return std::numeric_limits<float>::lowest(); }
	CMPINLINE static float MaxValue(void) { return (std::numeric_limits<float>::max)(); }
};

// Short
template <> class DefaultValues<short>
{
public:
	CMPINLINE static short NullValue(void) { return 0; }
	CMPINLINE static short OneValue(void) { return 1; }
	CMPINLINE static short EpsilonValue(void) { return 1; }
	CMPINLINE static short InfiniteValue(void) { return INF_SHORT; }
	CMPINLINE static short MinValue(void) { return std::numeric_limits<short>::lowest(); }
	CMPINLINE static short MaxValue(void) { return (std::numeric_limits<short>::max)(); }
};

// Long
template <> class DefaultValues<long>
{
public:
	CMPINLINE static long NullValue(void) { return 0L; }
	CMPINLINE static long OneValue(void) { return 1L; }
	CMPINLINE static long EpsilonValue(void) { return 1L; }
	CMPINLINE static long InfiniteValue(void) { return INF_LONG; }
	CMPINLINE static long MinValue(void) { return std::numeric_limits<long>::lowest(); }
	CMPINLINE static long MaxValue(void) { return (std::numeric_limits<long>::max)(); }
};

// Double
template <> class DefaultValues<double>
{
public:
	CMPINLINE static double NullValue(void) { return 0.0; }
	CMPINLINE static double OneValue(void) { return 1.0; }
	CMPINLINE static double EpsilonValue(void) { return Game::C_EPSILON_DP; }
	CMPINLINE static double InfiniteValue(void) { return INF_DOUBLE; }
	CMPINLINE static double MinValue(void) { return std::numeric_limits<double>::lowest(); }
	CMPINLINE static double MaxValue(void) { return (std::numeric_limits<double>::max)(); }
};

// Unsigned int
template <> class DefaultValues<unsigned int>
{
public:
	CMPINLINE static unsigned int NullValue(void) { return 0U; }
	CMPINLINE static unsigned int OneValue(void) { return 1U; }
	CMPINLINE static unsigned int EpsilonValue(void) { return 1U; }
	CMPINLINE static unsigned int InfiniteValue(void) { return INF_UINT; }
	CMPINLINE static unsigned int MinValue(void) { return std::numeric_limits<unsigned int>::lowest(); }
	CMPINLINE static unsigned int MaxValue(void) { return (std::numeric_limits<unsigned int>::max)(); }
};

// Char
template <> class DefaultValues<char>
{
public:
	CMPINLINE static char NullValue(void) { return 0; }
	CMPINLINE static char OneValue(void) { throw "No default value for type"; return 0; }
	CMPINLINE static char EpsilonValue(void) { throw "Epsilon value not valid for type"; }
	CMPINLINE static char InfiniteValue(void) { throw "Infinite value not valid for type"; }
	CMPINLINE static char MinValue(void) { throw "Minimum value not defined for type"; }
	CMPINLINE static char MaxValue(void) { throw "Maximum value not defined for type"; }
};

// Char *
template <> class DefaultValues<char*>
{
public:
	CMPINLINE static char* NullValue(void) { return ""; }
	CMPINLINE static char* OneValue(void) { throw "No default value for type"; return ""; }
	CMPINLINE static char* EpsilonValue(void) { throw "Epsilon value not valid for type"; }
	CMPINLINE static char* InfiniteValue(void) { throw "Infinite value not valid for type"; }
	CMPINLINE static char* MinValue(void) { throw "Minimum value not defined for type"; }
	CMPINLINE static char* MaxValue(void) { throw "Maximum value not defined for type"; }
};

// Const char *
template <> class DefaultValues<const char*>
{
public:
	CMPINLINE static const char* NullValue(void) { return ""; }
	CMPINLINE static const char* OneValue(void) { throw "No default value for type"; return ""; }
	CMPINLINE static const char* EpsilonValue(void) { throw "Epsilon value not valid for type"; }
	CMPINLINE static const char* InfiniteValue(void) { throw "Infinite value not valid for type"; }
	CMPINLINE static const char* MinValue(void) { throw "Minimum value not defined for type"; }
	CMPINLINE static const char* MaxValue(void) { throw "Maximum value not defined for type"; }
};

// Bool
template <> class DefaultValues<bool>
{
public:
	CMPINLINE static bool NullValue(void) { return false; }
	CMPINLINE static bool OneValue(void) { return true; }
	CMPINLINE static bool EpsilonValue(void) { throw "Epsilon value not valid for type"; }
	CMPINLINE static bool InfiniteValue(void) { throw "Infinite value not valid for type"; }
	CMPINLINE static bool MinValue(void) { throw "Minimum value not defined for type"; }
	CMPINLINE static bool MaxValue(void) { throw "Maximum value not defined for type"; }
};

// String 
template <> class DefaultValues<std::string>
{
public:
	CMPINLINE static std::string NullValue(void) { return ""; }
	CMPINLINE static std::string OneValue(void) { throw "No default value for type"; return ""; }
	CMPINLINE static std::string EpsilonValue(void) { throw "Epsilon value not valid for type"; }
	CMPINLINE static std::string InfiniteValue(void) { throw "Infinite value not valid for type"; }
	CMPINLINE static std::string MinValue(void) { throw "Minimum value not defined for type"; }
	CMPINLINE static std::string MaxValue(void) { throw "Maximum value not defined for type"; }
};


#endif













