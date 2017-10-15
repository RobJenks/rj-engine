#pragma once

#include "CompilerSettings.h"
#include "TestBase.h"

class DataPortTests : public TestBase
{
public:

	CMPINLINE TestResult RunTests(void)
	{
		TestResult result = NewNamedResult(DataPortTests);

		result += BasicInitialisationTests();


		return result;
	}


private:

	TestResult BasicInitialisationTests();

};

