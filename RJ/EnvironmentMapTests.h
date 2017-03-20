#pragma once

#include "TestBase.h"
#include "TestResult.h"

class EnvironmentMapTests : public TestBase
{
public:

	CMPINLINE TestResult RunTests(void)
	{
		std::vector<std::string> msgs;
		msgs.push_back("Some first message");
		msgs.push_back("Message number 2");
		msgs.push_back("A third message");

		return TestResult(43, 21, msgs);
	}

};
