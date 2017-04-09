#pragma once

#ifndef __EnvironmentMapBlendModeH__
#define __EnvironmentMapBlendModeH__

#include "CompilerSettings.h"
#include "DefaultValues.h"

class EnvironmentMapBlendMode
{
public:

	// Enumeration describing the possible ways of combining cell values when the destination value 
	// is not equal to its initial value (== T::NullValue unless otherwise specified)
	enum MapBlendType
	{
		Additive = 0,
		Multiplicative, 
		Averaged,
		MinimumValue,
		MaximumValue,
		ReplaceDestination,			// Ignores and overwrites any existing value
		IgnoreNewValue				// Ignores the new value and retains the existing value
	};

	// Type definition for each blend type, to allow templating off the blend mode
	template <typename T>
	struct BlendAdditive 
	{ 
		static const bool USES_INITIAL_VALUE = false;
		CMPINLINE void SetInitialValue(T value) { /* Blend does not incorporate the initial value */}
		CMPINLINE T Apply(T current, T newvalue) { return (current + newvalue); }
	};

	template <typename T>
	struct BlendMultiplicative
	{
		static const bool USES_INITIAL_VALUE = false;
		CMPINLINE void SetInitialValue(T value) { /* Blend does not incorporate the initial value */ }
		CMPINLINE T Apply(T current, T newvalue) { return (current * newvalue); }
	};

	template <typename T>
	struct BlendAveraged
	{
		static const bool USES_INITIAL_VALUE = false;
		CMPINLINE void SetInitialValue(T value) { /* Blend does not incorporate the initial value */ }
		CMPINLINE T Apply(T current, T newvalue) { return (T)((current + newvalue) * 0.5f); }
	};

	template <typename T>
	struct BlendMinimumValue
	{
		static const bool USES_INITIAL_VALUE = false;
		CMPINLINE void SetInitialValue(T value) { /* Blend does not incorporate the initial value */ }
		CMPINLINE T Apply(T current, T newvalue) { return min(current, newvalue); }
	};

	template <typename T>
	struct BlendMaximumValue
	{
		static const bool USES_INITIAL_VALUE = false;
		CMPINLINE void SetInitialValue(T value) { /* Blend does not incorporate the initial value */ }
		CMPINLINE T Apply(T current, T newvalue) { return max(current, newvalue); }
	};

	template <typename T>
	struct BlendReplaceDestination
	{
		static const bool USES_INITIAL_VALUE = false;
		CMPINLINE void SetInitialValue(T value) { /* Blend does not incorporate the initial value */ }
		CMPINLINE T Apply(T current, T newvalue) { return newvalue; }
	};

	template <typename T>
	struct BlendIgnoreNewValue
	{
		static const bool USES_INITIAL_VALUE = true;
		CMPINLINE void SetInitialValue(T value) { m_initialvalue = value; }
		CMPINLINE T Apply(T current, T newvalue) { return (current == m_initialvalue ? newvalue : current); }

	private:
		T m_initialvalue = DefaultValues<T>::NullValue();
	};

};




#endif



