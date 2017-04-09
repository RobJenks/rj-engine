#pragma once

#include "CompilerSettings.h"
#include "TestResult.h"


class TestBase
{
public:

	TestBase(void) : m_failallcases(false) { }

	TestResult NewResult(void) const 
	{ 
		TestResult result;
		if (m_failallcases) result.FailAllCases();
		
		return result;
	}

	virtual TestResult RunTests(void) = 0;

	void FailAllCases(void) { m_failallcases = true; }


private:
	bool m_failallcases;

};

