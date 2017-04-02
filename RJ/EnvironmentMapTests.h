#pragma once

#include "TestBase.h"
#include "TestResult.h"

class EnvironmentMapTests : public TestBase
{
public:

	CMPINLINE TestResult RunTests(void)
	{
		TestResult result;

		result += BasicInitialisationTests();
		result += FalloffMethodTests();
		result += BlendModeTests();
		result += BasicAdditivePropogationTests();

		return result;
	}


private:

	TestResult BasicInitialisationTests();
	TestResult FalloffMethodTests();
	TestResult BlendModeTests();

	TestResult BasicAdditivePropogationTests();


	std::unique_ptr<ComplexShip> GenerateTestElementEnvironment(const INTVECTOR3 & size);


};
