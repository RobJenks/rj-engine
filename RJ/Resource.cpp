#include <vector>
#include "Utility.h"
#include "GameDataExtern.h"
#include "ResourceAmount.h"
#include "ProductionCost.h"
#include "Resource.h"
using namespace std;

Resource::Resource(void)
{
	// Initialise values to their defaults
	m_code = "";
	m_name = "";
	m_value = 1.0f;
	m_asteroidresource = false;
	m_planetresource = false;
	m_productioncost = new ProductionCost();
	m_compoundvalue = 1.0f;
}

void Resource::SetProductionCost(ProductionCost *pcost)
{
	// Deallocate any existing data
	if (m_productioncost) { delete m_productioncost; m_productioncost = NULL; }

	// Store the new data
	m_productioncost = pcost;
}


// Recursive method which will determine the compound value of the resource based on its dependencies.  Handles stack/infinite loop issues
Result Resource::DetermineCompoundValue(void)
{
	// We will maintain a count of the number of levels of recursion we follow to determine compound values.  If we
	// exceed some threshold then we assume there must be an infinite loop of dependencies and return an error
	static const int _INVOKE_LIMIT = 100;
	static int _INVOKE_COUNT = 0;

	// If we have exceeded the invocation limit then we are most likely in an infinite loop and should terminate with error here
	if (++_INVOKE_COUNT > _INVOKE_LIMIT) return ErrorCodes::EncounteredRecursionLimitValuingResources;

	// Make sure we have production cost data.  Also (as a sanity check) make sure this resource has a valid value
	if (!m_productioncost)	m_productioncost = new ProductionCost();
	if (m_value <= 0.0f)	m_value = 1.0f;

	// If this resource has no resource requirements then it is at the base of the resource tree, and has compound value equal to its own value
	std::vector<ProductionCost::ResourceRequirementCollection>::size_type depcount = m_productioncost->GetResourceRequirementCount();
	if (depcount == 0)
	{
		SetCompoundValue(m_value);			// Store the compound value ( = our own intrinsic value)
		--_INVOKE_COUNT;					// Reduce the invocation count now that we have calculated the compound value
		return ErrorCodes::NoError;			// Return success
	}

	// Otherwise, we need to check each resource requirement in turn
	ResourceAmount req;
	const Resource *res;
	Result result;

	// Start with the intrinsic value of this resource and add that of its component resources one-by-one
	float totalvalue = m_value;
	for (std::vector<ProductionCost::ResourceRequirementCollection>::size_type i = 0; i < depcount; i++)
	{
		// Get a reference to the resource that is a dependency
		req = m_productioncost->ResourceRequirements[i].Requirement;
		res = req.Type;
		
		// Make sure this is a valid resource
		if (!res) continue;

		// We need to take different action depending on whether this dependency already has its compound value calculated
		float cv = res->GetCompoundValue();
		if (cv > 0.0f)
		{
			// If it already has a compound value then we simply add to our running total, multiplied by the quantity that are required
			totalvalue += (cv * req.Amount);
		}
		else
		{
			// Otherwise we need to move recurisvely down the tree and calculate the compound values below this one in the tree
			// Get a non-const reference to this resource so that we can modify its contents
			Resource *r = D::GetResource(res->GetCode());

			// Determine the compound value of this resource (by recursively moving down the tree)
			result = r->DetermineCompoundValue();
			if (result != ErrorCodes::NoError) return result;

			// We should now have a valid value calculated, but validate it to be sure
			cv = res->GetCompoundValue();
			if (cv <= 0.0f) return ErrorCodes::UnreportedErrorCalculatingResourceCompoundValue;

			// Add this value to the running total and continue to the next dependency
			totalvalue += (cv * req.Amount);
		}	
	}

	// We should now have a valid compound value for this resource.  Make sure it is valid as a final check
	if (totalvalue <= 0.0f) return ErrorCodes::UnreportedErrorCalculatingResourceCompoundValue;
	
	// Set the compound value of this resource
	SetCompoundValue(totalvalue);

	// We have successfully processed this resource, so decrement the invocation counter again since we are moving back up the stack
	--_INVOKE_COUNT;

	// Return success
	return ErrorCodes::NoError;
}



Resource::~Resource(void)
{
	// Delete memory allocated to this object
	if (m_productioncost) { delete m_productioncost; m_productioncost = NULL; }
}
