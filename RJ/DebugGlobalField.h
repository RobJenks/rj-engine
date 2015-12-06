#pragma once

#ifndef __DebugGlobalFieldH__
#define __DebugGlobalFieldH__

#include <string>
#include <unordered_map>
#include "CompilerSettings.h"

template <typename T>
class DebugGlobalField
{
public:

	// Constructor; accepts a value that represents the NULL type for this data type
	DebugGlobalField(T null_type)						{ m_nulltype = null_type; }

	// Collection of debug fields
	std::unordered_map<std::string, T>					Items;

	// Add a new debug field
	CMPINLINE void Set(const std::string & key, T item)	{ if (key != "") Items[key] = item; }

	// Remove a debug field
	CMPINLINE void Remove(const std::string & key)		{ if (key != "" && Items.count(key) != 0) Items.erase(key); }

	// Retrieve a debug field, if it exists, otherwise return null_type
	CMPINLINE T Get(const std::string & key)			{ if (key != "" && Items.count(key) != 0) return Items[key]; else return m_nulltype; }

protected:

	T													m_nulltype;

};



#endif