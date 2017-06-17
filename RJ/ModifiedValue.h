#pragma once

#ifndef __ModifiedValueH__
#define __ModifiedValueH__

#include "Modifier.h"

template <typename T>
class ModifiedValue
{

public:

	// Default constructor for a null value
	ModifiedValue(void)							: BaseValue(DefaultValues<T>::NullValue()), Value(DefaultValues<T>::NullValue()), m_update_suspended(false) { }

	// Constructor to wrap a base value in modifier logic
	ModifiedValue(T value)						: BaseValue(value), Value(value), m_update_suspended(false) { }

	// Base value
	T											BaseValue;
	CMPINLINE T									GetBaseValue(void) const										{ return BaseValue; }
	CMPINLINE void								SetBaseValue(T value)
	{
		BaseValue = value;
		if (!m_update_suspended) RecalculateValue();
	}

	// Resulting (fully-modified) value
	T											Value;
	CMPINLINE T									GetValue(void) const											{ return Value; }

	// Adds a new modifier
	CMPINLINE void 								AddModifier(typename Modifier<T>::ModifierType type, T value)	
	{ 
		AddModifier(Modifier<T>(type, value)); 
	}

	// Adds a new modifier
	CMPINLINE void 								AddModifier(typename Modifier<T>::ModifierType type, T value, ModifierDetails::ModifierID details)	
	{ 
		AddModifier(Modifier<T>(type, value, details)); 
	}

	// Adds a new modifier
	// TODO: Should probably define a Modifier move constructor and then pass around by value only, since they are only comprised of three values
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
	CMPINLINE typename const Modifier<T>::ModifierSet &	GetModifiers(void) const	{ return m_modifiers; }

	// Sets modifiers for this value based on a constant l/r value reference to the modifiers of another
	void										SetModifiers(typename const Modifier<T>::ModifierSet & modifiers);
	void										SetModifiers(typename Modifier<T>::ModifierSet && modifiers);

	// Suspends recalculation of the value, e.g. while multiple modifiers are being applied
	CMPINLINE void								SuspendUpdates(void)		{ m_update_suspended = true; }

	// Resume updates of the value.  Performs an immediate recalculation
	CMPINLINE void								ResumeUpdates(void)
	{
		m_update_suspended = false;
		RecalculateValue();
	}

	// Forces a recalculation of the final value.  Not required unless Value is modified directly, which is not advisable
	CMPINLINE void								ForceRecalculation(void)	{ ResumeUpdates(); }

	// Copy constructor to create an ModifiedValue from an existing value
	ModifiedValue(const ModifiedValue<T> & source);

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
void ModifiedValue<T>::RemoveModifiersOfType(typename Modifier<T>::ModifierType type)
{
	// Erase all matching elements and then recalculate the value
	Modifier<T>::ModifierSet::iterator it = std::partition(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsNotOfType(type));
	if (it != m_modifiers.end())
	{
		m_modifiers.erase(it, m_modifiers.end());
		if (!m_update_suspended) RecalculateValue();
	}
}

// Removes the specified modifier.  Only one removed if multiple matching modifiers.  Exact matching
template <typename T>
void ModifiedValue<T>::RemoveModifier(const Modifier<T> & modifier)
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
void ModifiedValue<T>::RemoveModifier(typename Modifier<T>::ModifierType type, T value)
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
void ModifiedValue<T>::RemoveAnyModifier(const Modifier<T> & modifier)
{
	// Erase all matching elements and then recalculate the value
	Modifier<T>::ModifierSet::iterator it = std::partition(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsNotEqual(modifier));
	if (it != m_modifiers.end())
	{
		m_modifiers.erase(it, m_modifiers.end());
		if (!m_update_suspended) RecalculateValue();
	}
}

// Removes all instances of the specified modifier.  Exact matching
template <typename T>
void ModifiedValue<T>::RemoveAnyModifier(typename Modifier<T>::ModifierType type, T value)
{
	// Erase all matching elements and then recalculate the value
	Modifier<T>::ModifierSet::iterator it = std::partition(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsNotEqual(type, value));
	if (it != m_modifiers.end())
	{
		m_modifiers.erase(it, m_modifiers.end());
		if (!m_update_suspended) RecalculateValue();
	}
}

// Removes the specified modifier.  Only one removed if multiple matching modifiers.  Allows slight matching threshold for e.g. FP rounding differences
template <typename T>
void ModifiedValue<T>::RemoveModifierApprox(const Modifier<T> & modifier)
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
void ModifiedValue<T>::RemoveModifierApprox(typename Modifier<T>::ModifierType type, T value)
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
void ModifiedValue<T>::RemoveAnyModifierApprox(const Modifier<T> & modifier)
{
	// Erase all matching elements and then recalculate the value
	Modifier<T>::ModifierSet::iterator it = std::partition(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsNotApproxEqual(modifier));
	if (it != m_modifiers.end())
	{
		m_modifiers.erase(it, m_modifiers.end());
		if (!m_update_suspended) RecalculateValue();
	}
}

// Removes all instances of the specified modifier.  Allows slight matching threshold for e.g. FP rounding differences
template <typename T>
void ModifiedValue<T>::RemoveAnyModifierApprox(typename Modifier<T>::ModifierType type, T value)
{
	// Erase all matching elements and then recalculate the value
	Modifier<T>::ModifierSet::iterator it = std::partition(m_modifiers.begin(), m_modifiers.end(), Modifier<T>::IsNotApproxEqual(type, value));
	if (it != m_modifiers.end())
	{
		m_modifiers.erase(it, m_modifiers.end());
		if (!m_update_suspended) RecalculateValue();
	}
}


// Recalculates the value based on all modifiers that have been applied
template <typename T>
void ModifiedValue<T>::RecalculateValue(void)
{
	// Start with the base value and apply additive modifiers immediately.  Accumulate multiplicative 
	// multipliers for post-multiplication
	T mult = DefaultValues<T>::OneValue();
	Value = BaseValue;

	// Iterate over all modifiers on this value
	Modifier<T>::ModifierSet::const_iterator it_end = m_modifiers.end();
	for (Modifier<T>::ModifierSet::const_iterator it = m_modifiers.begin(); it != it_end; ++it)
	{
		if ((*it).Type == Modifier<T>::ModifierType::Multiplicative)	mult *= (*it).Value; 
		else															Value += (*it).Value;
	}

	// The value has all additive modifiers applied; post-multiply by the combined multipliers to give the final value
	Value *= mult;
}

// Sets modifiers for this value based on a constant reference to the modifiers of another
template <typename T>
void ModifiedValue<T>::SetModifiers(typename const Modifier<T>::ModifierSet & modifiers)
{
	// Simply use direct vector assignment and then recalculate the current value
	m_modifiers = modifiers;
	RecalculateValue();
}

// Sets modifiers for this value based on a movable rvalue reference to the modifiers of another
template <typename T>
void ModifiedValue<T>::SetModifiers(typename Modifier<T>::ModifierSet && modifiers)
{
	// Simply use direct vector assignment and then recalculate the current value
	m_modifiers = modifiers;
	RecalculateValue();
}

// Copy constructor to create an ModifiedValue from an existing value
template <typename T>
ModifiedValue<T>::ModifiedValue(const ModifiedValue<T> & source)
{
	BaseValue = source.BaseValue;
	m_modifiers = source.GetModifiers();
	Value = source.Value;
	m_update_suspended = false;
}


#endif







