#pragma once

#ifndef __FactionH__
#define __FactionH__

#include <string>
#include "CompilerSettings.h"


// This class has no special alignment requirements
class Faction
{
public:

	// Define the type used for faction IDs
	typedef int F_ID;

	// Enumeration of disposition values; the disposition of one faction towards another
	enum FactionDisposition { Neutral = 0, Hostile, Friendly };

	// Faction 0 will always be the null faction, which non-affiliated objects belong to
	static const F_ID				NullFaction = (F_ID)0;

	// Default constructor
	Faction(void);

	// Get / set the unique ID for this faction
	CMPINLINE F_ID					GetFactionID(void) const				{ return m_id; }
	CMPINLINE void					SetFactionID(F_ID id)					{ m_id = id; }

	// Get / set the string code and name of this faction
	CMPINLINE const std::string &	GetCode(void) const						{ return m_code; }
	CMPINLINE void					SetCode(const std::string & code)		{ m_code = code; }
	CMPINLINE const std::string &	GetName(void) const						{ return m_name; }
	CMPINLINE void					SetName(const std::string & name)		{ m_name = name; }

	// Default destructor
	~Faction(void);


protected:

	// Key fields
	F_ID							m_id;
	std::string						m_code;
	std::string						m_name;

};


#endif