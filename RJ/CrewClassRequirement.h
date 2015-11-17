#pragma once

#ifndef __CrewClassRequirementH__
#define __CrewClassRequirementH__

#include "Crew.h"


// This file contains no objects with special alignment requirements
class CrewClassRequirement
{
public:

	// Requirements for crew member class and minimum level
	Crew::CrewClass								Class;			// Can be set to "Unknown" to exclude class as a requirement
	int											Level;			// Can be set to zero to allow all levels of crew to meet the requirement
	
	CrewClassRequirement(void)									{ Class = Crew::CrewClass::UnknownCrewClass; Level = 0; }
	CrewClassRequirement(Crew::CrewClass _Class)				{ Class = _Class; Level = 0; }
	CrewClassRequirement(int _Level)							{ Class = Crew::CrewClass::UnknownCrewClass; Level = _Level; }
	CrewClassRequirement(Crew::CrewClass _Class, int _Level)	{ Class = _Class; Level = _Level; }

	~CrewClassRequirement(void) { }
};



#endif