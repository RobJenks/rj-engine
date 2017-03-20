#pragma once

#include "TestResult.h"


class TestBase
{
public:

	virtual TestResult RunTests(void) = 0;

};

