#include <string>
#include "GameDataExtern.h"
#include "GameVarsExtern.h"
#include "ProductionProgress.h"
#include "ProductionCost.h"
#include "ComplexShipTile.h"
#include "Crew.h"
#include "CrewClassRequirement.h"
#include "ShipTileRequirement.h"
class ComplexShipTileClass;
using namespace std;


ProductionCost::ProductionCost(void)
{
	// Initialise variables to their default values
	m_timereq = 1.0f;
	m_complete = false;
}


// Method to perform post-load initialisation; this will resolve any links that were previously stored via only their string code
void ProductionCost::PerformPostLoadInitialisation(void)
{
	Resource *resource;
	ComplexShipTileClass *cls;
	ComplexShipTileDefinition *tile;

	// Iterate through each string representation of a resource requirement
	vector<tmpstring_resreq>::const_iterator it_end = m_tmpstring_resourcerequirements.end();
	for (vector<tmpstring_resreq>::const_iterator it = m_tmpstring_resourcerequirements.begin(); it != it_end; ++it)
	{
		// Attempt to resolve the resource code specified in this requirement
		resource = D::GetResource(it->res);
		if (resource) ResourceRequirements.push_back(ProductionProgress(ResourceAmount(resource, it->amt)));
	}

	// Iterate through each string representation of a ship tile requirement
	vector<tmpstring_shiptilereq>::const_iterator it2_end = m_tmpstring_shiptilerequirements.end();
	for (vector<tmpstring_shiptilereq>::const_iterator it2 = m_tmpstring_shiptilerequirements.begin(); it2 != it2_end; ++it2)
	{
		// Attempt to resolve the class and definition specified in this requirement
		cls = D::GetComplexShipTileClass(it2->tilecls);
		if (cls) ShipTileRequirements.push_back(ShipTileRequirement(cls, D::GetComplexShipTile(it2->tiledef),
																	it2->level, it2->count));
	}

	// Iterate through each string code linking this production to an "owner"
	vector<string>::const_iterator it3_end = m_tmpstring_constructedby.end();
	for (vector<string>::const_iterator it3 = m_tmpstring_constructedby.begin(); it3 != it3_end; ++it3)
	{
		// Attempt to get a reference to the tile with this code
		tile = D::GetComplexShipTile(*it3);
		if (tile) ConstructedBy.push_back(tile);
	}

	// Clear the temporary vectors now that we have resolved all links
	m_tmpstring_resourcerequirements.clear();
	m_tmpstring_shiptilerequirements.clear();
	m_tmpstring_constructedby.clear();
}


// Adds production progress for the specified resource.  Returns the amount of progress that was added.  Timestep is the 
// amount of time since we last added resources to this tile element
float ProductionCost::AddProgress(const Resource *resource, float amountavailable, float timestep)
{
	// Parameter check.  Also go no further if we are already complete
	if (!resource || amountavailable <= 0 || m_complete) return 0.0f;

	// Iterate through each resource in turn to look for the item being added
	vector<ProductionProgress>::iterator it_end = ResourceRequirements.end();
	for (vector<ProductionProgress>::iterator it = ResourceRequirements.begin(); it != it_end; ++it)
	{
		// Test whether this is the resource we are incrementing
		if (it->Requirement.Type == resource)
		{
			// Determine the actual amount we are adding
			float targetamt = min( (it->Requirement.Amount / m_timereq) * timestep,				// The max amount that we can add in this time step, 
									amountavailable )	;										// or the amount we actually have available, if smaller

			// Attempt to add this progress, and store the amount that was actually returned 
			float amt = it->ChangeProgress(targetamt);

			// Test whether this now resulted in the ProdProgress object being completed; if so, check our overall completion
			if (it->Complete) CheckIfComplete();

			// Return the amount of progress that was added
			return amt;
		}
	}

	// We did not find a matching resource
	return 0.0f;
}

// Removes production progress for the specified resource.  Returns the amount of progress that was removed.  Timestep is
// the time since we last tried removing progress from this tile element
float ProductionCost::RemoveProgress(const Resource *resource, float amount, float timestep)
{
	// Parameter check.  Note that amount should be +ve (we are subtracting a positive amount)
	if (!resource || amount <= 0.0f) return 0.0f;

	// Iterate through each resource in turn to look for the item being removed
	vector<ProductionProgress>::iterator it_end = ResourceRequirements.end();
	for (vector<ProductionProgress>::iterator it = ResourceRequirements.begin(); it != it_end; ++it)
	{
		// Test whether this is the resource we are removing
		if (it->Requirement.Type == resource)
		{
			// Determine the actual amount we are removing (TODO: uses same timescale as adding resources.  Maybe increase?)
			float targetamt = min( (it->Requirement.Amount / m_timereq) * timestep,				// The max amount that we can remove in this time step, 
									amount )	;												// or the amount we are actually trying to remove, if smaller

			// Attempt to reduce the amount of this resource by 'amount'.  Store the actual amount removed
			float amt = it->ChangeProgress(-targetamt);

			// If the actual change in amount < 0 then we removed something, so cannot now be complete
			if (amt < 0) m_complete = false;

			// Return the actual amount that was removed (negated, since we want to return a +ve number)
			return (-amt);
		}
	}

	// We did not locate this resource if we got this far
	return 0.0f;
}

// Retrieves the construction progress for this resource
float ProductionCost::GetProgress(const Resource *resource)
{
	// Iterate through each resource in turn 
	vector<ProductionProgress>::const_iterator it_end = ResourceRequirements.end();
	for (vector<ProductionProgress>::const_iterator it = ResourceRequirements.begin(); it != it_end; ++it)	
	{
		// If this is the resource in question then return the progress value now
		if (it->Requirement.Type == resource) return it->Progress;
	}

	// If we cannot find this resource then return zero progress
	return 0.0f;
}

// Retrieves the construction progress for this resource, as a percentage of the total resource amount required
float ProductionCost::GetProgressPc(const Resource *resource)
{
	// Iterate through each resource in turn 
	vector<ProductionProgress>::const_iterator it_end = ResourceRequirements.end();
	for (vector<ProductionProgress>::const_iterator it = ResourceRequirements.begin(); it != it_end; ++it)	
	{
		// If this is the resource in question then return the progress percentage now
		if (it->Requirement.Type == resource && it->Requirement.Amount > 0.0f) return (it->Progress / it->Requirement.Amount);
	}

	// If we cannot find this resource then return zero progress
	return 0.0f;
}

// Returns the overall production progress across all resources
float ProductionCost::GetProgress(void)
{
	float prog = 0.0f;

	// Iterate through each resource in turn 
	vector<ProductionProgress>::const_iterator it_end = ResourceRequirements.end();
	for (vector<ProductionProgress>::const_iterator it = ResourceRequirements.begin(); it != it_end; ++it)	
	{
		// Store the progress this resource
		prog += it->Progress;
	}

	// Return the total progress
	return prog;
}

// Returns the overall production progress, as a percentage of the total resources required
float ProductionCost::GetProgressPc(void)
{
	float total = 0.0f;
	float prog = 0.0f;

	// Iterate through each resource in turn 
	vector<ProductionProgress>::const_iterator it_end = ResourceRequirements.end();
	for (vector<ProductionProgress>::const_iterator it = ResourceRequirements.begin(); it != it_end; ++it)	
	{
		// Store the requirement and progress for this resource
		total += it->Requirement.Amount;
		prog += it->Progress;
	}

	// Assuming we have valid amounts, return the progress as a percentage of total requirement now
	if (total >= 0.0f) return (prog / total); else return 0.0f;
}



// Determines whether construction is now complete
bool ProductionCost::CheckIfComplete(void)
{
	// Iterate through each resource in turn to check whether it is complete
	vector<ProductionProgress>::const_iterator it_end = ResourceRequirements.end();
	for (vector<ProductionProgress>::const_iterator it = ResourceRequirements.begin(); it != it_end; ++it)
	{
		// If this resource is not complete then the overall construction also cannot be complete
		if (it->Complete == false) { m_complete = false; return false; }
	}
	
	// If we got this far then all the components must be complete, so return true
	m_complete = true; 
	return true;
}

// Directly resets the production to 0% complete.  Used when starting new construction, for example
void ProductionCost::ResetToZeroPcProgress(void)
{
	// Loop through each resource requirement in turn
	int n = ResourceRequirements.size();
	for (int i = 0; i < n; i++)
	{
		// Reset the progress for this resource to 0%
		ResourceRequirements[i].Progress = 0.0f;
		ResourceRequirements[i].Complete = false;
	}

	// Set the overall completion flag to false
	m_complete = false;
}
// Directly resets the production to 100% complete.  Used when starting to deconstruct a complete tile, for example
void ProductionCost::ResetTo100PcProgress(void)
{
	// Loop through each resource requirement in turn
	int n = ResourceRequirements.size();
	for (int i = 0; i < n; i++)
	{
		// Reset the progress for this resource to be complete
		ResourceRequirements[i].Progress = ResourceRequirements[i].Requirement.Amount;
		ResourceRequirements[i].Complete = true;
	}

	// Set the overall completion flag to false
	m_complete = true;
}

void ProductionCost::AddResourceRequirement(const string & resource, float amount)
{ 
	// Store in a string vector; this is a property that will be resolved during post-load initialisation
	if (resource != NullString) 
	{
		string res = StrLower(resource);
		m_tmpstring_resourcerequirements.push_back(tmpstring_resreq(res, amount));
	}
}
void ProductionCost::AddCrewClassRequirement(const string & cls, int count)
{ 
	// Crew classes are stored in an enumeration, not loaded from file, so we can resolve these dependencies directly on load
	if (cls != NullString) 
	{
		string str = StrLower(cls);
		Crew::CrewClass cc = Crew::TranslateCrewClassFromString(str);
		if (cc != Crew::CrewClass::UnknownCrewClass) CrewClassRequirements.push_back(CrewClassRequirement(cc, count));
	}
}
void ProductionCost::AddShipTileRequirement(const string & tilecls, const string & tiledef, int level, int count)	
{ 
	// Store in a string vector; this is a property that will be resolved during post-load initialisation
	// Note that the ship class or definition can be null
	string cls = StrLower(tilecls);
	string def = StrLower(tiledef);
	m_tmpstring_shiptilerequirements.push_back(tmpstring_shiptilereq(cls, def, level, count));
}
void ProductionCost::AddConstructionOwner(const string & owner)	
{ 
	// Store in a string vector; this is a property that will be resolved during post-load initialisation
	if (owner != NullString) 
	{
		string str = StrLower(owner);
		m_tmpstring_constructedby.push_back(str); 
	}
}

// Create a clone of the production cost object.  Reduces the resource/time requirement proportional to
// the number of elements that are specified
ProductionCost * ProductionCost::CreateClone(float scaleresourcereq) const
{
	// Create a new resource requirement object
	ProductionCost *pc = new ProductionCost();

	// Copy the resource requirements one at a time
	vector<ProductionProgress>::const_iterator it_res_end = ResourceRequirements.end();
	for (vector<ProductionProgress>::const_iterator it_res = ResourceRequirements.begin(); it_res != it_res_end; ++it_res)
	{
		// Copy this requirement to the new object, scaling if necessary
		pc->ResourceRequirements.push_back(ProductionProgress(ResourceAmount(it_res->Requirement.Type, 
																			 it_res->Requirement.Amount * scaleresourcereq), 
																			 it_res->Progress * scaleresourcereq));
	}

	// Copy the time requirement, again scaled as necessary
	pc->SetTimeRequirement(m_timereq * scaleresourcereq);

	// Copy each crew requirement in turn
	vector<CrewClassRequirement>::const_iterator it_crew_end = CrewClassRequirements.end();
	for (vector<CrewClassRequirement>::const_iterator it_crew = CrewClassRequirements.begin(); it_crew != it_crew_end; ++it_crew)
	{
		// Copy this crew requirement to the new object
		pc->CrewClassRequirements.push_back(*it_crew);
	}

	// Copy each ship tile requirement in turn
	vector<ShipTileRequirement>::const_iterator it_tile_end = ShipTileRequirements.end();
	for (vector<ShipTileRequirement>::const_iterator it_tile = ShipTileRequirements.begin(); it_tile != it_tile_end; ++it_tile)
	{
		// Copy this tile requirement to the new object
		pc->ShipTileRequirements.push_back(*it_tile);
	}

	// Initialise the progress to either 0% or 100% depending on the parent
	if (m_complete)		pc->ResetTo100PcProgress(); 
	else				pc->ResetToZeroPcProgress();

	// Return the new object
	return pc;
}


// Default destructor
ProductionCost::~ProductionCost(void)
{
}
