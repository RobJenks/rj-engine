#pragma once

#ifndef __ProductionProgressH_
#define __ProductionProgressH_

#include "ResourceAmount.h"

class ProductionProgress
{
public:
	// Key variables
	ResourceAmount									Requirement;
	float											Progress;
	bool											Complete;

	// Constructors
	ProductionProgress(void)										{ Requirement = ResourceAmount(); Progress = 0.0f; Complete = false; }
	ProductionProgress(ResourceAmount requirement)					{ Requirement = requirement; Progress = 0.0f; Complete = false; }
	ProductionProgress(ResourceAmount requirement, float progress)	
	{ 
		Requirement = requirement; 
		Progress = progress; 
		Complete = (Progress >= Requirement.Amount);
	}

	// Method to change the progress of this construction
	float ChangeProgress(float delta)
	{
		float amt;

		// Test whether we are adding or subtracting progress
		if (delta > 0 && !Complete)
		{
			// If we are adding progress, allow the smaller of 'delta' and the remaining progress to be completed
			amt = min(delta, Requirement.Amount - Progress);
		}
		else if (delta < 0)
		{
			// If we are removing progress, remove the least of 'delta' and the amount of progress completed to date (-ve >>> =max(), not min)
			amt = max(delta, -Progress);
		}
		else return 0;

		// Apply the change
		Progress += amt;

		// Check if the production is now done
		Complete = (Progress >= Requirement.Amount);

		// Return the amount that was added or removed
		return amt;
	}

	// Default destructor
	~ProductionProgress(void) { }
};



#endif