#pragma once

#include <memory>
#include "TestBase.h"
#include "TestResult.h"
class CompoundElementModel;

class CompoundElementModelTests : public TestBase
{
public:

	CMPINLINE TestResult RunTests(void)
	{
		TestResult result = NewNamedResult(CompoundElementModelTests);

		result += LayoutCalculationTests();

		return result;
	}


private:

	TestResult LayoutCalculationTests();


	std::unique_ptr<CompoundElementModel>		GenerateCompoundElementModelData(const UINTVECTOR3 & size) const;
	UINTVECTOR3									DetermineElementLocationFromIndex(UINT index, const UINTVECTOR3 & size) const;

};
