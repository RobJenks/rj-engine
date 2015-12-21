#pragma once

#ifndef __MValueH__
#define __MValueH__

#include "CompilerSettings.h"
#include "DefaultValues.h"
#include "Modifier.h"

template <typename T>
class MValue
{

public:

	// Begin creating a new modified value
	static CMPINLINE void			New(void)								{ m_current = DefaultValues<T>::NullValue(); m_multiplier = DefaultValues<T>::OneValue(); }
	static CMPINLINE void			New(T basevalue)						{ m_current = basevalue; m_multiplier = DefaultValues<T>::OneValue(); }

	// Add a modifier to the current value
	static CMPINLINE void			AddModifier(const Modifier<T> & modifier)
	{
		if (modifier.Type == Modifier<T>::ModifierType::Multiplicative)		m_multiplier *= modifier.Value;
		else																m_current += modifier.Value;
	}

	// Add a modifier to the current value
	static CMPINLINE void			AddModifier(typename Modifier<T>::ModifierType type, T value)
	{
		if (type == Modifier<T>::ModifierType::Multiplicative)				m_multiplier *= value;
		else																m_current += value;
	}

	// Add a modifier to the current value
	static CMPINLINE void			AddAdditiveModifier(T value)			{ m_current += value; }
	
	// Add a modifier to the current value
	static CMPINLINE void			AddMultiplicativeModifier(T value)		{ m_multiplier *= value; }

	// Add a modifier to the current value
	static CMPINLINE void			RemoveModifier(const Modifier<T> & modifier)
	{
		if (modifier.Type == Modifier<T>::ModifierType::Multiplicative)		m_multiplier /= modifier.Value;
		else																m_current -= modifier.Value;
	}

	// Add a modifier to the current value
	static CMPINLINE void			RemoveModifier(typename Modifier<T>::ModifierType type, T value)
	{
		if (type == Modifier<T>::ModifierType::Multiplicative)				m_multiplier /= value;
		else																m_current -= value;
	}

	// Add a modifier to the current value
	static CMPINLINE void			RemoveAdditiveModifier(T value)			{ m_current -= value; }

	// Add a modifier to the current value
	static CMPINLINE void			RemoveMultiplicativeModifier(T value)	{ if (value != 0.0f) m_multiplier /= value; }


	// Return the final modified value
	static CMPINLINE T				Value(void)								{ return (m_current * m_multiplier); }




private:

	static T						m_current, m_multiplier;

};


// Initialise static field
template <typename T>
T MValue<T>::m_current = DefaultValues<T>::NullValue();

// Initialise static field
template <typename T>
T MValue<T>::m_multiplier = DefaultValues<T>::OneValue();




#endif








