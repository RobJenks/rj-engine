#pragma once

#include <string>
#include <vector>
#include "CompilerSettings.h"
#include "LogManager.h"
#include "TestError.h"


class TestResult
{
public:

	CMPINLINE TestResult(void) : m_passed(0), m_failed(0), m_failallcases(false) { }
	CMPINLINE TestResult(int _passed, int _failed) : m_passed(_passed), m_failed(_failed), m_failallcases(false) { }
	CMPINLINE TestResult(int _passed, int _failed, const std::vector<std::string> & _messages) : m_passed(_passed), m_failed(_failed), m_messages(_messages), m_failallcases(false) { }


	CMPINLINE void TestPassed(void) { if (!m_failallcases) ++m_passed; else ++m_failed; }
	CMPINLINE void TestFailed(void) { ++m_failed; }

	CMPINLINE void LogMessage(const std::string & message) { m_messages.push_back(message); }
	CMPINLINE void ClearAllLoggedMessages(void) { m_messages.clear(); }

	CMPINLINE int GetCurrentTestIndex(void) const { return (m_passed + m_failed); }

	CMPINLINE int GetPassCount(void) const { return m_passed; }
	CMPINLINE int GetFailCount(void) const { return m_failed; }
	CMPINLINE int GetTestCount(void) const { return (m_passed + m_failed); }
	CMPINLINE const std::vector<std::string> & GetMessages(void) const { return m_messages; }
	CMPINLINE bool HasMessages(void) const { return !m_messages.empty(); }

	CMPINLINE void FailAllCases(void) { m_failallcases = true; }

	CMPINLINE void Assert(bool condition, TEST_ERR_MSG failure_message)
	{
		AssertEqual(condition, true, failure_message);
	}

	CMPINLINE void AssertTrue(bool value, TEST_ERR_MSG failure_message)
	{
		AssertEqual(value, true, failure_message);
	}

	CMPINLINE void AssertFalse(bool value, TEST_ERR_MSG failure_message)
	{
		AssertEqual(value, false, failure_message);
	}
	
	template <typename T>
	CMPINLINE void AssertEqual(T x0, T x1, TEST_ERR_MSG failure_message)
	{
		if (x0 == x1 && !m_failallcases) ++m_passed;
		else
		{
			++m_failed;

			std::string inequality = concat("[")(x0)("] != [")(x1)("]").str();
			OutputDebugString(concat("Equality assertion failed; ")(inequality)("\n").str().c_str());

			if (!failure_message().empty()) LogMessage(concat(failure_message())(" (")(inequality)(")").str());
		}
	}


	CMPINLINE void combineWith(const TestResult & other)
	{
		m_passed += other.GetPassCount();
		m_failed += other.GetFailCount();
		m_messages.insert(m_messages.end(), other.GetMessages().begin(), other.GetMessages().end());
	}

	CMPINLINE TestResult & operator+=(const TestResult & rhs) { combineWith(rhs); return *this; }

private:

	int m_passed, m_failed;
	std::vector<std::string> m_messages;
	bool m_failallcases;

};

CMPINLINE TestResult operator+(TestResult lhs, const TestResult & rhs) { lhs += rhs; return lhs; }


