#pragma once

#ifndef __ModifierDetailsH__
#define __ModifierDetailsH__

#include <vector>
#include "CompilerSettings.h"

class ModifierDetails
{
public:

	// Type definitions
	typedef size_t									ModifierID;
	
	// Default constructor
	CMPINLINE ModifierDetails(void)					: m_id(0U), m_name("<null>"), m_description("<null>") { }

	// Constructor to define object with primary data
	CMPINLINE ModifierDetails(ModifierID id, const std::string & name, const std::string & description) 
													: m_id(id), m_name(name), m_description(description) { }

	CMPINLINE ModifierID							GetID(void) const						{ return m_id; }
	CMPINLINE void									SetID(ModifierID id)					{ m_id = id; }

	CMPINLINE std::string							GetName(void) const						{ return m_name; }
	CMPINLINE void									SetName(const std::string name)			{ m_name = name; }

	CMPINLINE std::string							GetDescription(void) const				{ return m_description; }
	CMPINLINE void									SetDescription(const std::string desc)	{ m_description = desc; }

	// Indicates whether this object is a "null" modifier
	bool											IsNull(void) const;


private:

	ModifierID										m_id;
	std::string										m_name;
	std::string										m_description;

};



#endif









