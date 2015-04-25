#pragma once

#ifndef __CrewH__
#define __CrewH__

#include <string>
#include "Utility.h"
using namespace std;

class Crew
{
public:
	// Enumeration of crew classes
	enum CrewClass
	{
		UnknownCrewClass = 0,
		Admiral,
		Engineer,
		Marine,
		Doctor 
		// , ...
	};


	// String decription of a crew member's role will be based upon a mapping from { Class, Level, ... }
	// e.g. someone with a class of "Engineer" and level 1 could be "Mechanic", level 5 could be "Engineer", level 12 could be "Master Engineer", ...
	// e.g. someone with a class of "Marine" and level 1 could be "Rookie", level 4 could be "Marine", level 10 could be "Special Ops", ...
	// Could also make this dependent on the skills/attributes of the person, e.g. if the engineer has highest skill in electronics then perhaps 
	// append "Systems" to make him "Systems Engineer", "Master Systems Engineer", ...




	Crew(void);
	~Crew(void);

	// Translates the string name of a crew class into the enum value itself.  Case-insensitive
	// TODO: Longer-term, replace all string comparisons to use hash values for greater efficiency?
	// Could create arrays at startup of {Hash value, Actual enum value, [printable string value?]} and then just loop & compare the numeric hash.
	// OR use a hash table component within C++ (~ std::hash_map) ?
	static CrewClass TranslateCrewClassFromString(const string & name)
	{
		// Case-insensitive string comparison
		string val = StrLower(name);

		// Test for each crew class in turn
		if (val == "admiral")					return CrewClass::Admiral; 
		else if (val == "engineer")				return CrewClass::Engineer;
		else if (val == "marine")				return CrewClass::Marine;
		else if (val == "doctor")				return CrewClass::Doctor;

		// Default value will be an "unknown" crew class
		else									return CrewClass::UnknownCrewClass;
	}

	static string TranslateCrewClassToString(CrewClass cls)
	{
		// Consider each crew class in turn
		switch (cls)
		{
			case CrewClass::Admiral:			return "Admiral";
			case CrewClass::Engineer:			return "Engineer";
			case CrewClass::Marine:				return "Marine";
			case CrewClass::Doctor:				return "Doctor";

			// Default will be an unknown crew class
			default:							return "Unknown";
		}
	}
};



#endif