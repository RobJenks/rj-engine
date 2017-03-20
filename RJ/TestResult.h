#pragma once

#include <string>
#include <vector>
#include "CompilerSettings.h"

class TestResult
{
public:

	CMPINLINE TestResult(void) : m_passed(0), m_failed(0) { }
	CMPINLINE TestResult(int _passed, int _failed) : m_passed(_passed), m_failed(_failed) { }
	CMPINLINE TestResult(int _passed, int _failed, const std::vector<std::string> & _messages) : m_passed(_passed), m_failed(_failed), m_messages(_messages) { }


	CMPINLINE void TestPassed(void) { ++m_passed; }
	CMPINLINE void TestFailed(void) { ++m_failed; }

	CMPINLINE void LogMessage(const std::string & message) { m_messages.push_back(message); }
	CMPINLINE void ClearAllLoggedMessages(void) { m_messages.clear(); }

	CMPINLINE int GetCurrentTestIndex(void) const { return (m_passed + m_failed); }

	CMPINLINE int GetPassCount(void) const { return m_passed; }
	CMPINLINE int GetFailCount(void) const { return m_failed; }
	CMPINLINE int GetTestCount(void) const { return (m_passed + m_failed); }
	CMPINLINE const std::vector<std::string> & GetMessages(void) const { return m_messages; }
	CMPINLINE bool HasMessages(void) const { return !m_messages.empty(); }

private:

	int m_passed, m_failed;
	std::vector<std::string> m_messages;

};