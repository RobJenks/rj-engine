#pragma once

#ifndef __ModifierH__
#define __ModifierH__

#include <vector>
#include "DefaultValues.h"
#include "ModifierDetails.h"
#include "StandardModifiers.h"

// Defines a modifier to be applied to some attribute or calculation
// This class has no special alignment requirements
template <typename T>
struct Modifier
{
public:

	// Defines the type of modifier being applied; generally multiplicative or additive
	enum ModifierType { Multiplicative = 0, Additive };

	// Defines the standard collection of modifiers
	typedef std::vector<Modifier<T>> ModifierSet;


	// Key fields
	ModifierType					Type;
	T								Value;
	ModifierDetails::ModifierID		Details;

	// Default constructor; set all values to default
	Modifier(void) : Type(ModifierType::Multiplicative), Value(DefaultValues<T>::OneValue()), Details(StandardModifiers::NO_MODIFIER)	{ }

	// Constuctor that passes required modifier values
	Modifier(ModifierType type, T value) : Type(type), Value(value), Details(StandardModifiers::NO_MODIFIER)							{ }
	Modifier(ModifierType type, T value, ModifierDetails::ModifierID details) : Type(type), Value(value), Details(details)				{ }

	// Copy constructor
	Modifier(const Modifier<T> & other) : Type(other.Type), Value(other.Value), Details(other.Details)									{ }

	// Define custom equality operators for this type
	CMPINLINE friend bool operator== (const Modifier<T> &v1, const Modifier<T> &v2) { return (v1.Type == v2.Type && v1.Value == v2.Value); }
	CMPINLINE friend bool operator!= (const Modifier<T> &v1, const Modifier<T> &v2) { return (v1.Type != v2.Type || v1.Value != v2.Value); }

	// Define custom Equals() and ApproxEquals() methods for each modifier type
	CMPINLINE bool Equals(const Modifier<T> & v) const { return (v.Type == Type && v.Value == Value); }
	CMPINLINE bool ApproxEquals(const Modifier<T> & v) const 
	{ 
		return (v.Type == Type && (std::abs(v.Value - Value) < DefaultValues<T>::EpsilonValue()));
	}

	// Define custom functor to allow std::* operations over sets of modifiers where we want to test equality
	struct IsEqual
	{
		ModifierType _type; T _value;
		IsEqual(const Modifier<T> & value) : _type(value.Type), _value(value.Value) { }
		IsEqual(ModifierType type, T value) : _type(type), _value(value) { }
		bool operator()(const Modifier<T> & value) const { return (value.Type == _type && value.Value == _value); }
	};
	// Define custom functor to allow std::* operations over sets of modifiers where we want to test equality
	struct IsNotEqual
	{
		ModifierType _type; T _value;
		IsNotEqual(const Modifier<T> & value) : _type(value.Type), _value(value.Value) { }
		IsNotEqual(ModifierType type, T value) : _type(type), _value(value) { }
		bool operator()(const Modifier<T> & value) const { return (value.Type != _type || value.Value != _value); }
	};

	// Define custom functor to allow std::* operations over sets of modifiers where we want to test approximate equality
	struct IsApproxEqual
	{
		ModifierType _type; T _value;
		IsApproxEqual(const Modifier<T> & value) : _type(value.Type), _value(value.Value) { }
		IsApproxEqual(ModifierType type, T value) : _type(type), _value(value) { }
		bool operator()(const Modifier<T> & value) const 
		{ 
			return (value.Type == _type && (std::abs(value.Value - _value) < DefaultValues<T>::EpsilonValue())); 
		}
	};
	// Define custom functor to allow std::* operations over sets of modifiers where we want to test approximate equality
	struct IsNotApproxEqual
	{
		ModifierType _type; T _value;
		IsNotApproxEqual(const Modifier<T> & value) : _type(value.Type), _value(value.Value) { }
		IsNotApproxEqual(ModifierType type, T value) : _type(type), _value(value) { }
		bool operator()(const Modifier<T> & value) const
		{
			return !(value.Type == _type && (std::abs(value.Value - _value) < DefaultValues<T>::EpsilonValue()));
		}
	};

	// Define custom functor to allow std::* operations over sets of modifiers where we want to test modifier type only
	struct IsOfType
	{
		ModifierType _type;
		IsOfType(ModifierType type) : _type(type) { }
		bool operator()(const Modifier<T> & value) const { return (value.Type == _type); }
	};
	// Define custom functor to allow std::* operations over sets of modifiers where we want to test modifier type only
	struct IsNotOfType
	{
		ModifierType _type;
		IsNotOfType(ModifierType type) : _type(type) { }
		bool operator()(const Modifier<T> & value) const { return (value.Type != _type); }
	};

};

// Specialised subclass of Modifier<T> for an additive modifier
template <typename T>
struct AdditiveModifier : public Modifier<T>
{
	AdditiveModifier(void)		: Modifier<T>(ModifierType::Additive, DefaultValues<T>::NullValue()) { }
	AdditiveModifier(T value)	: Modifier<T>(ModifierType::Additive, value) { }
};

// Specialised subclass of Modifier<T> for a multiplicative modifier
template <typename T>
struct MultiplicativeModifier : public Modifier<T>
{
	MultiplicativeModifier(void)		: Modifier<T>(ModifierType::Multiplicative, DefaultValues<T>::OneValue()) { }
	MultiplicativeModifier(T value)		: Modifier<T>(ModifierType::Multiplicative, value) { }
};

#endif