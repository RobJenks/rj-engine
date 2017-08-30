#pragma once

#ifndef __ProductionCostH__
#define __ProductionCostH__

#include <vector>
#include <string>
#include "CompilerSettings.h"
#include "ALIGN16.h"
#include "Resource.h"
#include "ResourceAmount.h"
#include "ProductionProgress.h"
#include "CrewClassRequirement.h"
#include "ShipTileRequirement.h"

// This class has no special alignment requirements
class ProductionCost
{
public:

	// Vector of resource requirements for this production
	typedef std::vector<ProductionProgress>					ResourceRequirementCollection;
	ResourceRequirementCollection 							ResourceRequirements;
	CMPINLINE ResourceRequirementCollection::size_type		GetResourceRequirementCount(void)	{ return ResourceRequirements.size(); }

	// Vector of crew class/level requirements for this production (e.g. ship must have an active Engineer of level >5 to build this)
	typedef std::vector<CrewClassRequirement>				CrewClassRequirementCollection;
	CrewClassRequirementCollection							CrewClassRequirements;
	
	// Vector of ship tile class/definition requirements for production (e.g. ship must have 1x advanced engineering to build this)
	typedef std::vector<ShipTileRequirement>				ShipTileRequirementCollection;
	ShipTileRequirementCollection							ShipTileRequirements;

	// Vector of tiles that can construct this object
	typedef std::vector<const ComplexShipTileDefinition*>	ConstructedByCollection;
	ConstructedByCollection									ConstructedBy;

	// Time requirement for the whole construction.  Used to calculate the amount of resouce that can be added per sec
	CMPINLINE float								GetTimeRequirement(void)			{ return m_timereq; }
	CMPINLINE void								SetTimeRequirement(float secs)		{ if (m_timereq > 0.0f) m_timereq = secs; }

	// Flag determining whether this production is complete or not
	CMPINLINE bool								IsComplete(void)					{ return m_complete; }
	CMPINLINE void								SetCompletionStatus(bool complete)	{ m_complete = complete; }

	// Methods to add and remove construction progress
	float										AddProgress(const Resource *resource, float amountavailable, float timestep);
	float										RemoveProgress(const Resource *resource, float amount, float timestep);

	// Methods to retrieve different aspects of the construction progress
	float										GetProgress(const Resource *resource);
	float										GetProgressPc(const Resource *resource);
	float										GetProgress(void);
	float										GetProgressPc(void);

	// Determines whether construction is now complete
	bool										CheckIfComplete(void);

	// Directly resets the production to be either 0% or 100% complete.  Used when starting new construction/deconstruction, for example
	void										ResetToZeroPcProgress(void);
	void										ResetTo100PcProgress(void);

	// Creates a clone of the object, with optional ability to scale the resource requirements upon cloning
	CMPINLINE ProductionCost *					CreateClone(void) const						{ return CreateClone(1.0f); }
	ProductionCost *							CreateClone(float scaleresourcereq) const;

	// Adds a new object.  In some cases this is via its string code, to be resolved later during post-load initialisation
	void										AddResourceRequirement(const std::string & resource, float amount);
	void										AddCrewClassRequirement(const std::string & cls, int count);
	void										AddShipTileRequirement(const std::string & tilecls, const std::string & tiledef,  int level, int count);
	void										AddConstructionOwner(const std::string & owner);

	// Method to perform post-load initialisation; this will resolve any links that were previously stored via only their string code
	void										PerformPostLoadInitialisation(void);

	// Default constructor / destructor
	ProductionCost(void);
	~ProductionCost(void);

protected:

	// Time requirement for the whole construction.  Used to calculate the amount of resouce that can be added per sec
	float								m_timereq;

	// Flag indicating whether production is complete
	bool								m_complete;
	
	// Structs to hold temporary pre-initialisation data
	struct tmpstring_resreq {
		std::string res; float amt;
		tmpstring_resreq(const std::string & _res, float _amt) { res = _res; amt = _amt; }
	};
	struct tmpstring_shiptilereq { 
		std::string tilecls; std::string tiledef; int level; int count;
		tmpstring_shiptilereq(const std::string & _cls, const std::string & _def, int _level, int _count)
			{ tilecls = _cls; tiledef = _def; level = _level; count = _count; }
	};

	// Temporary storage for loaded data before initialisation, e.g. the code of certain objects before they are resolved and linked
	std::vector<tmpstring_resreq>			m_tmpstring_resourcerequirements;
	std::vector<tmpstring_shiptilereq>		m_tmpstring_shiptilerequirements;
	std::vector<std::string>				m_tmpstring_constructedby;

};



#endif