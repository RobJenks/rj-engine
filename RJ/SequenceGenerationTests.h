#pragma once

#include "TestBase.h"
#include "TestResult.h"

class SequenceGenerationTests : public TestBase
{
public:

	CMPINLINE TestResult RunTests(void)
	{
		TestResult result = NewNamedResult(SequenceGenerationTests);

		result += BasicInitialisationTests();
		result += InvalidInitialisationTests();
		result += UniquenessTests();

		return result;
	}


private:

	TestResult BasicInitialisationTests();
	TestResult InvalidInitialisationTests();
	TestResult UniquenessTests();

};


