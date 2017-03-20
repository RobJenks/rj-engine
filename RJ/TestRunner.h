#pragma once

#include <vector>
#include "CompilerSettings.h"
#include "TestBase.h"
#include "TestResult.h"

class TestRunner
{
public:

	CMPINLINE TestRunner(void) { Initialise(); }

	CMPINLINE void Initialise(void) { m_results.clear(); }

	template <class T>
	CMPINLINE void Run(void)
	{
		TestBase *test = new T();
		m_results.push_back(test->RunTests());
		delete test;
	}

	CMPINLINE std::vector<TestResult> GetResults(void) const { return m_results; }

	CMPINLINE std::vector<TestResult>::size_type ResultCount(void) const { return m_results.size(); }

private:

	std::vector<TestResult> m_results;


};