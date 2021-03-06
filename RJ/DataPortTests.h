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
		result += BasicTerrainEnvironmentInitialisationTests();
		result += BasicConnectonTests();
		result += DataPortEnvironmentInteractionTests();
		result += BasicDataTransmissionTests();


		return result;
	}


private:

	TestResult BasicInitialisationTests();
	TestResult BasicTerrainEnvironmentInitialisationTests();
	TestResult BasicConnectonTests();
	TestResult DataPortEnvironmentInteractionTests();
	TestResult BasicDataTransmissionTests();

};

