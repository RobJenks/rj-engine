#pragma once

#ifndef __EnvironmentMapBlendModeH__
#define __EnvironmentMapBlendModeH__

#include "CompilerSettings.h"

class EnvironmentMapBlendMode
{
public:

	// Enumeration describing the possible ways of combining cell values when the destination value 
	// is not equal to its initial value (== T::NullValue unless otherwise specified)
	enum MapBlendType
	{
		Additive = 0,
		Multiplicative, 
		MinimumValue,
		MaximumValue,
		ReplaceDestination,			// Ignores and overwrites any existing value
		IgnoreNewValue				// Ignores the new value and retains the existing value
	};

	// Type definition for each blend type, to allow templating off the blend mode
	template <typename T>
	struct BlendAdditive 
	{ 
		CMPINLINE static const bool USES_INITIAL_VALUE = false;
		CMPINLINE void SetInitialValue(T value) { /* Blend does not incorporate the initial value */}
		CMPINLINE T Apply(T current, T newvalue) { return (current + newvalue); }
	};

	struct BlendSelectMin { };
	struct BlendSelectMax { };
	struct BlendMultiplicative { };
	struct BlendReplaceDestination { };
	struct BlendIgnoreNewValue { };

};




#endif



