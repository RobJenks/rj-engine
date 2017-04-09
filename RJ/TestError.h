#pragma once

#include <string>
#include <time.h>
#include "Utility.h"


struct TEST_ERR_MSG
{
private:
	std::string m_msg;
public:
	TEST_ERR_MSG(void) : m_msg(NullString) { }
	TEST_ERR_MSG(const std::string & msg) : m_msg(msg) { }
	std::string operator()(void) { return m_msg; }
};

#define TEST_ERR_PREFIX concat("ERROR: [")((unsigned int)timeGetTime())("|")(__FILE__)(":")(__LINE__)("] ").str()
#define ERR(msg) TEST_ERR_MSG(concat(TEST_ERR_PREFIX)(msg).str())



