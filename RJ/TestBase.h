#pragma once

#include "CompilerSettings.h"
#include "TestResult.h"


class TestBase
{
public:

	TestBase(void) : m_failallcases(false) { }

#	define NewResult() CreateNewResult("")
#	define NewNamedResult(Test) CreateNewResult(#Test)

	TestResult CreateNewResult(const std::string & test) const 
	{ 
		TestResult result;
		result.SetName(test);
		if (m_failallcases) result.FailAllCases();
		
		return result;
	}

	virtual TestResult RunTests(void) = 0;

	void FailAllCases(void) { m_failallcases = true; }


private:
	bool m_failallcases;

};

