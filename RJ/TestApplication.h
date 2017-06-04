#pragma once

#include <fstream>
#include "RJMain.h"
#include "TestRunner.h"
#include "TestResult.h"

#include "EnvironmentMapTests.h"
#include "SequenceGenerationTests.h"


class TestApplication
{
public:

	// Debug flag; can be set to report all tests as failed, such that all useful debug output is generated
	static const bool DEBUG_FAIL_ALL_CASES = false;


	// Run all tests - debug mode only
	void RunAllTests(RJMain & ApplicationReference)
	{
		// Initialise test runner
		TestRunner tester = Initialise();
		if (DEBUG_FAIL_ALL_CASES) tester.FailAllCases();

		// Run all required tests
		tester.Run<EnvironmentMapTests>();
		tester.Run<SequenceGenerationTests>();
			


		// Test complete
		Complete(tester);
	}

	// Default constructor
	TestApplication(void) { }

	// Move constructor
	TestApplication(TestApplication && other) { }

	// Delete copy constructor & assignment operators since member std::ofstream is non-copyable
	TestApplication(const TestApplication & other) { }
	TestApplication& operator=(const TestApplication & other) = delete;


private:

	std::ofstream *m_testlog = NULL;


	CMPINLINE void Log(const std::string & text)
	{
		OutputDebugString(text.c_str());
		(*m_testlog) << text;
		(*m_testlog).flush();
	}

	CMPINLINE TestRunner Initialise(void)
	{
		m_testlog = new std::ofstream("test_log.txt", std::ofstream::out | std::ofstream::trunc);
		Log("\n=== Test application initialised; executing tests ===\n\n");

		return TestRunner();
	}

	CMPINLINE void Complete(TestRunner tester)
	{
		// Output test results
		size_t test_count = tester.ResultCount();
		Log(concat("Executed ")(test_count)(" tests")(test_count != 0U ? ":\n" : "\n").str());
		for (size_t i = 0U; i < test_count; ++i)
		{
			TestResult result = tester.GetResults().at(i);
			std::string s = concat("Test ")(i)(": ")(result.GetPassCount())(" Passed, ")(result.GetFailCount())(" Failed").str();
			if (result.HasMessages())
			{
				s = concat(s)(", ")(result.GetMessages().size())(" Messages:\n").str();
				for (std::string msg : result.GetMessages())
				{
					s = concat(s)("    ")(msg)("\n").str();
				}
			}
			else s = concat(s)("\n").str();
			Log(s);
		}

		Log("\n=== Test execution completed ===\n\n");

		// Close the test log
		(*m_testlog).flush();
		(*m_testlog).close();
		delete m_testlog;
	}





};





