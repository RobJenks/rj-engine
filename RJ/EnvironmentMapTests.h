#pragma once

#include "TestBase.h"
#include "TestResult.h"

class EnvironmentMapTests : public TestBase
{
public:

	CMPINLINE TestResult RunTests(void)
	{
		TestResult result = NewResult();

		result += BasicInitialisationTests();
		result += FalloffMethodTests();
		result += BlendModeTests();
		result += BasicAdditivePropogationTests();
		result += BasicMultiplicativePropogationTests();
		result += BasicAveragedPropogationTests();
		result += BasicRelativeFalloffPropogationTests();

		return result;
	}


private:

	TestResult BasicInitialisationTests();
	TestResult FalloffMethodTests();
	TestResult BlendModeTests();

	TestResult BasicAdditivePropogationTests();
	TestResult BasicMultiplicativePropogationTests();
	TestResult BasicAveragedPropogationTests();
	TestResult BasicRelativeFalloffPropogationTests();


	std::unique_ptr<ComplexShip> GenerateTestElementEnvironment(const INTVECTOR3 & size);


};
