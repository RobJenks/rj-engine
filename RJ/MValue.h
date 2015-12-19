#pragma once

#ifndef __MValueH__
#define __MValueH__

#include "Modifier.h"

template <typename T>
class MValue
{

public:

	// Default constructor for a null value
	MValue(void)		: BaseValue(DefaultValues<T>::NullValue()), Value(DefaultValues<T>::NullValue()), m_update_suspended(false) { }

	// Constructor to wrap a base value in modifier logic
	MValue(T value)		: BaseValue(value), Value(value), m_update_suspended(false) { }

	// Base value
	T											BaseValue;

	// Resulting (fully-modified) value
	T											Value;

	// Adds a new modifier
	CMPINLINE void 								AddModifier(typename Modifier<T>::ModifierType type, T value)	{ AddModifier(Modifier<T>(type, value)); }
	CMPINLINE void								AddModifier(const Modifier<T> & modifier)				
	{ 
		m_modifiers.push_back(modifier); 
		if (!m_update_suspended) RecalculateValue();
	}

	// Remove all modifiers
	CMPINLINE void								RemoveAllModifiers(void)
	{
		m_modifiers.clear();
		if (!m_update_suspended) Value = BaseValue;
	}
	

	// Removes any modifier of the specified type
	void										RemoveModifiersOfType(typename Modifier<T>::ModifierType type);

	// Removes the specified modifier.  Only one removed if multiple matching modifiers.  Exact matching
	void										RemoveModifier(const Modifier<T> & modifier);
	void										RemoveModifier(typename Modifier<T>::ModifierType type, T value);

	// Removes all instances of the specified modifier.  Exact matching
	void										RemoveAnyModifier(const Modifier<T> & modifier);
	void										RemoveAnyModifier(typename Modifier<T>::ModifierType type, T value);

	// Removes the specified modifier.  Only one removed if multiple matching modifiers.  Allows slight matching threshold for e.g. FP rounding differences
	void										RemoveModifierApprox(const Modifier<T> & modifier);
	void										RemoveModifierApprox(typename Modifier<T>::ModifierType type, T value);

	// Removes all instances of the specified modifier.  Allows slight matching threshold for e.g. FP rounding differences
	void										RemoveAnyModifierApprox(const Modifier<T> & modifier);
	void										RemoveAnyModifierApprox(typename Modifier<T>::ModifierType type, T value);

	// Retrieves a constant reference to the full modifier set for this value
	CMPINLINE typename const Modifier<T>::ModifierSet &	GetModifiers(void) const	{ return Modifiers; }

	// Sets modifiers for this value based on a constant reference to the modifiers of another
	void										SetModifiers(typename const Modifier<T>::ModifierSet & modifiers);

	// Suspends recalculation of the value, e.g. while multiple modifiers are being applied
	CMPINLINE void								SuspendUpdates(void)		{ m_update_suspended = true; }

	// Resume updates of the value.  Performs an immediate recalculation
	CMPINLINE void								ResumeUpdates(void)
	{
		m_update_suspended = false;
		RecalculateValue();
	}

	// Copy constructor to create an MValue from an existing value
	MValue(const MValue<T> & source);

protected:

	// Set of modifiers applied to this value
	typename Modifier<T>::ModifierSet			m_modifiers;

	// Flag that indicates whether updates are currently suspended, e.g. while we add multiple modifiers in one go
	bool										m_update_suspended;

	// Recalculates the value based on all modifiers that have been applied
	void										RecalculateValue(void);

};


// Removes any modifier of the specified type
template <typename T>
void MValue<T>::RemoveModifiersOfType(typename Modifier<T>::ModifierType type)
{
	// Erase all matching elements and then recalculate the value
	m_modifiers.erase(std::remove_if(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsOfType(type)), m_modifiers.end());
	if (!m_update_suspended) RecalculateValue();
}

// Removes the specified modifier.  Only one removed if multiple matching modifiers.  Exact matching
template <typename T>
void MValue<T>::RemoveModifier(const Modifier<T> & modifier)
{
	Modifier<T>::ModifierSet::const_iterator it_end = m_modifiers.end();
	for (Modifier<T>::ModifierSet::const_iterator it = m_modifiers.begin(); it != it_end; ++it)
	{
		if ((*it) == modifier)
		{
			m_modifiers.erase(it);
			RecalculateValue();
			return;
		}
	}
}

// Removes the specified modifier.  Only one removed if multiple matching modifiers.  Exact matching
template <typename T>
void MValue<T>::RemoveModifier(typename Modifier<T>::ModifierType type, T value)
{
	Modifier<T>::ModifierSet::const_iterator it_end = m_modifiers.end();
	for (Modifier<T>::ModifierSet::const_iterator it = m_modifiers.begin(); it != it_end; ++it)
	{
		if ((*it).Type == type && (*it).Value == value)
		{
			m_modifiers.erase(it);
			RecalculateValue();
			return;
		}
	}
}

// Removes all instances of the specified modifier.  Exact matching
template <typename T>
void MValue<T>::RemoveAnyModifier(const Modifier<T> & modifier)
{
	// Erase all matching elements and then recalculate the value
	m_modifiers.erase(std::remove_if(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsEqual(modifier)), m_modifiers.end());
	if (!m_update_suspended) RecalculateValue();
}

// Removes all instances of the specified modifier.  Exact matching
template <typename T>
void MValue<T>::RemoveAnyModifier(typename Modifier<T>::ModifierType type, T value)
{
	// Erase all matching elements and then recalculate the value
	m_modifiers.erase(std::remove_if(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsEqual(type, value)), m_modifiers.end());
	if (!m_update_suspended) RecalculateValue();
}

// Removes the specified modifier.  Only one removed if multiple matching modifiers.  Allows slight matching threshold for e.g. FP rounding differences
template <typename T>
void MValue<T>::RemoveModifierApprox(const Modifier<T> & modifier)
{
	Modifier<T>::ModifierSet::const_iterator it_end = m_modifiers.end();
	for (Modifier<T>::ModifierSet::const_iterator it = m_modifiers.begin(); it != it_end; ++it)
	{
		if ((*it).Type == modifier.Type && (std::abs((*it).Value - modifier.Value) < DefaultValues<T>::EpsilonValue()))
		{
			m_modifiers.erase(it);
			RecalculateValue();
			return;
		}
	}
}

// Removes the specified modifier.  Only one removed if multiple matching modifiers.  Allows slight matching threshold for e.g. FP rounding differences
template <typename T>
void MValue<T>::RemoveModifierApprox(typename Modifier<T>::ModifierType type, T value)
{
	Modifier<T>::ModifierSet::const_iterator it_end = m_modifiers.end();
	for (Modifier<T>::ModifierSet::const_iterator it = m_modifiers.begin(); it != it_end; ++it)
	{
		if ((*it).Type == type && (std::abs((*it).Value - value) < DefaultValues<T>::EpsilonValue()))
		{
			m_modifiers.erase(it);
			RecalculateValue();
			return;
		}
	}
}

// Removes all instances of the specified modifier.  Allows slight matching threshold for e.g. FP rounding differences
template <typename T>
void MValue<T>::RemoveAnyModifierApprox(const Modifier<T> & modifier)
{
	// Erase all matching elements and then recalculate the value
	m_modifiers.erase(std::remove_if(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsApproxEqual(modifier)), m_modifiers.end());
	if (!m_update_suspended) RecalculateValue();
}

// Removes all instances of the specified modifier.  Allows slight matching threshold for e.g. FP rounding differences
template <typename T>
void MValue<T>::RemoveAnyModifierApprox(typename Modifier<T>::ModifierType type, T value)
{
	// Erase all matching elements and then recalculate the value
	m_modifiers.erase(std::remove_if(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsApproxEqual(type, value)), m_modifiers.end());
	if (!m_update_suspended) RecalculateValue();
}


// Recalculates the value based on all modifiers that have been applied
template <typename T>
void MValue<T>::RecalculateValue(void)
{
	// Start with the base value and apply additive modifiers immediately.  Accumulate multiplicative 
	// multipliers for post-multiplication
	T mult = DefaultValues<T>::OneValue();
	Value = BaseValue;

	// Iterate over all modifiers on this value
	Modifier<T>::ModifierSet::const_iterator it_end = m_modifiers.end();
	for (Modifier<T>::ModifierSet::const_iterator it = m_modifiers.begin(); it != it_end; ++it)
	{
		if ((*it).Type == Modifier<T>::ModifierType::Additive)	Value += (*it).Value;
		else													mult *= (*it).Value;
	}

	// The value has all additive modifiers applied; post-multiply by the combined multipliers to give the final value
	Value *= mult;
}

// Sets modifiers for this value based on a constant reference to the modifiers of another
template <typename T>
void MValue<T>::SetModifiers(typename const Modifier<T>::ModifierSet & modifiers)
{
	// Simply use direct vector assignment and then recalculate the current value
	Modifiers = modifiers;
	RecalculateValue();
}

// Copy constructor to create an MValue from an existing value
template <typename T>
MValue<T>::MValue(const MValue<T> & source)
{
	BaseValue = source.BaseValue;
	Modifiers = source.Modifiers;
	Value = source.Value;
	m_update_suspended = false;
}


#endif







